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

#include <stdint.h>
#include <iostream>

#include <cryptlib.h>
#include <base64.h>
#include <aes.h>
#include <filters.h>
#include <eccrypto.h>
#include <osrng.h>
#include <oids.h>
#include <hex.h>
#include <files.h>
#include <config.h>
#include <modes.h>


#include "CryptoFactory.h"

using namespace sai::utils;

Crypto::Crypto()
{
}

Crypto::~Crypto()
{
}

SymmetricCrypto::SymmetricCrypto():
  size(0)
{
}

SymmetricCrypto::~SymmetricCrypto()
{
}

AsymmetricCrypto::AsymmetricCrypto()
{
}

AsymmetricCrypto::~AsymmetricCrypto()
{
}

CryptoFactory::CryptoFactory()
{
}

CryptoFactory::~CryptoFactory()
{
}

class CryptoRijindael : public SymmetricCrypto
{
public:
  CryptoRijindael();
  ~CryptoRijindael();
  void encrypt(SymmetricKey& key, std::string data, std::string& output);
  void decrypt(SymmetricKey& key, std::string data, std::string& output);
};

class EllipticCurve : public AsymmetricCrypto
{
public:
  EllipticCurve();
  ~EllipticCurve();
  void encrypt(AsymmetricKey& key, std::string data, std::string& output);
  void decrypt(AsymmetricKey& key, std::string data, std::string& output);
};

CryptoRijindael::CryptoRijindael() {}

CryptoRijindael::~CryptoRijindael() {}

EllipticCurve::EllipticCurve()
{
}

EllipticCurve::~EllipticCurve()
{
}

void 
EllipticCurve::encrypt(AsymmetricKey& key, std::string data, std::string& output)
{
  output.clear();

  CryptoPP::ECIES<CryptoPP::EC2N>::PublicKey * pubKey = static_cast<CryptoPP::ECIES<CryptoPP::EC2N>::PublicKey*>(key.getPublicKey());
  CryptoPP::ECIES<CryptoPP::EC2N>::Encryptor * encryptor = new CryptoPP::ECIES<CryptoPP::EC2N>::Encryptor(*pubKey);

  size_t encryptedSize = encryptor->CiphertextLength(data.size());
  if (encryptedSize == 0)
  {
    // TODO: Throw security exception! "Source data to be encrypted is too long!
    delete encryptor;
    return;
  }

  CryptoPP::AutoSeededRandomPool rng;
  byte* encryptedRawData = new byte[encryptedSize];
  //memset(encryptedRawData, 0, encryptedSize);
  encryptor->Encrypt(rng, (const byte*) data.data(), data.size(), encryptedRawData);
  //encryptor->Encrypt(rng, reinterpret_cast<const byte*> (data.data()), data.size(), encryptedRawData);
  output.assign(reinterpret_cast<char *>(encryptedRawData), encryptedSize);
  delete [] encryptedRawData;
  delete encryptor;
}

void 
EllipticCurve::decrypt(AsymmetricKey& key, std::string data, std::string& output)
{
  CryptoPP::AutoSeededRandomPool rng;

  CryptoPP::ECIES<CryptoPP::EC2N>::PrivateKey * priKey = static_cast<CryptoPP::ECIES<CryptoPP::EC2N>::PrivateKey*>(key.getPrivateKey());
  CryptoPP::ECIES<CryptoPP::EC2N>::Decryptor * decryptor = new CryptoPP::ECIES<CryptoPP::EC2N>::Decryptor(*priKey);

  size_t decryptedSize = decryptor->MaxPlaintextLength(data.size());

  if (decryptedSize == 0)
  {
    // TODO: Throw security exception! "Source data to be decrypted is too long!
    delete decryptor;
    return;
  }

  byte* decryptedRawData = new byte[decryptedSize];
  memset(decryptedRawData, 0, decryptedSize);
  decryptor->Decrypt(rng, reinterpret_cast<const byte*>(data.data()), data.size(), decryptedRawData);
  output.clear();
  output.assign(reinterpret_cast<char *>(decryptedRawData), decryptedSize);
  delete [] decryptedRawData;
  delete decryptor;
}

void 
CryptoRijindael::encrypt(SymmetricKey& key, std::string data, std::string& output) 
{
  try
  {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor(key.getKeyBytes(), key.size(), key.getIVBytes());
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
CryptoRijindael::decrypt(SymmetricKey& key, std::string data, std::string& output) 
{
  try
  {
    CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor(key.getKeyBytes(), key.size(), key.getIVBytes());
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

Crypto * 
CryptoFactory::create(CryptoAlgoType type)
{
  switch(type)
  {
    case AES256:
      return new CryptoRijindael(); 
    case ECC571:
      return new EllipticCurve();
  }
  return 0;
}

