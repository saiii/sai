//=============================================================================
// Copyright (C) 2011 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//	        
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#include <utils/Logger.h>
#include "ThreadPool.h"

using namespace sai::utils;

namespace sai { namespace utils {

class ThreadImpl
{
public:
#ifdef _WIN32
  HANDLE        thread;
  unsigned long id;
#else
  pthread_t     thread;
#endif

public:
  ThreadImpl()
#ifdef _WIN32
    :thread(NULL),
     id(0)
#endif
  {
  }
};

class LockImpl
{
public:
#ifdef _WIN32
  CRITICAL_SECTION  _mutex;
#else
  pthread_mutex_t   _mutex;
#endif

public:
  LockImpl()
  {
#ifdef _WIN32
    InitializeCriticalSection(&_mutex);
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
  }


  ~LockImpl()
  {
  }

  void lock()
  {
#ifdef _WIN32
    EnterCriticalSection(&_mutex);
#else
    pthread_mutex_lock(&_mutex);
#endif
  }

  void unlock()
  {
#ifdef _WIN32
    LeaveCriticalSection(&_mutex);
#else
    pthread_mutex_unlock(&_mutex);
#endif
  }
};

}}

Locker::Locker():
  _impl(0)
{
  _impl = new LockImpl();
}

Locker::~Locker()
{
  delete _impl;
}

void 
Locker::lock()
{
  _impl->lock();
}

void 
Locker::unlock()
{
  _impl->unlock();
}

ThreadTask::ThreadTask():
  _thrd(0)
{
}

ThreadTask::~ThreadTask()
{
  _thrd = 0;
}

bool
ThreadTask::schedule()
{
  _thrd = ThreadPool::getInstance()->getFreeThread();
  if (_thrd == 0)
  {
    sai::utils::Logger::GetInstance()->print(sai::utils::Logger::SAILVL_ERROR,
      "Unable to create a new thread. Too many threads are running at this time.\n");
    return false;
  }

  _thrd->start(this); 
  return true;
}

void 
ThreadTask::wait(ThreadTask* other)
{
  _thrd->join(other->_thrd);
}

Thread::Thread():
  _impl(0)
{
  _impl = new ThreadImpl();
}

Thread::~Thread()
{
  delete _impl;
}

void 
Thread::start(ThreadTask * task)
{
  if (!task) 
    return;
  _task = task;

#ifdef _WIN32
  _impl->thread = ::CreateThread(NULL, 0, Thread::run, (LPVOID)this, 0, &(_impl->id));
  ::SetThreadPriority(_impl->thread, THREAD_PRIORITY_BELOW_NORMAL);
  //::SetThreadPriority(_thread, THREAD_PRIORITY_NORMAL);
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_create(&(_impl->thread), &attr, Thread::run, this);
  pthread_attr_destroy(&attr);
#endif
}

void 
Thread::join(Thread * other)
{
#ifdef _WIN32
  // TODO
#else
  pthread_join(other->_impl->thread, 0);
#endif
}

#ifdef _WIN32
DWORD WINAPI Thread::run(LPVOID closure)
#else
void * Thread::run(void * closure)
#endif
{
  Thread * thread = (Thread*) closure;
  thread->_task->threadEvent();
  thread->_task->_thrd = 0; 
  thread->_task        = 0;
  ThreadPool::getInstance()->addFreeThread(thread);
  return 0;
}

ThreadPool * ThreadPool::_instance = 0;

ThreadPool * 
ThreadPool::getInstance()
{
  if (_instance == 0)
    _instance = new ThreadPool();
  return _instance;
}

void 
ThreadPool::setSize(unsigned int size)
{
  _locker->lock();
  if (size == _list.size()) 
  {
    _locker->unlock();
    return;
  }

  if (size > _list.size())
  {
    size -= _list.size();
    while(size > 0)
    {
      Thread * thread = new Thread();
      _list.push_back(thread);
      size -= 1;
    }
  }
  else
  {
    size = _list.size() - size;
    while (size > 0)
    {
      Thread * thread = _list.front();
      _list.erase(_list.begin());
      delete thread;
      size -= 1;
    }
  }
  _locker->unlock();
}

ThreadPool::ThreadPool():
  _locker(0)
{
  _locker = new Locker();
  // initial a number of free thread to the list
  int initSize = 10;
  for (int i = 0; i < initSize; i += 1)
  {
    Thread * thread = new Thread();
    _list.push_back(thread);
  } 
}

ThreadPool::~ThreadPool()
{
  _locker->lock();
  while (_list.size() > 0)
  {
    Thread * thread = _list.front();
    _list.erase(_list.begin());
    delete thread;
  }
  _locker->unlock();
  delete _locker;
}

Thread * 
ThreadPool::getFreeThread()
{
  _locker->lock();
  if (_list.size() > 0)
  {
    Thread * ret = _list.front();
    _list.erase(_list.begin());
    _locker->unlock();
    return ret;
  }
  else
  {
    _locker->unlock();
    return 0;
  }
}

void 
ThreadPool::addFreeThread(Thread * t)
{
  _locker->lock();
  _list.push_back(t);
  _locker->unlock();
}

