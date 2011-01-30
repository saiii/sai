// SAI [ 13 Oct 2009 ]
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

