/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include "config.h"

#include <app/util/types_stub.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>

chip::Messaging::ExchangeDelegate * GetDataModelHandler();

/**
 *  @brief
 *    Singleton handler for DataModel messages. Will be registered into ExchangeManager
 */
class DataModelHandler : public chip::Messaging::ExchangeDelegate
{
public:
    DataModelHandler() {}
    virtual ~DataModelHandler() {}

    // Not copyable or movable
    DataModelHandler(const DataModelHandler &) = delete;
    DataModelHandler & operator=(const DataModelHandler &) = delete;
    DataModelHandler(DataModelHandler &&)                  = delete;
    DataModelHandler & operator=(DataModelHandler &&) = delete;

    /* ExchangeDelegate interface functions */
    void OnMessageReceived(chip::Messaging::ExchangeContext * ec, const chip::PacketHeader & packetHeader, uint32_t protocolId,
                           uint8_t msgType, chip::System::PacketBufferHandle payload) override;
    void OnResponseTimeout(chip::Messaging::ExchangeContext * ec) override;
    void OnExchangeClosing(chip::Messaging::ExchangeContext * ec) override;
};

/**
 *  @brief
 *    Short live class for storing variables to be used to send or handle a message. The object of this class will be created on the
 *    stack when start processing a message, or handle a timeout event. The object will be passed all way along the processing
 *    procedure. Almost all functions will take an argument of the context.
 */
class DataModelContext
{
public:
    DataModelContext() {}

    class DataModelContextLock
    {
    public:
        ~DataModelContextLock()
        {
            if (mContext != nullptr)
                mContext->Release();
        }

        DataModelContextLock(const DataModelContextLock &) = delete;
        DataModelContextLock & operator=(const DataModelContextLock &) = delete;
        DataModelContextLock(DataModelContextLock && that)
        {
            that.mContext = mContext;
            mContext      = nullptr;
        }
        DataModelContextLock & operator=(DataModelContextLock &&) = delete;

    private:
        friend class DataModelContext;
        DataModelContextLock(DataModelContext * context) : mContext(context) {}

        DataModelContext * mContext;
    };

    /* Ensure the lifespan of DataModelContext. The context will be automatically released when the lock goes out of scope. It will
     * also crash when the lock is grabbed multiple times recursively without release.
     */
    DataModelContextLock Scoped(chip::Messaging::ExchangeContext * exchangeContext)
    {
        VerifyOrDie(mExchangeContext == nullptr);
        mExchangeContext = exchangeContext;
        return { this };
    }

    void Release() { mExchangeContext = nullptr; }

    // Not copyable or movable
    DataModelContext(const DataModelContext &) = delete;
    DataModelContext & operator=(const DataModelContext &) = delete;
    DataModelContext(DataModelContext &&)                  = delete;
    DataModelContext & operator=(DataModelContext &&) = delete;

    chip::Messaging::ExchangeContext & GetExchangeContext() { return *mExchangeContext; }

private:
    chip::Messaging::ExchangeContext * mExchangeContext = nullptr;
};
