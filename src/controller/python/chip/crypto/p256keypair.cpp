
#include <controller/python/chip/crypto/p256keypair.h>

#include <cassert>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::python;
using namespace chip::Crypto;

pychip_P256Keypair::pychip_P256Keypair(void * aPyContext, pychip_P256Keypair_ECDSA_sign_msg aSignMsgFunct,
                                       pychip_P256Keypair_ECDH_derive_secret aDeriveSecretFunct) :
    mPyContext(aPyContext),
    mSignMsgFunct(aSignMsgFunct), mDeriveSecretFunct(aDeriveSecretFunct)
{}

pychip_P256Keypair::~pychip_P256Keypair() {}

CHIP_ERROR pychip_P256Keypair::Initialize(Crypto::ECPKeyTarget key_target)
{
    // Just override the initialize routing of the platform implementation.
    return CHIP_NO_ERROR;
}

CHIP_ERROR pychip_P256Keypair::Serialize(Crypto::P256SerializedKeypair & output) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR pychip_P256Keypair::Deserialize(Crypto::P256SerializedKeypair & input)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR pychip_P256Keypair::NewCertificateSigningRequest(uint8_t * csr, size_t & csr_length) const
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR pychip_P256Keypair::ECDSA_sign_msg(const uint8_t * msg, size_t msg_length,
                                              Crypto::P256ECDSASignature & out_signature) const
{
    VerifyOrReturnError(mSignMsgFunct != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    size_t signatureLength = out_signature.Capacity();

    VerifyOrReturnError(mSignMsgFunct(mPyContext, msg, msg_length, out_signature.Bytes(), &signatureLength), CHIP_ERROR_INTERNAL);
    out_signature.SetLength(signatureLength);

    return CHIP_NO_ERROR;
}

CHIP_ERROR pychip_P256Keypair::ECDH_derive_secret(const Crypto::P256PublicKey & remote_public_key,
                                                  Crypto::P256ECDHDerivedSecret & out_secret) const
{
    VerifyOrReturnError(mDeriveSecretFunct != nullptr, CHIP_ERROR_NOT_IMPLEMENTED);

    size_t secretLength = out_secret.Capacity();

    VerifyOrReturnError(mDeriveSecretFunct(mPyContext, remote_public_key.ConstBytes(), out_secret.Bytes(), &secretLength),
                        CHIP_ERROR_INTERNAL);
    out_secret.SetLength(secretLength);

    return CHIP_NO_ERROR;
}

void pychip_P256Keypair::UpdatePubkey(uint8_t * publicKey)
{
    memcpy(mPublicKey.Bytes(), publicKey, mPublicKey.Length());

    mInitialized = true;
}

chip::python::pychip_P256Keypair * pychip_NewP256Keypair(void * pyObject, pychip_P256Keypair_ECDSA_sign_msg aSignMsgFunct,
                                                         pychip_P256Keypair_ECDH_derive_secret aDeriveSecretFunct)
{
    auto res = new pychip_P256Keypair(pyObject, aSignMsgFunct, aDeriveSecretFunct);

    return res;
}

void pychip_P256Keypair_UpdatePubkey(chip::python::pychip_P256Keypair * this_, uint8_t * aPubKey)
{
    this_->UpdatePubkey(aPubKey);
}
