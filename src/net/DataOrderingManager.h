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

#ifndef __SAI_NET_DATAORDERINGMANAGER__
#define __SAI_NET_DATAORDERINGMANAGER__

#include <stdint.h>
#include <map>
#include <vector>
#include <string>
#include <net/DataDescriptor.h>
#include <net/DataDispatchable.h>
#include <net/DataHandler.h>
#include <net/TimerTask.h>

namespace sai
{
namespace net
{

typedef enum
{
  SENT,            // Used by sender
  PENDING_REQUEST, // User by receiver, it is a to be requested data
  REQUESTED,       // User by receiver, we are waiting for this piece of data
  READY            // User by receiver, this data is ready but we 
                   // still need to wait the previous packets
}DataState;

class DataOrderingManager;
class DataBuffer
{
friend class DataOrderingManager;
private:
  uint32_t       _time; // it is a real time for sender, but it is used as a counter for receiver
  DataState      _state;
  DataDescriptor _desc;
  std::string    _data;
};

typedef std::map<uint32_t, DataBuffer*>           BufferTable;
typedef std::map<uint32_t, DataBuffer*>::iterator BufferTableIterator;
typedef std::vector<DataBuffer*>                  BufferList;
typedef std::vector<DataBuffer*>::iterator        BufferListIterator;

class DataOrderingManager;
class DataOrderingHandler
{
public:
  DataOrderingManager * manager;
};

class DataRequestHandler : public DataOrderingHandler, public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc, std::string& msg);
};

class DataResponseHandler : public DataOrderingHandler, public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc, std::string& msg);
};

class DataOrderingManager : public TimerTask
{
private:
  DataDispatchable* _dispatcher;
  BufferTable       _outgoingTable;
  BufferList        _outgoingList;
  BufferTable       _incomingTable;
  BufferList        _incomingList;

  uint32_t          _expectedId;
  uint32_t          _lastId;

  DataRequestHandler  _requestHandler;
  DataResponseHandler _responseHandler;

public:
  DataOrderingManager(DataDispatchable*);
  ~DataOrderingManager();

  void initialize();

  void addOutgoingData(DataDescriptor&, std::string);
  void addPendingRequest(DataDescriptor&);
  void addIncomingData(DataDescriptor&, std::string);

  bool check(DataDescriptor&, std::string);

  void timerEvent(); // remove expiring message from the outgoing list & table

  void processDataRequest (DataDescriptor& desc, std::string& msg);
  void processDataResponse(DataDescriptor& desc, std::string& msg);
};

}
}

#endif
