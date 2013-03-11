//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
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

#include "Resolver.h"
#include "Net.h"

using namespace sai::net2;

Resolver::Resolver()
{
}

Resolver::~Resolver()
{
}

void
Resolver::getIpFromName(std::string name, std::string& ret)
{
  ret.clear();

  boost::asio::io_service* io = (boost::asio::io_service*)Net::GetInstance()->getIO();
  boost::asio::ip::tcp::resolver resolver(*io);
  boost::asio::ip::tcp::resolver::query query(name.c_str(), "http");
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
  boost::asio::ip::tcp::resolver::iterator end;

  boost::asio::ip::tcp::socket socket(*io);
  boost::system::error_code error = boost::asio::error::host_not_found;
  while (error && iter != end)
  {
    socket.close();
    socket.connect(*iter++, error);
  }

  if (!error)
  {
    ret = socket.remote_endpoint().address().to_string();
  }
}
