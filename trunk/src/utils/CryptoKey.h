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

#ifndef __SAI_UTILS_CRYPTOKEY__
#define __SAI_UTILS_CRYPTOKEY__

#include <stdint.h>
#include <string>
#include <cryptlib.h>

namespace sai
{
namespace utils
{

typedef enum {
  AES256 = 1,
  ECC521 = 2,
  ECC571 = 3
}CryptoAlgoType;

class CryptoKey
{
public:
  CryptoKey();
  virtual ~CryptoKey();
};

class SymmetricKeyImpl;
class SymmetricKey : public CryptoKey
{
public:
  virtual void        getIVString(std::string& ret) = 0;
  virtual void        getKeyString(std::string& ret) = 0;
  virtual byte *      getIVBytes() = 0;
  virtual byte *      getKeyBytes() = 0;
  virtual void        setIV(std::string iv) = 0;
  virtual void        setKey(std::string key) = 0;
  virtual uint32_t    size() = 0;
  virtual void        randomKey() = 0;
  virtual void        randomIV() = 0;
};

class AsymmetricKey : public CryptoKey
{
public:
  virtual void   generate() = 0;
  virtual void   getPrivateKeyString(std::string& ret) = 0;
  virtual void   getPublicKeyString(std::string& ret) = 0;
  virtual void * getPrivateKey() = 0;
  virtual void * getPublicKey() = 0;
  virtual void   setPrivateKey(std::string key) = 0;
  virtual void   setPublicKey(std::string key) = 0;
};

class CryptoKeyFactory
{
public:
  CryptoKeyFactory();
  ~CryptoKeyFactory();
  CryptoKey * create(CryptoAlgoType);
};

}
}

#endif
