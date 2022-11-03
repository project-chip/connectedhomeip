/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRPersistentStorageDelegate.h"

#import "MTRError_Internal.h"
#include <lib/core/CHIPPersistentStorageDelegate.h>

NS_ASSUME_NONNULL_BEGIN

class MTRPersistentStorageDelegateBridge : public chip::PersistentStorageDelegate
{
public:
    MTRPersistentStorageDelegateBridge(id<MTRPersistentStorageDelegate> delegate);
    ~MTRPersistentStorageDelegateBridge();

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

private:
    id<MTRPersistentStorageDelegate> mDelegate;
    dispatch_queue_t mWorkQueue;
};

NS_ASSUME_NONNULL_END
