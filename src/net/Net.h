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

#ifndef __SAI_NET_NET__
#define __SAI_NET_NET__

#include <boost/asio.hpp>
#include <stdint.h>

namespace sai 
{ 
namespace net 
{

// SAI : TODO Change all std::vector to sai::utils::List
typedef std::vector<std::string*>           StringList;
typedef std::vector<std::string*>::iterator StringListIterator;
typedef std::vector<uint32_t>           IntList;
typedef std::vector<uint32_t>::iterator IntListIterator;

class Net
{
private:
  boost::asio::io_service& _io;
  static Net * _instance;
  char         _sender[17];
  uint32_t     _id;

public:
  Net(boost::asio::io_service& io);
  ~Net();
  static Net * GetInstance();
  char *       getSenderId() { return _sender; }

  boost::asio::io_service& getIO() { return _io; }

  std::string getIpFromName(std::string);
  uint32_t    getMessageId();
};

}
}
#endif
