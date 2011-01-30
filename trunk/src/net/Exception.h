// SAI [ 13 Oct 2009 ]
#ifndef __SAI_NETEXCEPTION__
#define __SAI_NETEXCEPTION__

#include <string>
#include <exception>

namespace sai { namespace net {

class DataException : public std::exception
{
private:
  std::string _msg;

public:
  DataException(std::string) throw();
  virtual ~DataException() throw();
  virtual const char *what() const throw();
};

class SocketException : public std::exception
{
private:
  std::string _msg;

public:
  SocketException(std::string) throw();
  virtual ~SocketException() throw();
  virtual const char *what() const throw();
};

}}
#endif
