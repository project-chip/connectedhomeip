/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "WebSocketServer.h"

constexpr uint16_t kDefaultWebSocketServerPort = 9002;
constexpr uint16_t kMaxMessageBufferLen        = 8192;

namespace {
void LogWebSocketCallbackReason(lws_callback_reasons reason)
{
#if CHIP_DETAIL_LOGGING
    switch (reason)
    {
    case LWS_CALLBACK_GET_THREAD_ID:
        ChipLogDetail(chipTool, "LWS_CALLBACK_GET_THREAD_ID");
        break;
    case LWS_CALLBACK_ADD_HEADERS:
        ChipLogDetail(chipTool, "LWS_CALLBACK_ADD_HEADERS");
        break;
    case LWS_CALLBACK_PROTOCOL_INIT:
        ChipLogDetail(chipTool, "LWS_CALLBACK_PROTOCOL_INIT");
        break;
    case LWS_CALLBACK_PROTOCOL_DESTROY:
        ChipLogDetail(chipTool, "LWS_CALLBACK_PROTOCOL_DESTROY");
        break;
    case LWS_CALLBACK_HTTP:
        ChipLogDetail(chipTool, "LWS_CALLBACK_HTTP");
        break;
    case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
        ChipLogDetail(chipTool, "LWS_CALLBACK_EVENT_WAIT_CANCELLED");
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_CLIENT_WRITEABLE");
        break;
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        ChipLogDetail(chipTool, "LWS_CALLBACK_FILTER_NETWORK_CONNECTION");
        break;
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        ChipLogDetail(chipTool, "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
        break;
    case LWS_CALLBACK_WSI_CREATE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_WSI_CREATE");
        break;
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        ChipLogDetail(chipTool, "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED");
        break;
    case LWS_CALLBACK_HTTP_CONFIRM_UPGRADE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_HTTP_CONFIRM_UPGRADE");
        break;
    case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
        ChipLogDetail(chipTool, "LWS_CALLBACK_HTTP_BIND_PROTOCOL");
        break;
    case LWS_CALLBACK_ESTABLISHED:
        ChipLogDetail(chipTool, "LWS_CALLBACK_ESTABLISHED");
        break;
    case LWS_CALLBACK_RECEIVE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_RECEIVE");
        break;
    case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        ChipLogDetail(chipTool, "LWS_CALLBACK_WSI_DESTROY");
        break;
    case LWS_CALLBACK_CLOSED:
        ChipLogDetail(chipTool, "LWS_CALLBACK_CLOSED");
        break;
    case LWS_CALLBACK_SERVER_WRITEABLE:
        ChipLogDetail(chipTool, "LWS_CALLBACK_SERVER_WRITEABLE");
        break;
    case LWS_CALLBACK_CLOSED_HTTP:
        ChipLogDetail(chipTool, "LWS_CALLBACK_CLOSED_HTTP");
        break;
    default:
        ChipLogError(chipTool, "Unknown reason: %d ", static_cast<int>(reason));
    }
#endif // CHIP_DETAIL_LOGGING
}

static int OnWebSocketCallback(lws * wsi, lws_callback_reasons reason, void * user, void * in, size_t len)
{
    LogWebSocketCallbackReason(reason);

    if (LWS_CALLBACK_RECEIVE == reason)
    {
        char msg[kMaxMessageBufferLen + 1 /* for null byte */] = {};
        VerifyOrDie(sizeof(msg) > len);
        memcpy(msg, in, len);

        auto protocol = lws_get_protocol(wsi);
        auto delegate = static_cast<WebSocketServerDelegate *>(protocol->user);
        if (nullptr == delegate)
        {
            ChipLogError(chipTool, "Failed to retrieve the server interactive context.");
            return -1;
        }

        if (!delegate->OnWebSocketMessageReceived(msg))
        {
            auto context = lws_get_context(wsi);
            lws_default_loop_exit(context);
        }
    }
    else if (LWS_CALLBACK_CLIENT_ESTABLISHED == reason)
    {
        lws_callback_on_writable(wsi);
    }

    return 0;
}
} // namespace

CHIP_ERROR WebSocketServer::Run(chip::Optional<uint16_t> port, WebSocketServerDelegate * delegate)
{
    VerifyOrReturnError(nullptr != delegate, CHIP_ERROR_INVALID_ARGUMENT);

    lws_protocols protocols[] = { { "ws", OnWebSocketCallback, 0, 0, 0, delegate, 0 }, LWS_PROTOCOL_LIST_TERM };

    lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port             = port.ValueOr(kDefaultWebSocketServerPort);
    info.iface            = nullptr;
    info.pt_serv_buf_size = kMaxMessageBufferLen;
    info.protocols        = protocols;

    auto context = lws_create_context(&info);
    VerifyOrReturnError(nullptr != context, CHIP_ERROR_INTERNAL);

    lws_context_default_loop_run_destroy(context);
    return CHIP_NO_ERROR;
}
