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

#define TIME_TEST 0

#if TIME_TEST
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string>
#include <utils/CryptoKey.h>
#include <utils/CryptoFactory.h>

#define PRINT_CIPHER 0

using namespace sai::utils;

void testEncryptAES(int argc, char * argv[], std::string iv, std::string& output);
void testDecryptAES(std::string input, std::string iv, std::string& output);

void testEncryptECC1(int argc, char * argv[], std::string& output, std::string& privKey);
void testDecryptECC1(std::string input, std::string& privKey, std::string& output);

void testEncryptECC2(int argc, char * argv[], std::string& output, std::string& privKey);
void testDecryptECC2(std::string input, std::string& privKey, std::string& output);

int main(int argc, char * argv[])
{
  if (argc != 2)
  {
    return 1;
  }

  printf("--------------------------------------------\n");
  SymmetricKey::Initialize();
  SymmetricKey::Update();
  std::string iv = SymmetricKey::IV();
  //printf("IV (%s) %u\n", iv.c_str(), iv.length());

  std::string out;
  testEncryptAES(argc, argv, iv, out);

  //SymmetricKey::Update();
  //iv = SymmetricKey::IV();
  //printf("IV (%s) %u\n", iv.c_str(), iv.length());

  std::string orig;
  testDecryptAES(out, iv, orig);
  printf("--------------------------------------------\n");

  std::string priv;
  testEncryptECC1(argc, argv, out, priv);
  testDecryptECC1(out, priv, orig);

  printf("--------------------------------------------\n");

  testEncryptECC2(argc, argv, out, priv);
  testDecryptECC2(out, priv, orig);
  return 0;
}

#if TIME_TEST
uint32_t diff(struct timeval tv1, struct timeval tv2)
{
  uint64_t v1 = (tv1.tv_sec * 10000000000LL) + tv1.tv_usec;
  uint64_t v2 = (tv2.tv_sec * 10000000000LL) + tv2.tv_usec;
  return (uint32_t)((v2 - v1) / 10000);
}
#endif

void 
testEncryptECC1(int argc, char * argv[], std::string& output, std::string& privKey)
{
  std::string data = argv[1];

  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  AsymmetricKey * akey = static_cast<AsymmetricKey*>(keyFactory.create(ECC521));
  AsymmetricCrypto * acrypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC521));

#if TIME_TEST
  struct timeval tv1,tv2,tv3,tv4;
#endif
  printf("Generating Key....\n");
#if TIME_TEST
  gettimeofday(&tv1, 0);
#endif
  akey->generate();
#if TIME_TEST
  gettimeofday(&tv2, 0);
#endif
  printf("Done\n");
  privKey = akey->getPrivateKeyString();

#if TIME_TEST
  gettimeofday(&tv3, 0);
#endif
  acrypt->encrypt(*akey, data, output);
#if TIME_TEST
  gettimeofday(&tv4, 0);
#endif
  printf("      Text: %s\n", data.c_str());
#if PRINT_CIPHER
  printf("CipherText: %s\n", output.c_str());
#endif

  delete akey;
  delete acrypt;

#if TIME_TEST
  printf("Generate key %u usec\n", diff(tv1, tv2));
  printf("Encrypt %u usec\n", diff(tv3, tv4));
#endif
}

void 
testDecryptECC1(std::string input, std::string& privKey, std::string& output)
{
  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  AsymmetricKey * akey = static_cast<AsymmetricKey*>(keyFactory.create(ECC521));
  AsymmetricCrypto * acrypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC521));

  akey->setPrivateKey(privKey);

#if TIME_TEST
  struct timeval tv1,tv2;
  gettimeofday(&tv1, 0);
#endif
  acrypt->decrypt(*akey, input, output);
#if TIME_TEST
  gettimeofday(&tv2, 0);
  printf("Decrypt %u usec\n", diff(tv1, tv2));
#endif
#if PRINT_CIPHER
  printf("CipherText: %s\n", input.c_str());
#endif
  printf("      Text: %s\n", output.c_str());

  delete akey;
  delete acrypt;
}

