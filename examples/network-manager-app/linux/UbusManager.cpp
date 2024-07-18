/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include "UbusManager.h"

#include "UbusSchema.h"
#include "UloopHandler.h"
#include <lib/support/CodeUtils.h>

#include <libubus.h>

using namespace chip;

CHIP_ERROR UbusManager::Init()
{
    VerifyOrReturnError(!mInitialized, CHIP_ERROR_INCORRECT_STATE);

    UloopHandler::Register();

    int status;
    if ((status = ubus_connect_ctx(&mContext, nullptr)))
    {
        ChipLogError(AppServer, "Failed to connect to ubus: %s", ubus_strerror(status));
        UloopHandler::Unregister();
        return CHIP_ERROR_INTERNAL;
    }

    mObjectType       = kMatterUbusObjectType;
    mObject.type      = &mObjectType; // can't use kMatterUbusObjectType directly, must be mutable
    mObject.name      = mObjectType.name;
    mObject.methods   = mObjectType.methods;
    mObject.n_methods = mObjectType.n_methods;
    if ((status = ubus_add_object(&mContext, &mObject)))
    {
        ChipLogError(AppServer, "Failed to register ubus service: %s", ubus_strerror(status));
        ubus_shutdown(&mContext);
        UloopHandler::Unregister();
        return CHIP_ERROR_INTERNAL;
    }

    ubus_add_uloop(&mContext);
    mInitialized = true;

    return CHIP_NO_ERROR;
}

void UbusManager::Shutdown()
{
    VerifyOrReturn(mInitialized);
    mInitialized = false;
    ubus_shutdown(&mContext);
    UloopHandler::Unregister();
}

int MatterUbusHandleStatus(struct ubus_context * ctx, struct ubus_object * obj, struct ubus_request_data * req, const char * method,
                           struct blob_attr * msg)
{
    FabricTable const & fabricTable = Server::GetInstance().GetFabricTable();

    blob_buf blob{};
    blob_buf_init(&blob, 0);
    blobmsg_add_u8(&blob, "commissioned", fabricTable.FabricCount() > 0);
    ubus_send_reply(ctx, req, blob.head);
    blob_buf_free(&blob);
    return 0;
}
