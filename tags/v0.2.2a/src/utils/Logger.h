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

#ifndef __SAI_UTILS_LOGGER__
#define __SAI_UTILS_LOGGER__

namespace sai
{
namespace utils
{

class Logger
{
public:
  typedef enum
  {
    SAILVL_DEBUG,
    SAILVL_INFO,
    SAILVL_WARNING,
    SAILVL_ERROR,
    SAILVL_SYSERROR,
    SAILVL_CRITICAL
  }LogLevel;

private:
  static Logger * _instance;

protected:
  Logger();

public:
  virtual ~Logger();

  static void    SetInstance(Logger *);
  static Logger* GetInstance();

  virtual void print(LogLevel lvl, const char *fmt, ...);
};

}
}

#endif
