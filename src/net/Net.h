// SAI [ 10 Oct 2009 ]
#ifndef __SAI_NET__
#define __SAI_NET__
#include <boost/asio.hpp>
#include <stdint.h>
namespace sai { namespace net {

typedef std::vector<std::string*>           StringList;
typedef std::vector<std::string*>::iterator StringListIterator;
typedef std::vector<uint32_t>           IntList;
typedef std::vector<uint32_t>::iterator IntListIterator;

class Net
{
private:
  boost::asio::io_service& _io;

public:
  Net(boost::asio::io_service& io);
  ~Net();

  boost::asio::io_service& getIO() { return _io; }

  std::string getIpFromName(std::string);
};

}}
#endif
