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


#ifndef __SAI_NET2_TIMERTASK__
#define __SAI_NET2_TIMERTASK__

namespace sai 
{ 
namespace net2
{

class Net;
class TimerTaskImpl;
class TimerTask
{
private:
  TimerTaskImpl * _impl;

public:
  TimerTask();
  virtual ~TimerTask();

  unsigned int getSecInterval();
  unsigned int getMSecInterval();
  void schedule(unsigned int sec, unsigned int msec);
  void schedule();
  void cancel();

  virtual void timerEvent() = 0;
};

}
}

#endif
