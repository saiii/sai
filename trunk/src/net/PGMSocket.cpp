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

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#endif

#ifndef _WIN32
#include <syslog.h>
#endif
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <utils/ThreadPool.h>
#include <pgm/pgm.h>
#include <pgm/http.h>
#include "PGMSocket.h"

#define NET_BUFFER_SIZE 81920

using namespace sai::net;

NetworkOptions::NetworkOptions():
  _port(0),
  _withHttp(false)
{
}

NetworkOptions::~NetworkOptions()
{
  while (_receive.size() > 0)
  {
    char * ch = _receive.front();
    _receive.erase(_receive.begin());
    free(ch);
  }
}

void 
NetworkOptions::setInterface(std::string intf)
{
  _interface = intf;
}

void     
NetworkOptions::addReceive(std::string rec)
{
  _receive.push_back(strdup(rec.c_str()));
}

void     
NetworkOptions::setSend(std::string snd)
{
  _send = snd;
}

void 
NetworkOptions::toString(std::string& ret)
{
  ret.clear();
  ret.append(_interface);
  ret.append(";");

  if (_receive.size() == 1 && strcmp(_receive.at(0), _send.c_str()) == 0)
  {
    ret.append(_send);
  }
  else
  {
    std::vector<char *>::iterator iter;
    for (iter  = _receive.begin();
         iter != _receive.end();
         iter ++)
    {
      char * ch = *iter;
      ret.append(ch);
      if (iter + 1 != _receive.end())
      {
        ret.append(",");
      }
    }
    ret.append(";");
    ret.append(_send);
  }
}

void
Initialize()
{
  pgm_error_t* err = 0;
  pgm_messages_init();
  if (!pgm_init (&err)) {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "Failed to initialize (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    pgm_messages_shutdown();
    return;
  }
}

void
Shutdown()
{
  pgm_shutdown();
  pgm_messages_shutdown();
}


uint8_t PGMSocket::_InitCnt = 0;

