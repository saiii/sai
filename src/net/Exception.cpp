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

#include "Exception.h"

using namespace sai::net;

DataException::DataException(std::string msg) throw():
  _msg(msg)
{
}

DataException::~DataException() throw()
{
}

const char *
DataException::what() const throw()
{
  return _msg.c_str();
}

SocketException::SocketException(std::string msg) throw():
  _msg(msg)
{
}

SocketException::~SocketException() throw()
{
}

const char *
SocketException::what() const throw()
{
  return _msg.c_str();
}

