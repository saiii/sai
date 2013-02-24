//=============================================================================
// Copyright (C) 2012 Athip Rooprayochsilp <athipr@gmail.com>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//=============================================================================

#ifdef _WIN32
#include <Winsock2.h>
#include <ws2tcpip.h> 
#else
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#endif

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>

#include <utils/Logger.h>
#include <utils/ThreadPool.h>

#include <net2/Net.h>
#include <net2/PGMSocket.h>
#include "Transport.h"

#ifdef _WIN32
#include <Objbase.h>
#endif

#define NET_BUFFER_SIZE1  40960
#define NET_BUFFER_SIZE2 819200

using namespace sai::net2;

DirectDb::DirectDb()
{
}

DirectDb::~DirectDb()
{
}

void 
DirectDb::add(DirectReply* d)
{
  _list.push_back(d);
}

void 
DirectDb::drop(DirectReply* d)
{
  DirectReplyListIterator iter;
  for (iter  = _list.begin(); 
       iter != _list.end();
       iter ++)
  {
    DirectReply * dp = *iter;
    if (dp == d)
    {
      _list.erase(iter);
      break;
    }
  }
}

bool 
DirectDb::has(DirectReply* d)
{
  if (_list.size() <= 0) return false;

  DirectReplyListIterator iter;
  for (iter  = _list.begin(); 
       iter != _list.end();
       iter ++)
  {
    DirectReply * dp = *iter;
    if (dp == d)
    {
      return true;
    }
  }
  return false;
}

Endpoint::Endpoint(std::string addr, uint32_t p, IncomingSource source, TCPPtr* ptr):
  _port(p),
  _source(source),
  _tcpPtr(0, 0)
{
  _tcpPtr.obj = 0;
  if (ptr)
  {
    _tcpPtr.db  = ptr->db;
    _tcpPtr.obj = ptr->obj;
  }
  _addr.assign(addr.c_str(), addr.length()+1);
}

Endpoint::~Endpoint()
{
}

Transport::Transport()
{
}

Transport::~Transport()
{
}

void 
Transport::bind(std::string ip)
{
}

void 
Transport::open()
{
}

namespace sai
{
namespace net2
{

class BufferManager
{
private:
  char*                         buffer;
  char*                         buffer2;
  uint32_t                      &buffer2Size;

public:
  RawDataHandler*               handler; 

public:
  BufferManager(char * buf, char * buf2, uint32_t& size, RawDataHandler * hdlr);
  ~BufferManager();
  void processBuffer(Endpoint& endpoint, char * ptr, int32_t bytes_received);
};

BufferManager::BufferManager(char * buf, char * buf2, uint32_t& size, RawDataHandler * hdlr):
  buffer(buf),
  buffer2(buf2),
  buffer2Size(size),
  handler(hdlr)
{
}

BufferManager::~BufferManager()
{
}

void 
BufferManager::processBuffer(Endpoint& endpoint, char * ptr, int32_t bytes_received)
{
  if (buffer2Size) 
  {
    memcpy(buffer2+buffer2Size, buffer, bytes_received);
    buffer2Size += bytes_received;

    memcpy(buffer, buffer2, buffer2Size);
    bytes_received = buffer2Size;
    buffer2Size = 0;
  }

  uint32_t size = 0;
  memcpy(&size, buffer+6, sizeof(size));
  size  = ntohl(size);

  if (size == 0)
  {
	  return;
  }

#ifdef DEBUG_NET
  uint16_t seqNo = 0;
  memcpy(&seqNo, buffer+10, sizeof(seqNo));
  seqNo = ntohs(seqNo);
#endif

  if (size < (uint32_t)bytes_received)
  {
    char *ptr = buffer;
    do
    {
      //printf("[1] Got %u Size %u Receive %u\n", seqNo, size, bytes_received);
      handler->processDataEvent(&endpoint, ptr, size);
      ptr += size;
      bytes_received -= size;

      memcpy(&size, ptr+6, sizeof(size));
      size  = ntohl(size);

#ifdef DEBUG_NET
      memcpy(&seqNo, ptr+10, sizeof(seqNo));
      seqNo = ntohs(seqNo);
#endif
    }while(bytes_received > size);

    if (bytes_received != size)
    {
      memcpy(buffer2+buffer2Size, ptr, bytes_received);
      buffer2Size += bytes_received;
    }
    else
    {
      //printf("[2] Got %u Size %u Receive %u\n", seqNo, size, bytes_received);
      handler->processDataEvent(&endpoint, ptr, size);
    }
  }
  else if (size == bytes_received)
  {
    //printf("[3] Got %u Size %u Receive %u\n", seqNo, size, bytes_received);
    handler->processDataEvent(&endpoint, buffer, bytes_received);
  }
  else
  {
    memcpy(buffer2+buffer2Size, buffer, bytes_received);
    buffer2Size += bytes_received;
  }
  //fflush(stdout);
}

typedef boost::asio::ssl::stream<boost::asio::ip::tcp::socket> ssl_socket;

class Session : public DirectReply
{
private:
  RawDataHandler*               handler; 

public:
  char*                         buffer;
  char*                         buffer2;
  uint32_t                      buffer2Size;
  boost::asio::ip::tcp::socket *sckt;
  ssl_socket                   *sslSckt;
  BufferManager*                manager;
  static DirectDb *             db;

public:
  Session(boost::asio::ssl::context* context, RawDataHandler * hdlr):
    handler(hdlr),
    buffer(0),
    buffer2(0),
    buffer2Size(0),
    sckt(0),
    sslSckt(0),
    manager(0)
  {
    if (!db)
    {
      db = new DirectDb();
    }
    db->add(this);

    if (!context)
    {
      sckt = new boost::asio::ip::tcp::socket(*((boost::asio::io_service*)Net::GetInstance()->getIO()));
    }
    else
    {
      sslSckt = new ssl_socket(*((boost::asio::io_service*)Net::GetInstance()->getIO()), *context);
    }
#ifdef _WIN32
    buffer = (char*)::CoTaskMemAlloc(NET_BUFFER_SIZE2);
    buffer2= (char*)::CoTaskMemAlloc(NET_BUFFER_SIZE2);
#else
    buffer = new char[NET_BUFFER_SIZE2];
    buffer2= new char[NET_BUFFER_SIZE2];
#endif
    manager = new BufferManager(buffer, buffer2, buffer2Size, handler);
  }
  ~Session()
  {
    if (_dhandler)
    {
      _dhandler->destroyEvent();
    }
    delete sckt;
    delete sslSckt;
	sckt = 0;
	sslSckt = 0;
#ifdef _WIN32
    ::CoTaskMemFree(buffer2);
    ::CoTaskMemFree(buffer);
#else
    delete [] buffer2;
    delete [] buffer;
#endif
    delete manager;
    db->drop(this);
  }
  ssl_socket::lowest_layer_type& socket()
  {
    return sslSckt->lowest_layer();
  }

