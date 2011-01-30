// SAI [ 19 Oct 2009 ]
#include "Net.h"

using namespace sai::net;

Net::Net(boost::asio::io_service& io):
  _io(io)
{
}

Net::~Net()
{
}

std::string 
Net::getIpFromName(std::string nm)
{
  static std::string ret; // Not a thread-safe var
  ret.clear();

  boost::asio::ip::tcp::resolver resolver(_io);
  boost::asio::ip::tcp::resolver::query query(nm.c_str(), "http");
  boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
  boost::asio::ip::tcp::resolver::iterator end;


  boost::asio::ip::tcp::socket socket(_io);
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

  return ret;
}
