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

#include <lib/support/ScopedBuffer.h>
#include <libwebsockets.h>

#include <deque>
#include <mutex>
#include <string>

constexpr uint16_t kDefaultWebSocketServerPort                 = 9002;
constexpr uint16_t kMaxMessageBufferLen                        = 8192;
[[maybe_unused]] constexpr char kWebSocketServerReadyMessage[] = "== WebSocket Server Ready";

namespace {
lws * gWebSocketInstance = nullptr;
std::deque<std::string> gMessageQueue;

// This mutex protect the global gMessageQueue instance such that messages
// can be added/removed from multiple threads.
std::mutex gMutex;

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
        WebSocketServer * server = nullptr;
        auto protocol            = lws_get_protocol(wsi);
        if (!protocol)
        {
            ChipLogError(chipTool, "Failed to retrieve the protocol.");
            return -1;
        }
        server = static_cast<WebSocketServer *>(protocol->user);
        if (nullptr == server)
        {
            ChipLogError(chipTool, "Failed to retrieve the server interactive context.");
            return -1;
        }
        char msg[kMaxMessageBufferLen + 1 /* for null byte */] = {};
        VerifyOrDie(sizeof(msg) > len);
        memcpy(msg, in, len);

        server->OnWebSocketMessageReceived(msg);
    }
    else if (LWS_CALLBACK_SERVER_WRITEABLE == reason)
    {
        std::lock_guard<std::mutex> lock(gMutex);

        for (auto & msg : gMessageQueue)
        {
            chip::Platform::ScopedMemoryBuffer<unsigned char> buffer;
            VerifyOrDie(buffer.Calloc(LWS_PRE + msg.size()));
            memcpy(&buffer[LWS_PRE], (void *) msg.c_str(), msg.size());
            lws_write(wsi, &buffer[LWS_PRE], msg.size(), LWS_WRITE_TEXT);
        }

        gMessageQueue.clear();
    }
    else if (LWS_CALLBACK_ESTABLISHED == reason)
    {
        gWebSocketInstance = wsi;
    }
    else if (LWS_CALLBACK_WSI_DESTROY == reason)
    {
        gWebSocketInstance = nullptr;
    }
    else if (LWS_CALLBACK_PROTOCOL_INIT == reason)
    {
        ChipLogProgress(chipTool, "%s", kWebSocketServerReadyMessage);
    }

    return 0;
}
} // namespace

CHIP_ERROR WebSocketServer::Run(chip::Optional<uint16_t> port, WebSocketServerDelegate * delegate)
{
    VerifyOrReturnError(nullptr != delegate, CHIP_ERROR_INVALID_ARGUMENT);

    lws_protocols protocols[] = { { "ws", OnWebSocketCallback, 0, 0, 0, this, 0 }, LWS_PROTOCOL_LIST_TERM };

    lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port                         = port.ValueOr(kDefaultWebSocketServerPort);
    info.iface                        = nullptr;
    info.pt_serv_buf_size             = kMaxMessageBufferLen;
    info.protocols                    = protocols;
    static const lws_retry_bo_t retry = {
        .secs_since_valid_ping   = 400,
        .secs_since_valid_hangup = 420,
    };
    info.retry_and_idle_policy = &retry;

    auto context = lws_create_context(&info);
    VerifyOrReturnError(nullptr != context, CHIP_ERROR_INTERNAL);

    mRunning  = true;
    mDelegate = delegate;

    while (mRunning)
    {
        lws_service(context, -1);

        std::lock_guard<std::mutex> lock(gMutex);
        if (gMessageQueue.size())
        {
            lws_callback_on_writable(gWebSocketInstance);
        }
    }
    lws_context_destroy(context);
    return CHIP_NO_ERROR;
}

bool WebSocketServer::OnWebSocketMessageReceived(char * msg)
{
    auto shouldContinue = mDelegate->OnWebSocketMessageReceived(msg);
    if (!shouldContinue)
    {
        mRunning = false;
    }
    return shouldContinue;
}

void WebSocketServer::Send(const char * msg)
{
    std::lock_guard<std::mutex> lock(gMutex);
    gMessageQueue.push_back(msg);
}
