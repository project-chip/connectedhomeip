/* See Project CHIP LICENSE file for licensing information. */

#pragma once

namespace chip {
namespace Logging {

/**
 *  @enum LogModule
 *
 *  @brief
 *    Identifies a logical section of code that is a source of log
 *    messages.
 *
 *  @note If you add modules or rearrange this list you must update the
 *        ModuleNames tables in ChipLogging.cpp.
 *
 */
enum LogModule
{
    kLogModule_NotSpecified = 0,

    kLogModule_Inet,
    kLogModule_Ble,
    kLogModule_MessageLayer,
    kLogModule_SecurityManager,
    kLogModule_ExchangeManager,
    kLogModule_TLV,
    kLogModule_ASN1,
    kLogModule_Crypto,
    kLogModule_Controller,
    kLogModule_Alarm,
    kLogModule_SecureChannel,
    kLogModule_BDX,
    kLogModule_DataManagement,
    kLogModule_DeviceControl,
    kLogModule_DeviceDescription,
    kLogModule_Echo,
    kLogModule_FabricProvisioning,
    kLogModule_NetworkProvisioning,
    kLogModule_ServiceDirectory,
    kLogModule_ServiceProvisioning,
    kLogModule_SoftwareUpdate,
    kLogModule_FailSafe,
    kLogModule_TimeService,
    kLogModule_Heartbeat,
    kLogModule_chipSystemLayer,
    kLogModule_EventLogging,
    kLogModule_Support,
    kLogModule_chipTool,
    kLogModule_Zcl,
    kLogModule_Shell,
    kLogModule_DeviceLayer,
    kLogModule_SetupPayload,
    kLogModule_AppServer,
    kLogModule_Discovery,
    kLogModule_InteractionModel,
    kLogModule_Test,
    kLogModule_OperationalSessionSetup,
    kLogModule_Automation,
    kLogModule_CASESessionManager,

    kLogModule_Max
};

/* Log modules enablers. Those definitions can be overwritten with 0 to disable
   some log regions. */

#ifndef CHIP_CONFIG_LOG_MODULE_NotSpecified
#define CHIP_CONFIG_LOG_MODULE_NotSpecified 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Inet
#define CHIP_CONFIG_LOG_MODULE_Inet 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Ble
#define CHIP_CONFIG_LOG_MODULE_Ble 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_MessageLayer
#define CHIP_CONFIG_LOG_MODULE_MessageLayer 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_SecurityManager
#define CHIP_CONFIG_LOG_MODULE_SecurityManager 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_ExchangeManager
#define CHIP_CONFIG_LOG_MODULE_ExchangeManager 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_TLV
#define CHIP_CONFIG_LOG_MODULE_TLV 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_ASN1
#define CHIP_CONFIG_LOG_MODULE_ASN1 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Crypto
#define CHIP_CONFIG_LOG_MODULE_Crypto 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Controller
#define CHIP_CONFIG_LOG_MODULE_Controller 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Alarm
#define CHIP_CONFIG_LOG_MODULE_Alarm 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_SecureChannel
#define CHIP_CONFIG_LOG_MODULE_SecureChannel 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_BDX
#define CHIP_CONFIG_LOG_MODULE_BDX 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_DataManagement
#define CHIP_CONFIG_LOG_MODULE_DataManagement 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_DeviceControl
#define CHIP_CONFIG_LOG_MODULE_DeviceControl 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_DeviceDescription
#define CHIP_CONFIG_LOG_MODULE_DeviceDescription 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Echo
#define CHIP_CONFIG_LOG_MODULE_Echo 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_FabricProvisioning
#define CHIP_CONFIG_LOG_MODULE_FabricProvisioning 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_NetworkProvisioning
#define CHIP_CONFIG_LOG_MODULE_NetworkProvisioning 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_ServiceDiscovery
#define CHIP_CONFIG_LOG_MODULE_ServiceDiscovery 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_ServiceProvisioning
#define CHIP_CONFIG_LOG_MODULE_ServiceProvisioning 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_SoftwareUpdate
#define CHIP_CONFIG_LOG_MODULE_SoftwareUpdate 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_FailSafe
#define CHIP_CONFIG_LOG_MODULE_FailSafe 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_TimeService
#define CHIP_CONFIG_LOG_MODULE_TimeService 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Heartbeat
#define CHIP_CONFIG_LOG_MODULE_Heartbeat 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_chipSystemLayer
#define CHIP_CONFIG_LOG_MODULE_chipSystemLayer 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_EventLogging
#define CHIP_CONFIG_LOG_MODULE_EventLogging 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Support
#define CHIP_CONFIG_LOG_MODULE_Support 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_chipTool
#define CHIP_CONFIG_LOG_MODULE_chipTool 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Zcl
#define CHIP_CONFIG_LOG_MODULE_Zcl 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Shell
#define CHIP_CONFIG_LOG_MODULE_Shell 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_DeviceLayer
#define CHIP_CONFIG_LOG_MODULE_DeviceLayer 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_SetupPayload
#define CHIP_CONFIG_LOG_MODULE_SetupPayload 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_AppServer
#define CHIP_CONFIG_LOG_MODULE_AppServer 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Discovery
#define CHIP_CONFIG_LOG_MODULE_Discovery 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_InteractionModel
#define CHIP_CONFIG_LOG_MODULE_InteractionModel 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Test
#define CHIP_CONFIG_LOG_MODULE_Test 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_OperationalSessionSetup
#define CHIP_CONFIG_LOG_MODULE_OperationalSessionSetup 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_Automation
#define CHIP_CONFIG_LOG_MODULE_Automation 1
#endif

#ifndef CHIP_CONFIG_LOG_MODULE_CASESessionManager
#define CHIP_CONFIG_LOG_MODULE_CASESessionManager 1
#endif

/**
 *  @enum LogCategory
 *
 *  @brief
 *    Identifies a category to which an particular error message
 *    belongs.
 *
 */
enum LogCategory
{
    /*!<
     *   This log category indicates, when passed to SetLogFilter(),
     *   that no messages should be logged.
     *
     */
    kLogCategory_None = 0,

