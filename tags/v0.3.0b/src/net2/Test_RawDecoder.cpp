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

#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <iostream>
#include <net2/RawDecoder.h>
#include <net2/DataDispatcher.h>
#include <net2/DataDescriptor.h>

using namespace sai::net2;

void test(std::string fileName);

namespace sai { namespace net2 {
class DataDispatcherTest
{
public:
  DataDispatcher * dispatcher;

public:
  DataDispatcherTest() { dispatcher = new DataDispatcher(); }
  ~DataDispatcherTest(){ delete dispatcher; }
};
}}

int main(int argc, char * argv[])
{
  printf("Test #1\n");
  test("test1.dat");

  printf("Test #2\n");
  test("test2.dat");

  printf("Test #3\n");
  test("test3.dat");
  return 0;
}

class MyHandler : public DataHandler
{
public:
  void processDataEvent(DataDescriptor& desc)
  {
    desc.print();
  }
};

void 
test(std::string fileName)
{
  FILE * fp = fopen(fileName.c_str(), "r");
  if (!fp)
  {
    return;
  }

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  char * buffer = new char[size];
  long bytes = fread(buffer, 1, size, fp);
  fclose(fp);

  if (bytes != size) 
  {
    fprintf(stderr, "Unable to read all data from file!\n");
  }

  MyHandler handler;
  DataDispatcherTest test;
  RawDecoder decoder(test.dispatcher);
  test.dispatcher->registerHandler(10000, &handler);
  decoder.processData(0, buffer, bytes);
}

