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

#ifndef __SAI_NET2_DATAMESSENGER__
#define __SAI_NET2_DATAMESSENGER__

namespace sai
{
namespace net2
{

class Transport;
class Packet;
class DataMessenger
{
friend class DataMessengerFactory;
private:
  Transport * _transport;

private:
  DataMessenger(Transport*);

public:
  virtual ~DataMessenger();
 
  bool send(Packet * packet); 
};

}}

#endif
