/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2016-2017 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 *    @file
 *      Header file for the fault-injection utilities for Inet.
 */

#pragma once

#include <inet/InetConfig.h>

#if CHIP_WITH_NLFAULTINJECTION

#include <nlfaultinjection.hpp>

#include <lib/support/DLLUtil.h>

#include <system/SystemFaultInjection.h>

namespace chip {
namespace Inet {
namespace FaultInjection {

/**
 * @brief   Fault injection points
 *
 * @details
 * Each point in the code at which a fault can be injected
 * is identified by a member of this enum.
 */
typedef enum
{
    kFault_DNSResolverNew,  /**< Fail the allocation of a DNSResolver object */
    kFault_Send,            /**< Fail sending a message over TCP or UDP */
    kFault_SendNonCritical, /**< Fail sending a UDP message returning an error considered non-critical by RMP */
    kFault_NumItems,
} InetFaultInjectionID;

DLL_EXPORT nl::FaultInjection::Manager & GetManager();

} // namespace FaultInjection
} // namespace Inet
} // namespace chip

/**
 * Execute the statements included if the Inet fault is
 * to be injected.
 *
 * @param[in] aFaultID      An Inet fault-injection id
 * @param[in] aStatements   Statements to be executed if the fault is enabled.
 */
#define INET_FAULT_INJECT(aFaultID, aStatement) nlFAULT_INJECT(Inet::FaultInjection::GetManager(), aFaultID, aStatement)

#else // CHIP_WITH_NLFAULTINJECTION

#define INET_FAULT_INJECT(aFaultID, aStatement)

#endif // CHIP_WITH_NLFAULTINJECTION