PGMSocket::PGMSocket(NetworkOptions* options):
  _handler(0),
  _scktRecv(0),
  _scktSend(0),
  _addrInfo(0),
  _receiver(0)
{
  if (_InitCnt == 0)
  {
    Initialize();
  }
  _InitCnt++;

  std::string network;
  options->toString(network);

  pgm_error_t* err = 0;
  if (options->enableHttp())
  {
    //if (!pgm_http_init(2010, &err))
    //{
    //  printf("Initializing http interface error %s\n", err->message);
    //  pgm_error_free(err);
    //}
    //else
    //{
    //  printf("Initializing http interface at port %d\n", 2010);
    //}
  }

  if (!pgm_getaddrinfo (network.c_str(), 0, &_addrInfo, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_getaddrinfo (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }

  sa_family_t family = _addrInfo->ai_send_addrs[0].gsr_group.ss_family;
  if (!pgm_socket (&_scktRecv, family, SOCK_SEQPACKET, IPPROTO_UDP, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_socket (recv) (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }
  if (!pgm_socket (&_scktSend, family, SOCK_SEQPACKET, IPPROTO_UDP, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_socket (send) (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }

  int32_t port = options->getPort();
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_UDP_ENCAP_UCAST_PORT, &port, sizeof(port));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_UDP_ENCAP_MCAST_PORT, &port, sizeof(port));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_UDP_ENCAP_UCAST_PORT, &port, sizeof(port));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_UDP_ENCAP_MCAST_PORT, &port, sizeof(port));

  const int no_router_assist = 0;
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_IP_ROUTER_ALERT, &no_router_assist, sizeof(no_router_assist));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_IP_ROUTER_ALERT, &no_router_assist, sizeof(no_router_assist));

  //pgm_drop_superuser(); // TODO check this one again

  const int ambient_spm = pgm_secs (30),
            heartbeat_spm[] = { pgm_msecs (100),
                                pgm_msecs (100),
                                pgm_msecs (100),
                                pgm_msecs (100),
                                pgm_msecs (1300),
                                pgm_secs  (7),
                                pgm_secs  (16),
                                pgm_secs  (25),
                                pgm_secs  (30) };
  const int g_max_tpdu = 1500;
  const int g_sqns = 100;
  const int g_max_rte = 10 * 1024 * 1024;
  const int peer_expiry = pgm_secs (300);
  const int spmr_expiry = pgm_msecs (250);
  const int nak_bo_ivl = pgm_msecs (50);
  const int nak_rpt_ivl = pgm_secs (2);
  const int nak_rdata_ivl = pgm_secs (2);
  const int nak_data_retries = 50;
  const int nak_ncf_retries = 50;
  const int recv_only = 1;
  const int send_only = 1;
  const int passive = 0;

  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_TXW_SQNS, &g_sqns, sizeof(g_sqns));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_TXW_MAX_RTE, &g_max_rte, sizeof(g_max_rte));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_SEND_ONLY, &send_only, sizeof(send_only));

  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_MTU, &g_max_tpdu, sizeof(g_max_tpdu));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_AMBIENT_SPM, &ambient_spm, sizeof(ambient_spm));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_HEARTBEAT_SPM, &heartbeat_spm, sizeof(heartbeat_spm));

  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_RECV_ONLY, &recv_only, sizeof(recv_only));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_PASSIVE, &passive, sizeof(passive));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_MTU, &g_max_tpdu, sizeof(g_max_tpdu));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_RXW_SQNS, &g_sqns, sizeof(g_sqns));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_PEER_EXPIRY, &peer_expiry, sizeof(peer_expiry));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_SPMR_EXPIRY, &spmr_expiry, sizeof(spmr_expiry));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NAK_BO_IVL, &nak_bo_ivl, sizeof(nak_bo_ivl));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NAK_RPT_IVL, &nak_rpt_ivl, sizeof(nak_rpt_ivl));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NAK_RDATA_IVL, &nak_rdata_ivl, sizeof(nak_rdata_ivl));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NAK_DATA_RETRIES, &nak_data_retries, sizeof(nak_data_retries));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NAK_NCF_RETRIES, &nak_ncf_retries, sizeof(nak_ncf_retries));

  bool useFec = false;
  const uint32_t BLOCK_SIZE = 255;
  const uint32_t GROUP_SIZE = 8;
  if (useFec) 
  {
    struct pgm_fecinfo_t fecinfo;
    fecinfo.block_size              = BLOCK_SIZE;
    fecinfo.proactive_packets       = 0;
    fecinfo.group_size              = GROUP_SIZE;
    fecinfo.ondemand_parity_enabled = true;
    fecinfo.var_pktlen_enabled      = true;
    pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_USE_FEC, &fecinfo, sizeof(fecinfo));
  }

  // BIND
  const int32_t fixPort = 8080 + port; // the pgm logical port is fixed
  struct pgm_sockaddr_t addr;
  memset (&addr, 0, sizeof(addr));
  addr.sa_port = fixPort;
  addr.sa_addr.sport = DEFAULT_DATA_SOURCE_PORT; // Let's random
  if (!pgm_gsi_create_from_hostname (&addr.sa_addr.gsi, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_gsi_create_from_hostname (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }

  struct pgm_interface_req_t ifReq;
  memset (&ifReq, 0, sizeof(ifReq));
  ifReq.ir_interface = _addrInfo->ai_recv_addrs[0].gsr_interface;
  ifReq.ir_scope_id  = 0;
  if (AF_INET6 == family) 
  {
    struct sockaddr_in6 sa6;
    memcpy (&sa6, &_addrInfo->ai_recv_addrs[0].gsr_group, sizeof(sa6));
    ifReq.ir_scope_id = sa6.sin6_scope_id;
  }
  if (!pgm_bind3 (_scktRecv,
                  &addr, sizeof(addr),
                  &ifReq, sizeof(ifReq),        /* tx interface */
                  &ifReq, sizeof(ifReq),        /* rx interface */
                  &err))
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_bind3 (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }
  if (!pgm_bind3 (_scktSend,
                  &addr, sizeof(addr),
                  &ifReq, sizeof(ifReq),        /* tx interface */
                  &ifReq, sizeof(ifReq),        /* rx interface */
                  &err))
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_bind3 (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }

  // Join
  for (unsigned i = 0; i < _addrInfo->ai_recv_addrs_len; i++)
  {
    pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_JOIN_GROUP, &_addrInfo->ai_recv_addrs[i], sizeof(struct group_req));
    pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_JOIN_GROUP, &_addrInfo->ai_recv_addrs[i], sizeof(struct group_req));
  }
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_SEND_GROUP, &_addrInfo->ai_send_addrs[0], sizeof(struct group_req));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_SEND_GROUP, &_addrInfo->ai_send_addrs[0], sizeof(struct group_req));
  pgm_freeaddrinfo (_addrInfo);

  const int nonblocking = 1;
  const int blocking = 0;
  const int multicast_loop = 1;
  const int multicast_hops = 16;
  const int dscp = 0x2e << 2; 
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_MULTICAST_LOOP, &multicast_loop, sizeof(multicast_loop));
  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_MULTICAST_HOPS, &multicast_hops, sizeof(multicast_hops));

  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_MULTICAST_LOOP, &multicast_loop, sizeof(multicast_loop));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_MULTICAST_HOPS, &multicast_hops, sizeof(multicast_hops));
  if (AF_INET6 != family)
  {
    pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_TOS, &dscp, sizeof(dscp));
    pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_TOS, &dscp, sizeof(dscp));
  }

  pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NOBLOCK, &nonblocking, sizeof(nonblocking));
  //pgm_setsockopt (_scktRecv, IPPROTO_PGM, PGM_NOBLOCK, &blocking, sizeof(blocking));
  pgm_setsockopt (_scktSend, IPPROTO_PGM, PGM_NOBLOCK, &blocking, sizeof(blocking));

}

