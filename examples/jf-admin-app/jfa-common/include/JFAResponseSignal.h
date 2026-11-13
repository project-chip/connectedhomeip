/*
 *
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

#pragma once

#include <cstdint>

namespace joint_fabric_service {

// Platform shim used by the shared JFA RPC server (RpcServer.cpp) to wait
// for and signal the single outstanding ICAC CSR response. The shared server
// owns the request/response data; only the wait/signal mechanism is
// platform-specific, so each platform supplies these three functions with its
// own primitive (POSIX condition variable, FreeRTOS semaphore, Zephyr k_sem).

// Signal that a response has been delivered. Safe to call when no waiter is
// blocked; the signal is latched so a later JFAWaitResponse() returns true.
void JFASignalResponse();

// Block until signaled or until timeoutMs elapses. Returns true if signaled
// within the timeout, false on timeout.
bool JFAWaitResponse(uint32_t timeoutMs);

// Clear any pending signal left over from a previous request that timed out
// here but was answered late by the RPC thread. Without this, the next
// JFAWaitResponse() could return immediately with a response that belongs to
// the earlier request (potentially a different key).
// For simplicity, this does not catch the scenario where we send a request,
// get a timeout, send a second request, then receive the first request's
// late response.
// The code here would treat that as a response for the second request.
void JFAResetResponseSignal();

} // namespace joint_fabric_service
