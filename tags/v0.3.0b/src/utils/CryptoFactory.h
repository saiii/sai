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

#ifndef __SAI_UTILS_CRYPTOFACTORY__
#define __SAI_UTILS_CRYPTOFACTORY__

#include <stdint.h>
#include <string>
#include <utils/CryptoKey.h>

namespace sai
{
namespace utils
{

class Crypto
{
public:
  Crypto();
  virtual ~Crypto();
};

class SymmetricCrypto : public Crypto
{
private:
  uint32_t size;

public:
  SymmetricCrypto();
  virtual ~SymmetricCrypto();
  virtual void encrypt(SymmetricKey& key, std::string data, std::string& output) = 0;
  virtual void decrypt(SymmetricKey& key, std::string data, std::string& output) = 0;
};

class AsymmetricCrypto : public Crypto
{
public:
  AsymmetricCrypto();
  virtual ~AsymmetricCrypto();
  virtual void encrypt(AsymmetricKey& key, std::string data, std::string& output) = 0;
  virtual void decrypt(AsymmetricKey& key, std::string data, std::string& output) = 0;
};

class CryptoFactory
{
public:
  CryptoFactory();
  ~CryptoFactory();

  Crypto * create(CryptoAlgoType);
};

}
}

#endif