    /*!<
     *   Indicates a category of log message that describes an unexpected
     *   or severe failure.
     *
     *   This log category indicates that a logged message describes
     *   an unexpected or severe failure in the code.
     *
     *   It should be used for things such as out-of-resource errors,
     *   internal inconsistencies, API misuse, etc. In general, errors
     *   that are expected to occur as part of normal operation, or
     *   that are largely determined by external factors (e.g. network
     *   errors, user/operator induced errors, etc.) should be logged
     *   as kLogCategory_Progress messages, not as kLogCategory_Error
     *   messages.
     *
     */
    kLogCategory_Error = 1,

    /*!<
     *   Indicates a category of log message that describes an event
     *   that marks the start or end of a major activity, or a major
     *   change in the state of the overall system.
     *
     *   It should be reserved for high-level events. Such messages
     *   should provide the log reader with a good sense of the
     *   overall activity of the system at any point in time, while
     *   being minimally verbose. Where necessary such messages should
     *   include identifiers or other values that can be used to
     *   correlate messages involving a common actor or subject
     *   (e.g. connection ids, request ids, etc.) and/or to identify
     *   types of actions being taken or handled (e.g.  message types,
     *   requested resource types, error numbers, etc.).
     *
     */
    kLogCategory_Progress = 2,

    /*!<
     *   Indicates a category of log message that describes detailed
     *   information about an event or the state of the system.
     *
     *   Such messages can be used to provide ancillary information
     *   not suitable for the kLogCategory_Error and
     *   kLogCategory_Progress categories.
     *
     */
    kLogCategory_Detail = 3,

    /*!<
     *   Indicates a category of log message that describes automation
     *   information about an event or the state of the system.
     *
     *   Such messages can be used by automation for test validation.
     *
     */
    kLogCategory_Automation = 4,

    kLogCategory_Max = kLogCategory_Automation
};

} // namespace Logging
} // namespace chip
