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

#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <iostream>
#include <net2/RawEncoder.h>
#include <net2/DataDescriptor.h>

using namespace sai::net2;

void test1(std::string fileName);
void test2(std::string fileName);
void test3(std::string fileName);

int main(int argc, char * argv[])
{
  test1("test1.dat");
  test2("test2.dat");
  test3("test3.dat");
  return 0;
}

void 
test1(std::string fileName)
{
  std::stringstream txt;
  txt << "<userdata>";
  txt   << "<entry_1 value=\"5\" />";
  txt   << "<entry_2 value=\"hello\" />";
  txt   << "<entry_3 value=\"true\" />";
  txt << "</userdata>";
  std::string data = txt.str();

  DataDescriptor desc;
  desc.raw->encAlgo    = ENC_ALGO_NONE;
  desc.raw->encTagSize = 0;
  desc.raw->comAlgo    = COMP_ALGO_NONE;
  desc.raw->comTagSize = 0;
  desc.raw->hashAlgo   = HASH_ALGO_SHA256;
  desc.raw->hashTagSize= 32;
  // protSize cannot be managed by user
  desc.raw->xmlSize    = data.length();
  desc.raw->binSize    = 0;
  // protData cannot be managed by user
  desc.raw->xmlData    = (char*)data.c_str();
  desc.raw->binData    = 0;

  desc.protMessage->setId(10000);

  uint32_t size = 0;
  RawEncoder encoder;
  char * ptr = encoder.make(desc, size);

  FILE * fp = fopen(fileName.c_str(), "w+");
  if (fp)
  {
    fwrite(ptr, 1, size, fp);
    fclose(fp);
  }
}

void 
test2(std::string fileName)
{
  std::string data = "Hello, world";
  DataDescriptor desc;
  desc.raw->encAlgo    = ENC_ALGO_NONE;
  desc.raw->encTagSize = 0;
  desc.raw->comAlgo    = COMP_ALGO_NONE;
  desc.raw->comTagSize = 0;
  desc.raw->hashAlgo   = HASH_ALGO_SHA256;
  desc.raw->hashTagSize= 32;
  // protSize cannot be managed by user
  desc.raw->xmlSize    = 0;
  desc.raw->binSize    = data.size();
  // protData cannot be managed by user
  desc.raw->xmlData    = 0;
  desc.raw->binData    = (char*)data.data();

  desc.protMessage->setId(10000);

  uint32_t size = 0;
  RawEncoder encoder;
  char * ptr = encoder.make(desc, size);

  FILE * fp = fopen(fileName.c_str(), "w+");
  if (fp)
  {
    fwrite(ptr, 1, size, fp);
    fclose(fp);
  }
}

void 
test3(std::string fileName)
{
  std::stringstream txt;
  txt << "<userdata>";
  txt   << "<entry_4 value=\"8\" />";
  txt   << "<entry_5 value=\"hi\" />";
  txt   << "<entry_6 value=\"false\" />";
  txt << "</userdata>";

  std::string data = txt.str();
  std::string data2 = "Oh My God! It works!";

  DataDescriptor desc;
  desc.raw->encAlgo    = ENC_ALGO_NONE;
  desc.raw->encTagSize = 0;
  desc.raw->comAlgo    = COMP_ALGO_NONE;
  desc.raw->comTagSize = 0;
  desc.raw->hashAlgo   = HASH_ALGO_SHA256;
  desc.raw->hashTagSize= 32;
  // protSize cannot be managed by user
  desc.raw->xmlSize    = data.length();
  desc.raw->binSize    = data2.length();
  // protData cannot be managed by user
  desc.raw->xmlData    = (char*)data.c_str();
  desc.raw->binData    = (char*)data2.c_str();

  desc.protMessage->setId(10000);

  uint32_t size = 0;
  RawEncoder encoder;
  char * ptr = encoder.make(desc, size);

  FILE * fp = fopen(fileName.c_str(), "w+");
  if (fp)
  {
    fwrite(ptr, 1, size, fp);
    fclose(fp);
  }
}

