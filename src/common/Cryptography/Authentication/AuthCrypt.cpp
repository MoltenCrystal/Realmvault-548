/*
 * Copyright (C) 2011-2016 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2016 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2016 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "AuthCrypt.h"
#include "Cryptography/HMACSHA1.h"
#include "Cryptography/BigNumber.h"
#include <cstring>

AuthCrypt::AuthCrypt() : PacketCrypt(SHA_DIGEST_LENGTH) { }

void AuthCrypt::Init(BigNumber* K)
{
    uint8 ServerEncryptionKey[SEED_KEY_SIZE] = { 0x08, 0xF1, 0x95, 0x9F, 0x47, 0xE5, 0xD2, 0xDB, 0xA1, 0x3D, 0x77, 0x8F, 0x3F, 0x3E, 0xE7, 0x00 };
    uint8 ServerDecryptionKey[SEED_KEY_SIZE] = { 0x40, 0xAA, 0xD3, 0x92, 0x26, 0x71, 0x43, 0x47, 0x3A, 0x31, 0x08, 0xA6, 0xE7, 0xDC, 0x98, 0x2A };
    Init(K, ServerEncryptionKey, ServerDecryptionKey);
}

void AuthCrypt::Init(BigNumber* k, uint8 const* serverKey, uint8 const* clientKey)
{
    HmacSha1 serverEncryptHmac(SEED_KEY_SIZE, (uint8*)serverKey);
    uint8 *encryptHash = serverEncryptHmac.ComputeHash(k);

    HmacSha1 clientDecryptHmac(SEED_KEY_SIZE, (uint8*)clientKey);
    uint8 *decryptHash = clientDecryptHmac.ComputeHash(k);

    _clientDecrypt.Init(decryptHash);
    _serverEncrypt.Init(encryptHash);

    // Drop first 1024 bytes, as WoW uses ARC4-drop1024.
    uint8 syncBuf[1024];
    memset(syncBuf, 0, 1024);

    _serverEncrypt.UpdateData(1024, syncBuf);

    memset(syncBuf, 0, 1024);

    _clientDecrypt.UpdateData(1024, syncBuf);

    _initialized = true;
}
