/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "MatterUbusService.h"

#include "UboxUtils.h"

#include <app/server/Server.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>

extern "C" {
#include <libubus.h>
#undef fallthrough
}

using namespace chip::app::Clusters::AdministratorCommissioning;

namespace chip {

namespace {

// The spec caps a commissioning window at 15 minutes; default to the maximum,
// since the code is meant to be typed into a controller by a person.
constexpr uint32_t kDefaultWindowSeconds = 900;
constexpr uint32_t kMinWindowSeconds     = 180;

const char * WindowStatusString()
{
    // Not CommissioningWindowStatusForCluster(): that deliberately reports
    // locally opened windows as closed, because the cluster attribute only
    // covers windows opened through it. Here the local ones are the point.
    auto & mgr = Server::GetInstance().GetCommissioningWindowManager();
    if (!mgr.IsCommissioningWindowOpen())
    {
        return "closed";
    }
    switch (mgr.CommissioningWindowStatusForCluster())
    {
    case CommissioningWindowStatusEnum::kEnhancedWindowOpen:
        return "enhanced";
    case CommissioningWindowStatusEnum::kBasicWindowOpen:
    default:
        // A window opened locally is always a basic window.
        return "basic";
    }
}

void AddOnboarding(ubus::BlobMsgBuf & buf)
{
    PayloadContents payload;
    if (GetPayloadContents(payload, RendezvousInformationFlag::kOnNetwork) == CHIP_NO_ERROR)
    {
        char code[32] = {};
        char qr[128]  = {};
        MutableCharSpan codeSpan(code), qrSpan(qr);
        if (GetManualPairingCode(codeSpan, payload) == CHIP_NO_ERROR)
        {
            buf.Add("ManualCode", static_cast<const char *>(code));
        }
        if (GetQRCode(qrSpan, payload) == CHIP_NO_ERROR)
        {
            buf.Add("QrCode", static_cast<const char *>(qr));
        }
        buf.Add("VendorId", static_cast<uint32_t>(payload.vendorID));
        buf.Add("ProductId", static_cast<uint32_t>(payload.productID));
        buf.Add("Discriminator", static_cast<uint32_t>(payload.discriminator.GetLongValue()));
    }
}

int HandleStatus(ubus_context * ctx, ubus_object * obj, ubus_request_data * req, const char * method, blob_attr * msg)
{
    ubus::BlobMsgBuf buf;
    buf.Add("Fabrics", static_cast<uint32_t>(Server::GetInstance().GetFabricTable().FabricCount()));
    buf.Add("Window", WindowStatusString());
    // The initial onboarding code authenticates commissioning only while no
    // fabric is on the device (the initial basic window) or while a basic
    // window is open; report it so a UI can decide what to show.
    AddOnboarding(buf);
    ubus_send_reply(ctx, req, buf.head);
    return 0;
}

enum
{
    OPEN_WINDOW_TIMEOUT,
    __OPEN_WINDOW_MAX,
};

const blobmsg_policy kOpenWindowPolicy[__OPEN_WINDOW_MAX] = {
    [OPEN_WINDOW_TIMEOUT] = { .name = "timeout", .type = BLOBMSG_TYPE_INT32 },
};

int HandleOpenWindow(ubus_context * ctx, ubus_object * obj, ubus_request_data * req, const char * method, blob_attr * msg)
{
    // A caller can omit the argument table entirely; msg is NULL then.
    blob_attr * tb[__OPEN_WINDOW_MAX] = {};
    if (msg != nullptr)
    {
        blobmsg_parse(kOpenWindowPolicy, __OPEN_WINDOW_MAX, tb, blobmsg_data(msg), blobmsg_len(msg));
    }

    uint32_t timeout = kDefaultWindowSeconds;
    if (tb[OPEN_WINDOW_TIMEOUT] != nullptr)
    {
        timeout = blobmsg_get_u32(tb[OPEN_WINDOW_TIMEOUT]);
        VerifyOrReturnValue(timeout >= kMinWindowSeconds && timeout <= kDefaultWindowSeconds, UBUS_STATUS_INVALID_ARGUMENT);
    }

    // Opens a basic window: the device's own onboarding code becomes valid
    // for the duration, which is what lets a router UI show a code that a
    // controller can actually use after the device is already commissioned.
    auto & mgr     = Server::GetInstance().GetCommissioningWindowManager();
    CHIP_ERROR err = mgr.OpenBasicCommissioningWindow(System::Clock::Seconds32(timeout));

    ubus::BlobMsgBuf buf;
    buf.Add("Error", static_cast<uint32_t>(err == CHIP_NO_ERROR ? 0 : 1));
    buf.Add("Window", WindowStatusString());
    ubus_send_reply(ctx, req, buf.head);
    return 0;
}

int HandleCloseWindow(ubus_context * ctx, ubus_object * obj, ubus_request_data * req, const char * method, blob_attr * msg)
{
    Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();

    ubus::BlobMsgBuf buf;
    buf.Add("Error", static_cast<uint32_t>(0));
    buf.Add("Window", WindowStatusString());
    ubus_send_reply(ctx, req, buf.head);
    return 0;
}

ubus_method sMethods[] = {
    UBUS_METHOD_NOARG("status", HandleStatus),
    UBUS_METHOD("open_commissioning_window", HandleOpenWindow, kOpenWindowPolicy),
    UBUS_METHOD_NOARG("close_commissioning_window", HandleCloseWindow),
};

ubus_object_type sObjectType = UBUS_OBJECT_TYPE("matter", sMethods);

ubus_object sObject = {
    .name      = "matter",
    .type      = &sObjectType,
    .methods   = sMethods,
    .n_methods = MATTER_ARRAY_SIZE(sMethods),
};

} // namespace

CHIP_ERROR MatterUbusService::Init()
{
    return mUbusManager.Host(sObject);
}

} // namespace chip