PGMSocket::~PGMSocket()
{
  _InitCnt--;
  if (_InitCnt == 0)
  {
    Shutdown();
  }
}

void 
PGMSocket::listen()
{
  pgm_error_t* err = 0;
  if (!pgm_connect(_scktRecv, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "listen (recv) (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }
  _receiver->schedule();

  if (!pgm_connect(_scktSend, &err)) 
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "listen (send) (%s)", err->message);
    closelog();
#endif
    pgm_error_free(err);
    return;
  }
}

void 
PGMSocket::send(const char *data, uint32_t size)
{
  const int status = pgm_send (_scktSend, data, size, 0);
  if (PGM_IO_STATUS_NORMAL != status)
  {
#ifndef _WIN32
    openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
    syslog(LOG_ERR, "pgm_send failed");
    closelog();
#endif
  }
}

void 
PGMSocket::close()
{
  if (_scktRecv)
  {
    pgm_close (_scktRecv, true);
    _scktRecv = 0;
  }
  if (_scktSend)
  {
    pgm_close (_scktSend, true);
    _scktSend = 0;
  }
}

void 
PGMSocket::setEventHandler(PGMSocketEventHandler * h) 
{ 
  _handler = h; 
  _receiver = new Receiver(_scktRecv, _handler); 
}

Receiver::Receiver(pgm_sock_t* sckt, PGMSocketEventHandler* handler):
  _sckt(sckt),
  _handler(handler),
  _running(true)
{
}

Receiver::~Receiver()
{
}

