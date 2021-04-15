#include <cstdio>
#include <cassert>
#include <cstring>
#include <nlunit-test.h>
#include <core/CHIPSafeCasts.h>
#include <transport/PASESession.h>
#include <support/CHIPMem.h>

using namespace chip;
using namespace Crypto;

class TestSecurePairingDelegate : public SessionEstablishmentDelegate
{
public:
    CHIP_ERROR SendSessionEstablishmentMessage(const PacketHeader & header, const Transport::PeerAddress & peerAddress,
                                               System::PacketBufferHandle msgBuf) override
    {
        mNumMessageSend++;
        return (peer != nullptr) ? peer->HandlePeerMessage(header, peerAddress, std::move(msgBuf)) : mMessageSendError;
    }

    void OnSessionEstablishmentError(CHIP_ERROR error) override { mNumPairingErrors++; }

    void OnSessionEstablished() override { mNumPairingComplete++; }

    uint32_t mNumMessageSend     = 0;
    uint32_t mNumPairingErrors   = 0;
    uint32_t mNumPairingComplete = 0;
    CHIP_ERROR mMessageSendError = CHIP_NO_ERROR;

    PASESession * peer = nullptr;
};

int main(int argc, char ** args)
{
    PASESessionSerialized serialized;
    PASESessionSerialized serialized2;
    TestSecurePairingDelegate delegateAccessory;
    TestSecurePairingDelegate delegateCommissioner;
    uint8_t encrypted[128];
    uint8_t output[128];
    const uint8_t plain_text[] = { 0x86, 0x74, 0x64, 0xe5, 0x0b, 0xd4, 0x0d, 0x90, 0xe1, 0x17, 0xa3, 0x2d, 0x4b, 0xd4, 0xe1, 0xe6 };
    PacketHeader header;
    MessageAuthenticationCode mac;
    SecureSession session1;
    SecureSession session2;
    PacketHeader packetHeader;

    printf("Hello CHIP!\n");
    CHIP_ERROR error = chip::Platform::MemoryInit();

    if (error != CHIP_NO_ERROR)
    {
        printf("Memory init error!\n");
        return FAILURE;
    }

//    // Allocate on the heap to avoid stack overflow in some restricted test scenarios (e.g. QEMU)
    auto * pairingAccessory = chip::Platform::New<PASESession>();
    auto * pairingCommissioner = chip::Platform::New<PASESession>();

    delegateCommissioner.peer = pairingAccessory;
    delegateAccessory.peer    = pairingCommissioner;

    assert(pairingAccessory->WaitForPairing(1234, 500, (const uint8_t *) "salt", 4, Optional<NodeId>::Value(1), 0,
                                                   &delegateAccessory) == CHIP_NO_ERROR);
    assert(pairingCommissioner->Pair(Transport::PeerAddress(Transport::Type::kTcp), 1234, Optional<NodeId>::Value(2), 1, 0,
                                            &delegateCommissioner) == CHIP_NO_ERROR);

//    auto * testPairingSession1 = chip::Platform::New<PASESession>();
//    auto * testPairingSession2 = chip::Platform::New<PASESession>();
//    SecurePairingDeserialize(inSuite, inContext, *testPairingSession1, *testPairingSession2);
//    void SecurePairingDeserialize(nlTestSuite * inSuite, void * inContext, PASESession & pairingCommissioner,
//                                  PASESession & deserialized)

    assert(pairingCommissioner->Serialize(serialized) == CHIP_NO_ERROR);

    assert(pairingAccessory->Deserialize(serialized) == CHIP_NO_ERROR);

    // Serialize from the deserialized session, and check we get the same string back
    assert(pairingAccessory->Serialize(serialized2) == CHIP_NO_ERROR);

    assert(strncmp(Uint8::to_char(serialized.inner), Uint8::to_char(serialized2.inner), sizeof(serialized)) == 0);
    printf("Successfully encoded and decoded!\n");


    // Let's try encrypting using original session, and decrypting using deserialized
    {

        assert(pairingAccessory->DeriveSecureSession(Uint8::from_const_char("AAD"), 3, session1) == CHIP_NO_ERROR);

        assert(session1.Encrypt(plain_text, sizeof(plain_text), encrypted, header, mac) == CHIP_NO_ERROR);
    }

    {
        assert(pairingCommissioner->DeriveSecureSession(Uint8::from_const_char("AAD"), 3, session2) == CHIP_NO_ERROR);

        uint8_t decrypted[64];
        assert(session2.Decrypt(encrypted, sizeof(plain_text), decrypted, header, mac) == CHIP_NO_ERROR);
        assert(memcmp(plain_text, decrypted, sizeof(plain_text)) == 0);
    }

    assert(session1.Encrypt(plain_text, sizeof(plain_text), encrypted, packetHeader, mac) == CHIP_NO_ERROR);

    assert(session2.Decrypt(encrypted, sizeof(plain_text), output, packetHeader, mac) == CHIP_NO_ERROR);

    assert(memcmp(plain_text, output, sizeof(plain_text)) == 0);


    assert(session2.Encrypt(plain_text, sizeof(plain_text), encrypted, packetHeader, mac) == CHIP_NO_ERROR);

    assert(session1.Decrypt(encrypted, sizeof(plain_text), output, packetHeader, mac) == CHIP_NO_ERROR);

    assert(memcmp(plain_text, output, sizeof(plain_text)) == 0);

    chip::Platform::Delete(pairingAccessory);
    chip::Platform::Delete(pairingCommissioner);

    return SUCCESS;
}
