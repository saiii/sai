//=============================================================================
// Copyright (C) 2009 Athip Rooprayochsilp <athipr@gmail.com>
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

#include <stdint.h>
#include <iostream>

#include <cryptlib.h>
#include <base64.h>
#include <hex.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>

#include "Crypto.h"

using namespace sai::utils;

class CryptoRijindael : public Crypto
{
public:
  CryptoRijindael() {}
  ~CryptoRijindael() {}
  void encrypt(std::string& key, std::string& iv, std::string& data, std::string& output);
  void decrypt(std::string& key, std::string& iv, std::string& data, std::string& output);
};

CryptoManager::CryptoManager():
  _crypto(0)
{}

CryptoManager::~CryptoManager()
{
  delete _crypto;
}

Crypto * 
CryptoManager::create(AlgoType t)
{
  switch (t)
  {
    case AES256:
      _crypto = new CryptoRijindael(); 
      break;
  }
  return _crypto;
}

void 
CryptoRijindael::encrypt(std::string& key, std::string& iv, std::string& data, std::string& output) 
{
  const int SIZE = 32;
  byte _key[SIZE];
  byte _iv [SIZE];
  memset(_key, 0x01, SIZE);
  memset(_iv,  0x01, SIZE);
         
  int size = key.length() < SIZE ? key.length() : SIZE;
  for(uint16_t i = 0; i < size; i += 1)
  {
    _key[i] = (byte) key.at(i);
  }

  size = iv.length() < SIZE ? iv.length() : SIZE;
  for(uint16_t i = 0; i < size; i += 1)
  {
    _iv[i] = (byte) iv.at(i);
  }

  try
  {

  CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor(_key, sizeof(_key), _iv);
  CryptoPP::StringSource(data, true,
    new CryptoPP::StreamTransformationFilter(encryptor,
      new CryptoPP::StringSink(output)
    )
  );

  }
  catch(CryptoPP::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

void 
CryptoRijindael::decrypt(std::string& key, std::string& iv, std::string& data, std::string& output) 
{
  const int SIZE = 32;
  byte _key[SIZE];
  byte _iv [SIZE];
  memset(_key, 0x01, SIZE);
  memset(_iv,  0x01, SIZE);
         
  int size = key.length() < SIZE ? key.length() : SIZE;
  for(uint16_t i = 0; i < size; i += 1)
  {
    _key[i] = (byte) key.at(i);
  }

  size = iv.length() < SIZE ? iv.length() : SIZE;
  for(uint16_t i = 0; i < size; i += 1)
  {
    _iv[i] = (byte) iv.at(i);
  }

  try
  {

  CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor(_key, sizeof(_key), _iv);
  CryptoPP::StringSource(data, true,
    new CryptoPP::StreamTransformationFilter(decryptor,
      new CryptoPP::StringSink(output)
    )
  );

  }
  catch(CryptoPP::InvalidCiphertext& ict)
  {
    std::cerr << ict.what() << std::endl;
  }
  catch(CryptoPP::Exception& e)
  {
    std::cerr << e.what() << std::endl;
  }
}

