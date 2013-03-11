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
#include <math/Utils.h>
#include <utils/CryptoFactory.h>
#include "KeyManager.h"

using namespace sai::utils;
using namespace sai::math;
using namespace sai::net2;

KeyManager* KeyManager::instance = 0;

KeyManager::KeyManager():
  _sKey(0),
  _myPKIKeys(0),
  _othersPublicKey(0)
{
  CryptoKeyFactory keyFactory;
  _sKey            = static_cast<SymmetricKey*>(keyFactory.create(AES256));
  _myPKIKeys       = static_cast<AsymmetricKey*>(keyFactory.create(ECC521));
  _othersPublicKey = static_cast<AsymmetricKey*>(keyFactory.create(ECC521));

  _myPKIKeys->generate();
  const char possibleChar [] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                                '_', '#', '!', '@', '$', '(', ')', '-', '+', '=', '/', '{', '}',
                                '[', ']', ':', '?'};
  char buf[33] = {0};
  for (uint16_t i = 0; i < 32; i += 1)
  {
    uint32_t v = Utils::RandomInt(0, 78);
    char tmp[4];
    sprintf(tmp, "%c", possibleChar[v]);
    buf[i] = tmp[0];
    buf[i+1] = 0;
  }
  _sKey->setKey(buf);
  _sKey->randomIV();
  memset(buf, 0, sizeof(buf));
}

KeyManager* 
KeyManager::GetInstance()
{
  if (!instance)
  {
    instance = new KeyManager();
  }
  return instance;
}

KeyManager::~KeyManager()
{
  delete _sKey;
  delete _myPKIKeys;
  delete _othersPublicKey;
}

void 
KeyManager::setSymmetricKey(std::string key)
{
  _sKey->setKey(key);
}

void 
KeyManager::setOthersPublicKey(std::string key)
{
  _othersPublicKey->setPublicKey(key);
}

