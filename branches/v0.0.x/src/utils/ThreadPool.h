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

#ifndef __SAI_UTILS_THREADPOOL_
#define __SAI_UTILS_THREADPOOL_

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif
#include <vector>

namespace sai
{
namespace utils
{

class Thread;
class ThreadTask
{
friend class Thread;
protected:
  Thread * _thrd;

public:
  ThreadTask();
  virtual ~ThreadTask();
  bool schedule();
  void wait(ThreadTask* other);

  virtual void threadEvent() = 0;
};

class ThreadPool;
class ThreadImpl;
class Thread
{
friend class ThreadPool;
friend class ThreadTask;
private:
  ThreadImpl *_impl; 
  ThreadTask *_task;

private:
  Thread();
  ~Thread();

  void start(ThreadTask * task);
  void join(Thread * other);
#ifdef _WIN32
  static DWORD WINAPI run(LPVOID closure);
#else
  static void * run(void * closure);
#endif
};

typedef std::vector<Thread*>           ThreadList;
typedef std::vector<Thread*>::iterator ThreadListIterator;

class LockImpl;
class Locker
{
private:
  LockImpl * _impl;

public:
  Locker();
  ~Locker();
  void lock();
  void unlock();
};

class ThreadPool
{
friend class Thread;
friend class ThreadTask;
private:
  static ThreadPool * _instance;
  ThreadList          _list;
  Locker            * _locker;

private:
  ThreadPool();
  ~ThreadPool();

  Thread     * getFreeThread();
  void         addFreeThread(Thread * t);

public:
  static ThreadPool * getInstance();
  void                setSize(unsigned int size);
};

}
}

#endif

