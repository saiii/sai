//=============================================================================
// Copyright (C) 2009 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_NET_EXCEPTION__
#define __SAI_NET_EXCEPTION__

#include <string>
#include <exception>

namespace sai 
{ 
namespace net 
{

class DataException : public std::exception
{
private:
  std::string _msg;

public:
  DataException(std::string) throw();
  virtual ~DataException() throw();
  virtual const char *what() const throw();
};

class SocketException : public std::exception
{
private:
  std::string _msg;

public:
  SocketException(std::string) throw();
  virtual ~SocketException() throw();
  virtual const char *what() const throw();
};

}
}
#endif
