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
#ifndef __CRYPTO_ALG_H
#define __CRYPTO_ALG_H

typedef enum
{
    CRYPTO_ALG,
    DIGEST_ALG,
    ENC_ALG,
} AlgType;

typedef struct
{
    void *context;
    AlgType type;
    mDNSu8 alg;
} AlgContext;

typedef struct
{
    mStatus (*Create)(AlgContext *ctx);
    mStatus (*Destroy)(AlgContext *ctx);
    mDNSu32 (*Length)(AlgContext *ctx);
    mStatus (*Add)(AlgContext *ctx, const void *data, mDNSu32 len);
    // Verify the ctx using the key and compare it against signature/siglen
    mStatus (*Verify)(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *signature, mDNSu32 siglen);
    // Encode the data and return the encoded data
    mDNSu8* (*Encode)(AlgContext *ctx);
    // Return the finalized data in data whose length is len (used by hash algorithms)
    mStatus (*Final)(AlgContext *ctx, void *data, mDNSu32 len);
} AlgFuncs;

mDNSexport mStatus DigestAlgInit(mDNSu8 digestType, AlgFuncs *func);
mDNSexport mStatus CryptoAlgInit(mDNSu8 algType, AlgFuncs *func);
mDNSexport mStatus EncAlgInit(mDNSu8 algType, AlgFuncs *func);


extern AlgContext *AlgCreate(AlgType type, mDNSu8 alg);
extern mStatus AlgDestroy(AlgContext *ctx);
extern mDNSu32 AlgLength(AlgContext *ctx);
extern mStatus AlgAdd(AlgContext *ctx, const void *data, mDNSu32 len);
extern mStatus AlgVerify(AlgContext *ctx, mDNSu8 *key, mDNSu32 keylen, mDNSu8 *signature, mDNSu32 siglen);
extern mDNSu8* AlgEncode(AlgContext *ctx);
extern mStatus AlgFinal(AlgContext *ctx, void *data, mDNSu32 len);

#endif // __CRYPTO_ALG_H
