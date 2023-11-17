/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

struct CommissioningCallbacks
{
    /**
     * This is called when the PBKDFParamRequest is received and indicates the start of the session establishment process
     */
    std::function<void()> sessionEstablishmentStarted = {};

    /**
     * This is called when the commissioning session has been established
     */
    std::function<void()> sessionEstablished = {};

    /**
     * This is called when the PASE establishment failed (such as, when an invalid passcode is provided) or PASE was established
     * fine but then the fail-safe expired (including being expired by the commissioner)
     *
     * The CHIP_ERROR param indicates the error that occurred during session establishment or the error accompanying the fail-safe
     * timeout.
     */
    std::function<void(CHIP_ERROR)> sessionEstablishmentError = {};

    /**
     * This is called when the PASE establishment failed or PASE was established fine but then the fail-safe expired (including
     * being expired by the commissioner) AND the commissioning window is closed. The window may be closed because the commissioning
     * attempts limit was reached or advertising/listening for PASE failed.
     */
    std::function<void()> sessionEstablishmentStopped = {};

    /**
     * This is called when the commissioning has been completed
     */
    std::function<void(CHIP_ERROR)> commissioningComplete = {};
};
