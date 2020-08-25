/*
 *
 *    Copyright (c) 2019 Google LLC.
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

/**
 *    @file
 *          Wrapper header file for nrf_log_ctrl_internal.h supplied by
 *          the Nordic nRF5 SDK.
 */

// Include the version of nrf_log_ctrl_internal.h supplied by the nRF5 SDK.
#include_next "nrf_log_ctrl_internal.h"

// When using the non-deferred mode of the nRF5 loggging library in a multi-threaded
// application, calls to NRF_LOG_FLUSH must be serialized across all threads.  Later
// versions of the nRF5 SDK provide built-in support for this serialization, via the
// NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED option.  If this feature is NOT
// available, or not enabled, then we override the NRF_LOG_INTERNAL_FLUSH macro to
// enforce serialization.
#if !NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED

// Override the NRF_LOG_INTERNAL_FLUSH define to wrap the process of flushing
// pending log entries in a critical section.
#undef NRF_LOG_INTERNAL_FLUSH
#define NRF_LOG_INTERNAL_FLUSH()                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        CRITICAL_REGION_ENTER();                                                                                                   \
        while (NRF_LOG_INTERNAL_PROCESS())                                                                                         \
            ;                                                                                                                      \
        CRITICAL_REGION_EXIT();                                                                                                    \
    } while (0)

#endif // !NRF_LOG_NON_DEFFERED_CRITICAL_REGION_ENABLED
