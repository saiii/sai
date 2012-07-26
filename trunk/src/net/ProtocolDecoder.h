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

#ifndef __SAI_NET_PROTOCOLDECODER__
#define __SAI_NET_PROTOCOLDECODER__

#include <net/DataChainable.h>
#include <net/DataDescriptor.h>

namespace sai 
{ 
namespace net 
{

class ProtocolDecoder : public DataHandler
{
protected:
  class MagicToken : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class VersionToken : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class SenderToken : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class IdToken : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class FromToToken : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class Data : public DataChainable
  {
    public: uint32_t decode(DataDescriptor&, std::string&);
  };

  class DefaultDataHandler : public DataHandler
  {
    public: void processDataEvent(DataDescriptor&, std::string&);
  };

protected:
  MagicToken          _magic;
  VersionToken        _version;
  SenderToken         _sender;
  IdToken             _id;
  FromToToken         _fromTo;
  Data                _data;
  DefaultDataHandler* _defaultDataHandler;

public:
  ProtocolDecoder();
  ~ProtocolDecoder();

  void processDataEvent(DataDescriptor&, std::string&);
  bool registerHandler(uint32_t id, DataHandler * handler);
  void setDefaultHandler(DataHandler * handler);
  void activateChecker() { _data.activateChecker(); }
};

}
}

#endif
