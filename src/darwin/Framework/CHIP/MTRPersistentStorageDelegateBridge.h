/**
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRStorage.h"

#import "MTRError_Internal.h"
#include <lib/core/CHIPPersistentStorageDelegate.h>

NS_ASSUME_NONNULL_BEGIN

class MTRPersistentStorageDelegateBridge : public chip::PersistentStorageDelegate
{
public:
    MTRPersistentStorageDelegateBridge(id<MTRStorage> delegate);
    ~MTRPersistentStorageDelegateBridge();

    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override;

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override;

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override;

private:
    id<MTRStorage> mDelegate;
    dispatch_queue_t mWorkQueue;
};

NS_ASSUME_NONNULL_END
