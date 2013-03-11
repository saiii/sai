//=============================================================================
// Copyright (C) 2008 Athip Rooprayochsilp <athipr@gmail.com>
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

#ifndef __SAI_UTILS_CRYPTO__
#define __SAI_UTILS_CRYPTO__

#include <string>

namespace sai
{
namespace utils
{

class OldCrypto
{
public:
  virtual void encrypt(std::string& key, std::string& iv, std::string& data, std::string& output) = 0;
  virtual void decrypt(std::string& key, std::string& iv, std::string& data, std::string& output) = 0;
};

typedef enum {
  AES256 = 1
}OldAlgoType;

class OldCryptoManager
{
private:
  OldCrypto * _crypto;

public:
  OldCryptoManager();
  ~OldCryptoManager();

  OldCrypto * create(OldAlgoType);
};

}
}

#endif
