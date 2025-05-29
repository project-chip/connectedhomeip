/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

/**
 *    @file
 *          Platform-specific configuration overrides for CHIP on
 *          Darwin platforms.
 */

#pragma once

#include <TargetConditionals.h>

// ==================== General Platform Adaptations ====================

#define CHIP_CONFIG_ABORT() abort()

#include <os/trace_base.h> // for __dso_handle
extern "C" int __cxa_atexit(void (*f)(void *), void * p, void * d);
#define CHIP_CXA_ATEXIT(f, p) __cxa_atexit((f), (p), &__dso_handle)

#define CHIP_CONFIG_GLOBALS_LAZY_INIT 1

#define CHIP_CONFIG_ERROR_FORMAT_AS_STRING 1
#define CHIP_CONFIG_ERROR_SOURCE 1

#define CHIP_CONFIG_VERBOSE_VERIFY_OR_DIE 1

#define CHIP_CONFIG_IM_STATUS_CODE_VERBOSE_FORMAT 1

// ==================== Security Adaptations ====================

// If unspecified, assume crypto is fast on Darwin
#ifndef CHIP_CONFIG_SLOW_CRYPTO
#define CHIP_CONFIG_SLOW_CRYPTO 0
#endif // CHIP_CONFIG_SLOW_CRYPTO

// ==================== General Configuration Overrides ====================

#ifndef CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS
#define CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS 8
#endif // CHIP_CONFIG_MAX_UNSOLICITED_MESSAGE_HANDLERS

//
// Default of 8 ECs is not sufficient for some of the unit tests
// that try to validate multiple simultaneous interactions.
// In tests like TestReadHandler_MultipleSubscriptions, we are trying to issue as many read / subscription requests as possible in
// parallel. Since the default config says we support 16 fabrics, and we will have 4 read handlers for each fabric (3 subscriptions
// + 1 reserved for read) that is read transactions in parallel. Since the report handlers are allocated on the heap, we will issue
// 65 requests (the TestReadHandler_MultipleSubscriptions will issue CHIP_IM_MAX_NUM_READ_HANDLER + 1 subscriptions to verify heap
// allocation logic) in total and that is 130 ECs. Round this up to 150 ECs
//
#define CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS 150

#ifndef CHIP_LOG_FILTERING
#define CHIP_LOG_FILTERING 1
#endif // CHIP_LOG_FILTERING

#ifndef CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS
#define CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS 1
#endif // CHIP_CONFIG_BDX_MAX_NUM_TRANSFERS

#ifndef CHIP_CONFIG_KVS_PATH
#if TARGET_OS_IPHONE
#define CHIP_CONFIG_KVS_PATH "chip.store"
#else
#define CHIP_CONFIG_KVS_PATH "/tmp/chip_kvs"
#endif // TARGET_OS_IPHONE
#endif // CHIP_CONFIG_KVS_PATH

#define CHIP_SYSTEM_CONFIG_PACKETBUFFER_POOL_SIZE 0

// The session pool size limits how many subscriptions we can have live at
// once.  Home supports up to 1000 accessories, and we subscribe to all of them,
// so we need to make sure the pool is big enough for that.
#define CHIP_CONFIG_SECURE_SESSION_POOL_SIZE 1000

#define INET_CONFIG_OVERRIDE_SYSTEM_TCP_USER_TIMEOUT 0
