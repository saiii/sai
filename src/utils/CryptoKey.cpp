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

#ifdef _WIN32
#include <sys_time.h>
#else
#include <sys/time.h>
#endif

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

  void getIVString(std::string& ret) { _sIV.assign(reinterpret_cast<const char*>(_iv), _len); ret = _sIV; }
  void getKeyString(std::string& ret){ _sKey.assign(reinterpret_cast<const char*>(_key), _len); ret = _sKey; } 
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
  void randomKey()
  {
    CryptoPP::AutoSeededRandomPool rnd;
    rnd.GenerateBlock(_key, _len);
  }
  void randomIV()
  {
    CryptoPP::AutoSeededRandomPool rnd;
    rnd.GenerateBlock(_iv, _len);
  }
};

//-----------------------------------------------------------------------------

class EccKey571 : public AsymmetricKey
{
private:
  CryptoPP::ECIES<CryptoPP::EC2N>::PrivateKey _privateKey;
  CryptoPP::ECIES<CryptoPP::EC2N>::PublicKey  _publicKey;
  std::string _sPrivate;
  std::string _sPublic;

public:
  EccKey571() {}
  ~EccKey571() {}
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
  void getPrivateKeyString(std::string& ret)
  {
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPrivate));
    _privateKey.Save(encoder);
    ret = _sPrivate;
  }
  void getPublicKeyString(std::string& ret)
  {
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPublic));
    _publicKey.Save(encoder);
    ret = _sPublic;
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

class EccKey521 : public AsymmetricKey
{
private:
  CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey _privateKey;
  CryptoPP::ECIES<CryptoPP::ECP>::PublicKey  _publicKey;
  std::string _sPrivate;
  std::string _sPublic;

public:
  EccKey521() {}
  ~EccKey521() {}
  void generate()
  {
    CryptoPP::AutoSeededRandomPool rng;
    _privateKey.Initialize(rng, CryptoPP::ASN1::secp521r1());
    _privateKey.MakePublicKey(_publicKey);
    if(!_privateKey.Validate(rng, 3))
    {
      // TODO: Throw security exception!
    }
  }
  void getPrivateKeyString(std::string& ret)
  {
    _sPrivate.clear();
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPrivate));
    _privateKey.Save(encoder);
    ret = _sPrivate;
  }
  void getPublicKeyString(std::string& ret)
  {
    _sPublic.clear();
    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(_sPublic));
    _publicKey.Save(encoder);
    ret = _sPublic;
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
    case ECC521: // ECP
      return new EccKey521();
    case ECC571: // EC2N
      return new EccKey571();
  }
  return 0;
}

