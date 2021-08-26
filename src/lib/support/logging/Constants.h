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
    kLogModule_TokenPairing,
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

    kLogModule_Max
};

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

    kLogCategory_Max = kLogCategory_Detail
};

} // namespace Logging
} // namespace chip