  void start()
  {
    if (sckt)
    {
      sckt->async_read_some(boost::asio::buffer(buffer, NET_BUFFER_SIZE1),
        boost::bind(&Session::processDataEvent, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
    else
    {
      sslSckt->async_read_some(boost::asio::buffer(buffer, NET_BUFFER_SIZE1),
        boost::bind(&Session::processDataEvent, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
    }
  }
  void processDataEvent(const boost::system::error_code& err, size_t bytes_received)
  {
    if (!err)
    {
      if (handler) 
      {
        boost::asio::ip::tcp::endpoint ep = sckt ? sckt->remote_endpoint() : socket().remote_endpoint();
        TCPPtr ptr(db, this);
        Endpoint endpoint(ep.address().to_string(), ep.port(), TCP, &ptr);
        PerfMeasure::InCount++;

        manager->processBuffer(endpoint, buffer, bytes_received);
      }
      start();
    }
    else
    {
      // FIXME : We may need to start listening again
      delete this;
    }
  }

  void send(char * data, int size)
  {
    std::string message;
    boost::system::error_code ignored_error;

    bool isOpen = sckt ? sckt->is_open() : socket().is_open();
    if (!isOpen)
    {
      return;
    }

#if DEBUG_NET
    static uint16_t seqNo = 0;
    seqNo++;
    printf("Seq %u, Size %u\n", seqNo, size);
    uint16_t nSeqNo = htons(seqNo);
    memcpy((char*)(data + 10), &nSeqNo, sizeof(nSeqNo));
#endif

    message.append(data, size);

    if (sckt)
    {
      int32_t bytes = boost::asio::write(*sckt, boost::asio::buffer(message), ignored_error);
      if (bytes != message.size())
      {
        if (bytes == 0)
        {
          delete this;
        }
      }
    }
    else
    {
      char * nmsg = new char[size];
      DataSentHandler * handler = new DataSentHandler();
      handler->msg  = nmsg;
      handler->impl = this;
      memcpy(nmsg, data, size);
      boost::asio::async_write(*sslSckt, boost::asio::buffer(nmsg, size),
          boost::bind(&Session::DataSentHandler::processDataSent, handler,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }
  class DataSentHandler
  {
    public:
      Session * impl;
      char    * msg;

    public:
      void processDataSent(const boost::system::error_code& error, size_t bytes_transferred)
      {
        if (error)
        {
          delete [] msg;
          delete this;
          delete impl;
          return;
        }
        delete [] msg;
        delete this;
      }
  };
};

DirectDb * Session::db = 0;

class InternalTransportImpl
{
public:
  // TCP Stuff
  boost::asio::ip::tcp::acceptor* acceptor;
  boost::asio::ssl::context     * context;
  PGMSocket                     * localSckt;
  RawDataHandler                * handler; 

public:
  InternalTransportImpl():
    acceptor(0),
    context(0),
    handler(0),
    localSckt(0)
  {
  }

  ~InternalTransportImpl()
  {
    delete localSckt;
  }

  void initialize(NetworkOptions* opt, RawDataHandler * hndlr)
  {
    handler = hndlr;

    if (!opt->usePGM())
    {
      // TCP Stuff
      try{
        boost::asio::ip::tcp::endpoint endpoint;
        const boost::asio::ip::address address = boost::asio::ip::address::from_string(opt->getInterface());
        endpoint.address(address);
        endpoint.port(opt->getPort());
  
        acceptor = new boost::asio::ip::tcp::acceptor(
          *((boost::asio::io_service*)Net::GetInstance()->getIO()), endpoint);
        acceptor->set_option(boost::asio::ip::tcp::socket::reuse_address(true));
    
        if (opt->enableSSL())
        {
          context = new boost::asio::ssl::context(boost::asio::ssl::context::sslv23);
          context->set_options(boost::asio::ssl::context::default_workarounds | 
                               boost::asio::ssl::context::no_sslv2 | 
                               boost::asio::ssl::context::single_dh_use);
          context->set_password_callback(boost::bind(&InternalTransportImpl::getPassword, this));
          context->use_certificate_chain_file("server.pem");
          context->use_private_key_file("server.pem", boost::asio::ssl::context::pem);
          context->use_tmp_dh_file("dh512.pem");
        }
        Session * session = new Session(context, handler);
  
        acceptor->async_accept(session->sckt ? *session->sckt : session->socket(),
          boost::bind(&InternalTransportImpl::processConnectionRequestEvent,
                      this, session, boost::asio::placeholders::error));
      }
      catch(boost::system::system_error& e) 
      {
        fprintf(stderr, "Error %s\n", e.what());
      }
    }

    // PGM Stuff
    if (opt->usePGM())
    {
      localSckt = PGMSocketFactory::GetInstance()->create(opt);
      localSckt->setEventHandler(handler);
      localSckt->listen();
    }
  }

  std::string 
  getPassword() const
  {
    return "dummy";
  }

  void processConnectionRequestEvent(Session * session, const boost::system::error_code& err) // TCP Only
  {
    if (!err)
    {
      boost::asio::ip::tcp::no_delay option(true);
      if (session->sckt)
      {
        session->sckt->set_option(option);
      }
      else
      {
        session->socket().set_option(option);
      }

      session->manager->handler = handler;
      session->start();
    }
    else
    {
      delete session;
	  session = 0;
    }

    Session * sess = new Session(context, handler);

    acceptor->async_accept(sess->sckt ? *sess->sckt : sess->socket(),
      boost::bind(&InternalTransportImpl::processConnectionRequestEvent,
                    this, sess, boost::asio::placeholders::error));
  }

  void close()
  {
    // TCP Stuff
    {
      if (acceptor)
      {
        try
        {
          acceptor->cancel();
          acceptor->close();
          delete acceptor;
          acceptor = 0;
        } 
        catch(boost::system::system_error& e)
        {
#ifndef _WIN32
          openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
          syslog(LOG_ERR, "Failed to close InternalTransportImpl");
          syslog(LOG_ERR, "%s", e.what());
          closelog();
#endif
        }
      }
    }

    // PGM Stuff
    {
      if (localSckt)
      {
        localSckt->close();
      }
    }
  }
};

}}

InternalTransport::InternalTransport():
  _impl(0),
  _handler(0)
{
  _impl = new InternalTransportImpl();
}

InternalTransport::~InternalTransport()
{
  _impl->close();
  delete _impl;
}

void 
InternalTransport::initialize(NetworkOptions* opt, RawDataHandler * handler)
{
  _impl->initialize(opt, handler);
}

uint32_t PerfMeasure::IncomingTime = 0;
uint32_t PerfMeasure::OutgoingTime = 0;
uint32_t PerfMeasure::InCount = 0;
uint32_t PerfMeasure::OutCount = 0;

