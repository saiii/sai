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

class DataBuffer
{
friend class SenderProfile;
friend class DataOrderingManager;
private:
  time_t         _time; 
  DataDescriptor _desc;
  std::string    _data;
};

typedef std::map<uint32_t, DataBuffer*>              BufferTable;
typedef std::map<uint32_t, DataBuffer*>::iterator    BufferTableIterator;
typedef std::vector<DataBuffer*>                     BufferList;
typedef std::vector<DataBuffer*>::iterator           BufferListIterator;
typedef std::vector<uint32_t>                        IntList;
typedef std::vector<uint32_t>::iterator              IntListIterator;

class SenderProfile : public TimerTask
{
friend class DataOrderingManager;
private:
  bool           _flushMode;
  uint32_t       _sender;
  time_t         _time;
  DataDescriptor _desc;

  uint32_t       _expectedId;
  uint32_t       _lastId;

  BufferTable        _dataTable;
  IntList            _intList;
  DataDispatchable*  _dispatcher;

private:
  SenderProfile(DataDispatchable*);
  ~SenderProfile();
  bool hasMessage(uint32_t);
  void saveMessage(uint32_t);
  void saveMessage(DataDescriptor&, std::string);
  void flushMessage();
  void releaseMessage();
  void requestMissingMessages(uint32_t, uint32_t);

public:
  void timerEvent();
};

typedef std::map<std::string, SenderProfile*>           SenderTable;
typedef std::map<std::string, SenderProfile*>::iterator SenderTableIterator;

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
friend class DataBus;
private:
  static DataOrderingManager* _instance;
  DataDispatchable*           _dispatcher;
  BufferTable                 _outgoingTable;
  BufferList                  _outgoingList;
  SenderTable                 _senderTable;

  DataRequestHandler  _requestHandler;
  DataResponseHandler _responseHandler;

private:
  DataOrderingManager(DataDispatchable*);

public:
  static DataOrderingManager* GetInstance() { return _instance; }
  ~DataOrderingManager();

  void initialize();

  void addOutgoingData(DataDescriptor&, std::string);

  bool check(DataDescriptor&, std::string);

  void timerEvent(); // remove expiring message from the outgoing list & table

  void processDataRequest (DataDescriptor& desc, std::string& msg);
  void processDataResponse(DataDescriptor& desc, std::string& msg);
};

}
}

#endif
