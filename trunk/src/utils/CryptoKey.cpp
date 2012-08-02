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

#include <sys/time.h>

#include <cryptlib.h>
#include <eccrypto.h>
#include <osrng.h>
#include <oids.h>
#include <hex.h>
#include <files.h>
#include <config.h>
#include <modes.h>

#include "CryptoKey.h"

using namespace sai::utils;

CryptoKey::CryptoKey()
{
}

CryptoKey::~CryptoKey()
{
}

//-----------------------------------------------------------------------------

namespace sai 
{
namespace utils
{

class SymmetricKeyImpl
{
public:
  uint64_t v;
  uint64_t c;

public:
  SymmetricKeyImpl(): 
    v(0),
    c(0)
  {}
  ~SymmetricKeyImpl()  
  {}
};

}}

SymmetricKeyImpl * SymmetricKey::_instance = 0;

void     
SymmetricKey::Initialize()
{
  if (!_instance)
  {
    _instance = new SymmetricKeyImpl();
  }
  struct timeval tv;
  gettimeofday(&tv, 0);
  uint64_t val = (tv.tv_sec * 10000000000) + tv.tv_usec;

  _instance->v = _instance->c = val;
}

void 
SymmetricKey::Update()
{
  struct timeval tv;
  gettimeofday(&tv, 0);
  uint64_t val = (tv.tv_sec * 10000000000) + tv.tv_usec;

  _instance->c = val;
}

void     
SymmetricKey::Update(uint32_t c)
{
  _instance->c = _instance->v + c;
}

uint32_t 
SymmetricKey::Offset()
{
  return (_instance->c - _instance->v);
}

std::string 
SymmetricKey::IV()
{
  static std::string ret;
  ret.clear();
  char buf[33];
  sprintf(buf, "%llu", _instance->c);
  ret.assign(buf, strlen(buf)+1);
  return ret;
}

//-----------------------------------------------------------------------------

class AESKey : public SymmetricKey
{
private:
  uint32_t _len;
  byte * _key;
  byte * _iv;
  std::string _sKey;
  std::string _sIV;

public:
  AESKey(uint32_t bits):
    _len (0),
    _key(0),
    _iv(0)
  {
    _len = bits / 8;
    _key = new byte[_len];  
    _iv  = new byte[_len];  
  }

  ~AESKey()
  {
    delete [] _iv;
    delete [] _key;
  }

  std::string getIVString() { _sIV.assign(reinterpret_cast<const char*>(_iv), _len); return _sIV; }
  std::string getKeyString(){ _sKey.assign(reinterpret_cast<const char*>(_key), _len); return _sKey; } 
  byte *      getIVBytes() { return _iv; }
  byte *      getKeyBytes(){ return _key;}
  uint32_t    size() { return (_len); }
  void setIV(std::string iv)
  {
    uint32_t size = iv.size() < _len ? iv.size() : _len;
    memset(_iv, 0, size);
    memcpy(_iv, iv.data(), size);
  }
  void setKey(std::string key) 
  {
    uint32_t size = key.size() < _len ? key.size() : _len;
    memset(_key, 0, size);
    memcpy(_key, key.data(), size);
  }
};

//-----------------------------------------------------------------------------

class ECCKey571 : public AsymmetricKey
{
private:
  CryptoPP::ECIES<CryptoPP::EC2N>::PrivateKey _privateKey;
  CryptoPP::ECIES<CryptoPP::EC2N>::PublicKey  _publicKey;
  std::string _sPrivate;
  std::string _sPublic;

public:
  ECCKey571() {}
  ~ECCKey571() {}
  void generate()
  {
    CryptoPP::AutoSeededRandomPool rng;
    _privateKey.Initialize(rng, CryptoPP::ASN1::sect571r1());
    _privateKey.MakePublicKey(_publicKey);
    if(!_privateKey.Validate(rng, 3))
    {
      // TODO: Throw security exception!
    }
  }
  std::string getPrivateKeyString()
  {
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPrivate));
    _privateKey.Save(encoder);
    return _sPrivate;
  }
  std::string getPublicKeyString()
  {
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPublic));
    _publicKey.Save(encoder);
    return _sPublic;
  }
  void * getPrivateKey() { return (void*)&_privateKey; }
  void * getPublicKey()  { return (void*)&_publicKey;  }
  void setPrivateKey(std::string key)
  {
    CryptoPP::StringSource strPrivateSource(key, true, new CryptoPP::HexDecoder);
    _privateKey.Load(strPrivateSource);
  }
  void setPublicKey(std::string key)
  {
    CryptoPP::StringSource strPublicSource(key, true, new CryptoPP::HexDecoder);
    _publicKey.Load(strPublicSource);
  }
};

//-----------------------------------------------------------------------------

CryptoKeyFactory::CryptoKeyFactory()
{
}

CryptoKeyFactory::~CryptoKeyFactory()
{
}

CryptoKey * 
CryptoKeyFactory::create(CryptoAlgoType type)
{
  switch(type)
  {
    case AES256:
      return new AESKey(256);
    case ECC571:
      return new ECCKey571();
  }
  return 0;
}