void 
testEncryptECC2(int argc, char * argv[], std::string& output, std::string& privKey)
{
  std::string data = argv[1];

  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  AsymmetricKey * akey = static_cast<AsymmetricKey*>(keyFactory.create(ECC571));
  AsymmetricCrypto * acrypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC571));

#if TIME_TEST
  struct timeval tv1,tv2,tv3,tv4;
#endif
  printf("Generating Key....\n");
#if TIME_TEST
  gettimeofday(&tv1, 0);
#endif
  akey->generate();
#if TIME_TEST
  gettimeofday(&tv2, 0);
#endif
  printf("Done\n");
  privKey = akey->getPrivateKeyString();

#if TIME_TEST
  gettimeofday(&tv3, 0);
#endif
  acrypt->encrypt(*akey, data, output);
#if TIME_TEST
  gettimeofday(&tv4, 0);
#endif
  printf("      Text: %s\n", data.c_str());
#if PRINT_CIPHER
  printf("CipherText: %s\n", output.c_str());
#endif

  delete akey;
  delete acrypt;

#if TIME_TEST
  printf("Generate key %u usec\n", diff(tv1, tv2));
  printf("Encrypt %u usec\n", diff(tv3, tv4));
#endif
}

void 
testDecryptECC2(std::string input, std::string& privKey, std::string& output)
{
  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  AsymmetricKey * akey = static_cast<AsymmetricKey*>(keyFactory.create(ECC571));
  AsymmetricCrypto * acrypt = static_cast<AsymmetricCrypto*>(cryptFactory.create(ECC571));

  akey->setPrivateKey(privKey);

#if TIME_TEST
  struct timeval tv1,tv2;
  gettimeofday(&tv1, 0);
#endif
  acrypt->decrypt(*akey, input, output);
#if TIME_TEST
  gettimeofday(&tv2, 0);
  printf("Decrypt %u usec\n", diff(tv1, tv2));
#endif
#if PRINT_CIPHER
  printf("CipherText: %s\n", input.c_str());
#endif
  printf("      Text: %s\n", output.c_str());

  delete akey;
  delete acrypt;
}

void 
testDecryptAES(std::string input, std::string iv, std::string& output)
{
  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  SymmetricKey * skey = static_cast<SymmetricKey*>(keyFactory.create(AES256));
  SymmetricCrypto * scrypt = static_cast<SymmetricCrypto*>(cryptFactory.create(AES256));

  skey->setKey("HelloWorld");
  skey->setIV(iv);

#if TIME_TEST
  struct timeval tv1,tv2;
  gettimeofday(&tv1, 0);
#endif
  scrypt->decrypt(*skey, input, output);
#if TIME_TEST
  gettimeofday(&tv2, 0);
#endif
#if PRINT_CIPHER
  printf("CipherText: %s\n", input.c_str());
#endif
  printf("      Text: %s\n", output.c_str());
#if TIME_TEST
  printf("Decrypt %u usec\n", diff(tv1, tv2));
#endif

  delete skey;
  delete scrypt;
}

void 
testEncryptAES(int argc, char * argv[], std::string iv, std::string& output)
{
  std::string data = argv[1];

  CryptoKeyFactory keyFactory;
  CryptoFactory cryptFactory;

  SymmetricKey * skey = static_cast<SymmetricKey*>(keyFactory.create(AES256));
  SymmetricCrypto * scrypt = static_cast<SymmetricCrypto*>(cryptFactory.create(AES256));

  skey->setKey("HelloWorld");
  skey->setIV(iv);

#if TIME_TEST
  struct timeval tv1,tv2;
  gettimeofday(&tv1, 0);
#endif
  scrypt->encrypt(*skey, data, output);
#if TIME_TEST
  gettimeofday(&tv2, 0);
#endif
  printf("      Text: %s\n", data.c_str());
#if PRINT_CIPHER
  printf("CipherText: %s\n", output.c_str());
#endif
#if TIME_TEST
  printf("Encrypt %u usec\n", diff(tv1, tv2));
#endif

  delete skey;
  delete scrypt;
}
