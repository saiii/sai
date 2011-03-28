//=============================================================================
// Copyright (C) 2008 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/bind.hpp>
#include "Net.h"
#include "TimerTask.h"

namespace sai 
{ 
namespace net 
{

class TimerTaskImpl
{
private:
	boost::asio::deadline_timer  _timer;
	TimerTask                   *_task;
	unsigned int                 _interval;
	bool                         _pending;

public:
	TimerTaskImpl(Net&, TimerTask * task);
	~TimerTaskImpl();

	inline void schedule(unsigned int interval);
	inline void schedule();
	inline unsigned int getInterval() { return _interval; }
	void cancel();
	void timerEvent(const boost::system::error_code& error);
};

TimerTaskImpl::TimerTaskImpl(Net& net, TimerTask * task):
	_timer(net.getIO()),
	_task(task),
	_interval(0),
	_pending(false)
{
	_timer.async_wait(boost::bind(&TimerTaskImpl::timerEvent, this, boost::asio::placeholders::error));
}

TimerTaskImpl::~TimerTaskImpl()
{
}

void 
TimerTaskImpl::cancel()
{
	_timer.cancel();
}

void
TimerTaskImpl::timerEvent(const boost::system::error_code& error)
{
	_pending = true;
	_task->timerEvent();

	if (!error && !_pending)
	{
		_timer.expires_from_now(boost::posix_time::milliseconds(_interval));
		_timer.async_wait(boost::bind(&TimerTaskImpl::timerEvent, this, boost::asio::placeholders::error));
	}
	_pending = false;
}

inline void 
TimerTaskImpl::schedule(unsigned int interval)
{
	_interval = interval;
	schedule();
}

inline void 
TimerTaskImpl::schedule()
{
	if (!_pending)
	{
		_timer.expires_from_now(boost::posix_time::milliseconds(_interval));
		_timer.async_wait(boost::bind(&TimerTaskImpl::timerEvent, this, boost::asio::placeholders::error));
	}
	else
	{
		_pending = false;
	}
}

}
}

//=============================================================================
using namespace sai::net;

TimerTask::TimerTask():
	_impl(0)
{
  Net * net = Net::GetInstance();
  _impl = new TimerTaskImpl(*net, this);
}

TimerTask::~TimerTask()
{
  _impl->cancel();
  delete _impl;
}

unsigned int 
TimerTask::getSecInterval()
{
  return (_impl->getInterval()/1000);
}

unsigned int 
TimerTask::getMSecInterval()
{
  return _impl->getInterval();
}

void 
TimerTask::schedule(unsigned int sec, unsigned int msec)
{
	unsigned int interval  = msec;
	interval += (sec * 1000);

	_impl->schedule(interval);
}

void 
TimerTask::schedule()
{
	_impl->schedule();
}

void 
TimerTask::cancel()
{
	_impl->cancel();
}