void 
Receiver::threadEvent()
{
  char* buffer = new char[NET_BUFFER_SIZE];

  pgm_error_t* err = 0;
  struct pgm_sockaddr_t from;
  socklen_t fromlen = sizeof(from);
  size_t len;

#ifndef _WIN32
  int fds;
  fd_set readfds;
  int g_quit_pipe[2];
  pipe(g_quit_pipe);
#else
  WSAEVENT g_quit_event;
  g_quit_event = WSACreateEvent();
  SOCKET recv_sock, pending_sock;
  DWORD cEvents = PGM_RECV_SOCKET_READ_COUNT + 1;
  WSAEVENT waitEvents[ 3 ];
  socklen_t socklen = sizeof (SOCKET);

  waitEvents[0] = g_quit_event;
  waitEvents[1] = WSACreateEvent();
  waitEvents[2] = WSACreateEvent();
  assert (2 == PGM_RECV_SOCKET_READ_COUNT);
  pgm_getsockopt (_sckt, IPPROTO_PGM, PGM_RECV_SOCK, &recv_sock, &socklen);
  WSAEventSelect (recv_sock, waitEvents[1], FD_READ);
  pgm_getsockopt (_sckt, IPPROTO_PGM, PGM_PENDING_SOCK, &pending_sock, &socklen);
  WSAEventSelect (pending_sock, waitEvents[2], FD_READ);

  DWORD dwTimeout, dwEvents;
#endif 

  struct timeval tv;

  while (_running)
  {
    const int status = pgm_recvfrom (_sckt,
                                     buffer,
                                     NET_BUFFER_SIZE,
                                     0,
                                     &len,
                                     &from,
                                     &fromlen,
                                     &err);
    switch(status)
    {
      case PGM_IO_STATUS_NORMAL:
        // TODO deal with from
        printf("processData\n");
        _handler->processDataEvent(buffer, (uint32_t)len);
        break;
      case PGM_IO_STATUS_TIMER_PENDING:
        {
          struct timeval tv;
          socklen_t optlen = sizeof (tv);
          pgm_getsockopt (_sckt, IPPROTO_PGM, PGM_TIME_REMAIN, &tv, &optlen);

#ifndef _WIN32
          fds = g_quit_pipe[0] + 1;
          FD_ZERO(&readfds);
          FD_SET(g_quit_pipe[0], &readfds);
          pgm_select_info (_sckt, &readfds, 0, &fds);
          fds = select(fds, &readfds, 0, 0, &tv);
#else
          dwTimeout = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
          dwEvents = WSAWaitForMultipleEvents (cEvents, waitEvents, FALSE, dwTimeout, FALSE);
          switch (dwEvents) {
            case WSA_WAIT_EVENT_0+1: WSAResetEvent (waitEvents[1]); break;
            case WSA_WAIT_EVENT_0+2: WSAResetEvent (waitEvents[2]); break;
            default: break;
          }
#endif
        }
        break;
      case PGM_IO_STATUS_RATE_LIMITED:
        {
          struct timeval tv;
          socklen_t optlen = sizeof (tv);
          pgm_getsockopt (_sckt, IPPROTO_PGM, PGM_RATE_REMAIN, &tv, &optlen);

#ifndef _WIN32
          fds = g_quit_pipe[0] + 1;
          FD_ZERO(&readfds);
          FD_SET(g_quit_pipe[0], &readfds);
          pgm_select_info (_sckt, &readfds, 0, &fds);
          fds = select(fds, &readfds, 0, 0, &tv);
#else
          dwTimeout = ((tv.tv_sec * 1000) + (tv.tv_usec / 1000));
          dwEvents = WSAWaitForMultipleEvents (cEvents, waitEvents, FALSE, dwTimeout, FALSE);
          switch (dwEvents) {
            case WSA_WAIT_EVENT_0+1: WSAResetEvent (waitEvents[1]); break;
            case WSA_WAIT_EVENT_0+2: WSAResetEvent (waitEvents[2]); break;
            default: break;
          }
#endif
        }
        break;
      case PGM_IO_STATUS_WOULD_BLOCK:
#ifndef _WIN32
        fds = g_quit_pipe[0] + 1;
        FD_ZERO(&readfds);
        FD_SET(g_quit_pipe[0], &readfds);
        pgm_select_info (_sckt, &readfds, 0, &fds);
        tv.tv_sec  = 1;
        tv.tv_usec = 0;
        fds = select(fds, &readfds, 0, 0, &tv);
        printf("would block\n");
#else
        dwTimeout = WSA_INFINITE;
        dwEvents = WSAWaitForMultipleEvents (cEvents, waitEvents, FALSE, dwTimeout, FALSE);
        switch (dwEvents) {
          case WSA_WAIT_EVENT_0+1: WSAResetEvent (waitEvents[1]); break;
          case WSA_WAIT_EVENT_0+2: WSAResetEvent (waitEvents[2]); break;
          default: break;
        }
#endif
        break;
      default:
        if (err)
        {
#ifndef _WIN32
          openlog("sai", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1);
          syslog(LOG_ERR, "pgm_recvfrom failed (%s)", err->message);
          if (status == PGM_IO_STATUS_ERROR)
          {
            syslog(LOG_ERR, "pgm_recvfrom PGM_IO_STATUS_ERROR");
          }
          closelog();
#endif
          pgm_error_free(err);
        }
        err = 0;
        break;
    }
  }
  delete [] buffer;
}

