/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// ***************************************************************************
// CryptoAlg.c:
// Interface to DNSSEC cryptographic algorithms. The crypto support itself is
// provided by the platform and the functions in this file just provide an
// interface to access them in a more generic way.
// ***************************************************************************

#include "mDNSEmbeddedAPI.h"
#include "CryptoAlg.h"

AlgFuncs *DigestAlgFuncs[DIGEST_TYPE_MAX];
AlgFuncs *CryptoAlgFuncs[CRYPTO_ALG_MAX];
AlgFuncs *EncAlgFuncs[ENC_ALG_MAX];

mDNSexport mStatus DigestAlgInit(mDNSu8 digestType, AlgFuncs *func)
{
    if (digestType >= DIGEST_TYPE_MAX)
    {
        LogMsg("DigestAlgInit: digestType %d exceeds bounds", digestType);
        return mStatus_BadParamErr;
    }
    // As digestTypes may not be consecutive, check for specific digest types
    // that we support
    if (digestType != SHA1_DIGEST_TYPE &&
        digestType != SHA256_DIGEST_TYPE)
    {
        LogMsg("DigestAlgInit: digestType %d not supported", digestType);
        return mStatus_BadParamErr;
    }
    DigestAlgFuncs[digestType] = func;
    return mStatus_NoError;
}

mDNSexport mStatus CryptoAlgInit(mDNSu8 alg, AlgFuncs *func)
{
    if (alg >= CRYPTO_ALG_MAX)
    {
        LogMsg("CryptoAlgInit: alg %d exceeds bounds", alg);
        return mStatus_BadParamErr;
    }
    // As algs may not be consecutive, check for specific algorithms
    // that we support
    if (alg != CRYPTO_RSA_SHA1 && alg != CRYPTO_RSA_SHA256 && alg != CRYPTO_RSA_SHA512 &&
        alg != CRYPTO_DSA_NSEC3_SHA1 && alg != CRYPTO_RSA_NSEC3_SHA1)
    {
        LogMsg("CryptoAlgInit: alg %d not supported", alg);
        return mStatus_BadParamErr;
    }

    CryptoAlgFuncs[alg] = func;
    return mStatus_NoError;
}

mDNSexport mStatus EncAlgInit(mDNSu8 alg, AlgFuncs *func)
{
    if (alg >= ENC_ALG_MAX)
    {
        LogMsg("EncAlgInit: alg %d exceeds bounds", alg);
        return mStatus_BadParamErr;
    }

    // As algs may not be consecutive, check for specific algorithms
    // that we support
    if (alg != ENC_BASE32 && alg != ENC_BASE64)
    {
        LogMsg("EncAlgInit: alg %d not supported", alg);
        return mStatus_BadParamErr;
    }

    EncAlgFuncs[alg] = func;
    return mStatus_NoError;
}

mDNSexport AlgContext *AlgCreate(AlgType type, mDNSu8 alg)
{
    AlgFuncs *func = mDNSNULL;
    AlgContext *ctx;

    if (type == CRYPTO_ALG)
    {
        if (alg >= CRYPTO_ALG_MAX) return mDNSNULL;
        func = CryptoAlgFuncs[alg];
    }
    else if (type == DIGEST_ALG)
    {
        if (alg >= DIGEST_TYPE_MAX) return mDNSNULL;
        func = DigestAlgFuncs[alg];
    }
    else if (type == ENC_ALG)
    {
        if (alg >= ENC_ALG_MAX) return mDNSNULL;
        func = EncAlgFuncs[alg];
    }

    if (!func)
    {
        // If there is no support from the platform, this case can happen.
        LogInfo("AlgCreate: func is NULL");
        return mDNSNULL;
    }

    if (func->Create)
    {
        mStatus err;
        ctx = mDNSPlatformMemAllocate(sizeof(AlgContext));
        if (!ctx) return mDNSNULL;
        // Create expects ctx->alg to be initialized
        ctx->alg = alg;
        err = func->Create(ctx);
        if (err == mStatus_NoError)
        {
            ctx->type = type;
            return ctx;
        }
        mDNSPlatformMemFree(ctx);
    }
    return mDNSNULL;
}

mDNSexport mStatus AlgDestroy(AlgContext *ctx)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    if (!func)
    {
        LogMsg("AlgDestroy: ERROR!! func is NULL");
        mDNSPlatformMemFree(ctx);
        return mStatus_BadParamErr;
    }

    if (func->Destroy)
        func->Destroy(ctx);

    mDNSPlatformMemFree(ctx);
    return mStatus_NoError;
}

mDNSexport mDNSu32 AlgLength(AlgContext *ctx)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    // This should never happen as AlgCreate would have failed
    if (!func)
    {
        LogMsg("AlgLength: ERROR!! func is NULL");
        return 0;
    }

    if (func->Length)
        return (func->Length(ctx));
    else
        return 0;
}

mDNSexport mStatus AlgAdd(AlgContext *ctx, const void *data, mDNSu32 len)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    // This should never happen as AlgCreate would have failed
    if (!func)
    {
        LogMsg("AlgAdd: ERROR!! func is NULL");
        return mStatus_BadParamErr;
    }

    if (func->Add)
        return (func->Add(ctx, data, len));
    else
        return mStatus_BadParamErr;
}

mDNSexport mStatus AlgVerify(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *signature, mDNSu32 siglen)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    // This should never happen as AlgCreate would have failed
    if (!func)
    {
        LogMsg("AlgVerify: ERROR!! func is NULL");
        return mStatus_BadParamErr;
    }

    if (func->Verify)
        return (func->Verify(ctx, key, keylen, signature, siglen));
    else
        return mStatus_BadParamErr;
}

mDNSexport mDNSu8* AlgEncode(AlgContext *ctx)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    // This should never happen as AlgCreate would have failed
    if (!func)
    {
        LogMsg("AlgEncode: ERROR!! func is NULL");
        return mDNSNULL;
    }

    if (func->Encode)
        return (func->Encode(ctx));
    else
        return mDNSNULL;
}

mDNSexport mStatus AlgFinal(AlgContext *ctx, void *data, mDNSu32 len)
{
    AlgFuncs *func = mDNSNULL;

    if (ctx->type == CRYPTO_ALG)
        func = CryptoAlgFuncs[ctx->alg];
    else if (ctx->type == DIGEST_ALG)
        func = DigestAlgFuncs[ctx->alg];
    else if (ctx->type == ENC_ALG)
        func = EncAlgFuncs[ctx->alg];

    // This should never happen as AlgCreate would have failed
    if (!func)
    {
        LogMsg("AlgEncode: ERROR!! func is NULL");
        return mDNSNULL;
    }

    if (func->Final)
        return (func->Final(ctx, data, len));
    else
        return mStatus_BadParamErr;
}
