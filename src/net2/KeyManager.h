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

#ifndef __SAI_NET2_KEYMANAGER__
#define __SAI_NET2_KEYMANAGER__

#include <utils/CryptoKey.h>

namespace sai 
{
namespace net2
{

class KeyManager
{
private:
  static KeyManager* instance;
  sai::utils::SymmetricKey  * _sKey;
  sai::utils::AsymmetricKey * _myPKIKeys;
  sai::utils::AsymmetricKey * _othersPublicKey;

private:
  KeyManager();

public:
  static KeyManager* GetInstance();
  ~KeyManager();

  sai::utils::SymmetricKey*  getSymmetricKey()    { return _sKey;      }
  sai::utils::AsymmetricKey* getAsymmetricKey()   { return _myPKIKeys; }
  void                       setSymmetricKey(std::string key);
  void                       setOthersPublicKey(std::string key);
  sai::utils::AsymmetricKey* getOthersPublicKey() { return _othersPublicKey; }
};

}}

#endif
