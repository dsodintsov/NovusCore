/*
# MIT License

# Copyright(c) 2018-2019 NovusCore

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files(the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions :

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
*/
#pragma once

#include <asio/ip/tcp.hpp>
#include <Networking/BaseSocket.h>
#include <Networking/Opcode/Opcode.h>
#include <Cryptography/StreamEncryption.h>
#include <Cryptography/BigNumber.h>
#include <Cryptography/SHA1.h>
#include <random>

#include <Database/Cache/CharacterDatabaseCache.h>

#pragma pack(push, 1)
struct cAuthSessionData
{
    u32 build;
    u32 loginServerId;
    std::string accountName;
    u32 loginServerType;
    u32 localChallenge;
    u32 regionId;
    u32 battlegroupId;
    u32 realmId;
    u64 dosResponse;
    u8 digest[SHA_DIGEST_LENGTH];

    void Read(ByteBuffer& buffer)
    {
        buffer.GetU32(build);
        buffer.GetU32(loginServerId);
        buffer.GetString(accountName);
        buffer.GetU32(loginServerType);
        buffer.GetU32(localChallenge);
        buffer.GetU32(regionId);
        buffer.GetU32(battlegroupId);
        buffer.GetU32(realmId);
        buffer.GetU64(dosResponse);
        buffer.GetBytes(digest, 20);
    }
};
#pragma pack(pop)

class WorldNodeHandler;
class WorldConnection : public BaseSocket
{
public:
    WorldConnection(WorldNodeHandler* worldNodeHandler, asio::ip::tcp::socket* socket) : BaseSocket(socket), account(0), _headerBuffer(nullptr, sizeof(ClientPacketHeader)), _packetBuffer(nullptr, 4096)
    {
        _seed = static_cast<u32>(rand());
        _worldNodeHandler = worldNodeHandler;
    }

    bool Start() override;
    void Close(asio::error_code error) override;
    void HandleRead() override;
    void SendPacket(ByteBuffer* packet, u16 opcode);

    bool HandleNewHeader();
    bool HandleNewPacket();

    void HandleAuthSession();

    template <size_t T>
    inline void convert(char* val)
    {
        std::swap(*val, *(val + T - 1));
        convert<T - 2>(val + 1);
    }

    inline void apply(u16* val)
    {
        convert<sizeof(u16)>(reinterpret_cast<char*>(val));
    }
    inline void EndianConvertReverse(u16& val) { apply(&val); }

    u32 account;
    u64 characterGuid;
    cAuthSessionData sessionData;
    BigNumber sessionKey, seed1, seed2;

private:
    ByteBuffer _headerBuffer;
    ByteBuffer _packetBuffer;

    u32 _seed;
    StreamEncryption _streamEncryption;
    WorldNodeHandler* _worldNodeHandler;
};

template <>
inline void WorldConnection::convert<0>(char*) {}
template <>
inline void WorldConnection::convert<1>(char*) {} // ignore central byte
