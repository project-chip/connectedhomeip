/***************************************************************************//**
 * @file
 * @brief ZCL Core API
 ******************************************************************************/

#ifndef ZCL_CORE_H
#define ZCL_CORE_H

#include PLATFORM_HEADER
#include CHIP_AF_API_STACK
#include "zclip-struct.h"
#include "cbor.h"
#include "zcl-core-types.h"

#ifndef CHIP_SCRIPTED_TEST
  #include "thread-zclip.h"
#endif

#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginZclCorePrint(...)
  #define chipAfPluginZclCorePrintln(...)
  #define chipAfPluginZclCoreFlush()
  #define chipAfPluginZclCoreDebugExec(x)
  #define chipAfPluginZclCorePrintBuffer(buffer, len, withSpace)
  #define chipAfPluginZclCorePrintString(buffer)
#endif

/**
 * @addtogroup ZCLIP
 *
 * The ZCL Core plugin provides the necessary foundation of APIs to interface
 * with a ZCLIP-capable device.
 *
 * The functionality contained in this plugin provides basic ZCLIP features
 * including, but not limited to, the following:
 * - Attribute management
 * - Binding management
 * - Command handling and dispatching
 * - Endpoint management
 * - Group management
 * - Reporting configuration management
 * - Notification handling and dispatching
 * - Device discovery
 * - Application provisioning
 * - General ZCLIP utilities
 *
 * This plugin uses the Silicon Labs Constrained Application Protocol (CoAP)
 * implementation to communicate over the air with remote devices. More
 * information about the Silicon Labs CoAP implementation can be found in @ref
 * coap. This plugin also uses the Silicon Labs token system for storing
 * non-volatile ZCLIP data. More information about the Silicon Labs token
 * system can be found in @ref tokens.
 *
 * This plugin also provides a list of command line interface (CLI) commands
 * with which users can drive their applications. These CLI commands are
 * documented in the README file included in this release.
 *
 * See @ref zcl-core-callbacks for the ZCLIP application callback API.
 * @{
 */

// ----------------------------------------------------------------------------
// Addresses.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern ChipZclUid_t chZclUid;

// Commonly used IPv6 addresses
extern const ChipIpv6Address chipZclAllThreadNodes;

size_t chZclCacheGetEntryCount(void);
bool chZclCacheAdd(const ChipZclUid_t *key,
                   const ChipIpv6Address *value,
                   ChZclCacheIndex_t *index);
bool chZclCacheGet(const ChipZclUid_t *key,
                   ChipIpv6Address *value);
bool chZclCacheGetByIndex(ChZclCacheIndex_t index,
                          ChipZclUid_t *key,
                          ChipIpv6Address *value);
bool chZclCacheGetFirstKeyForValue(const ChipIpv6Address *value,
                                   ChipZclUid_t *key);
bool chZclCacheGetIndex(const ChipZclUid_t *key,
                        ChZclCacheIndex_t *index);
bool chZclCacheRemove(const ChipZclUid_t *key);
bool chZclCacheRemoveByIndex(ChZclCacheIndex_t index);
void chZclCacheRemoveAll(void);
size_t chZclCacheRemoveAllByValue(const ChipIpv6Address *value);
size_t chZclCacheRemoveAllByIpv6Prefix(const ChipIpv6Address *prefix,
                                       uint8_t prefixLengthInBits);
void chZclCacheScan(const void *criteria, ChZclCacheScanPredicate match);
#endif

// -----------------------------------------------------------------------------
// Endpoints.

/**
 * @addtogroup ZCLIP_endpoints Endpoints
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function finds the endpoint index for the specified endpoint identifier and
 * cluster specification.
 *
 * @param endpointId An endpoint identifier
 * @param clusterSpec A cluster specification or NULL
 * @return An endpoint index or @ref CHIP_ZCL_ENDPOINT_INDEX_NULL if no match
 *         is found.
 *
 * This function searches the endpoint table and returns the endpoint index for the entry
 * that matches the specified endpoint identifier and cluster specification.
 * If clusterSpec is NULL, match on endpointId only.
 *
 * @note The endpoint index is the zero-based relative position of an
 *       endpoint among the subset of endpoints that support the specified
 *       cluster. For example, an index of 3 refers to the fourth endpoint
 *       that supports the specified cluster. The value of endpoint index
 *       for a given endpoint identifier may be different for different
 *       clusters.
 *
 * @sa chipZclEndpointIndexToId()
 *****************************************************************************/
ChipZclEndpointIndex_t chipZclEndpointIdToIndex(ChipZclEndpointId_t endpointId,
                                                  const ChipZclClusterSpec_t *clusterSpec);

/**************************************************************************//**
 * This function finds the endpoint identifier for the specified endpoint index and
 * cluster specification.
 *
 * @param index An endpoint index
 * @param clusterSpec A cluster specification or NULL
 * @return An endpoint identifier or @ref CHIP_ZCL_ENDPOINT_NULL if no match
 *         is found.
 *
 * This function searches the endpoint table and returns the endpoint identifier for the entry
 * that matches the specified endpoint index and cluster specification.
 * If clusterSpec is NULL, match on index only.
 *
 * @note The endpoint index is the zero-based relative position of an
 *       endpoint among the subset of endpoints that support the specified
 *       cluster. For example, an index of 3 refers to the fourth endpoint
 *       that supports the specified cluster. The value of endpoint index
 *       for a given endpoint identifier may be different for different
 *       clusters.
 *
 * @sa chipZclEndpointIdToIndex()
 *****************************************************************************/
ChipZclEndpointId_t chipZclEndpointIndexToId(ChipZclEndpointIndex_t index,
                                               const ChipZclClusterSpec_t *clusterSpec);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const ChZclEndpointEntry_t chZclEndpointTable[];
extern const size_t chZclEndpointCount;
const ChZclEndpointEntry_t *chZclFindEndpoint(ChipZclEndpointId_t endpointId);
bool chZclEndpointHasCluster(ChipZclEndpointId_t endpointId,
                             const ChipZclClusterSpec_t *clusterSpec);
ChipZclStatus_t chZclMultiEndpointDispatch(const ChZclContext_t *context,
                                            ChZclMultiEndpointHandler handler,
                                            CborState *state,
                                            void *data);
void chZclUriEndpointHandler(ChZclContext_t *context);
void chZclUriEndpointIdHandler(ChZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Groups.

/**
 * @addtogroup ZCLIP_groups Groups
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function determines if an endpoint is a mchip of a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId A group identifier
 * @return `true` if the endpoint is a mchip of the group, `false` otherwise.
 *****************************************************************************/
bool chipZclIsEndpointInGroup(ChipZclEndpointId_t endpointId,
                               ChipZclGroupId_t groupId);

/**************************************************************************//**
 * This function returns TRUE if the supplied endpoint is a mchip of the group
 * and the supplied address is associated with the group. The following addresses
 * are automatically considered associated with any group:
 *  - Any unicast IPv6 address
 *  - All CoAP Nodes IPv6 address
 *  - All Nodes IPv6 address
 *  - The multicast IPv6 address constructed from the group ID according to the
 *    ZCL Base Device Behavior Specification
 *
 * @param endpointId An endpoint identifier
 * @param dstAddr IPv6 address to be checked
 * @param groupId A group identifier
 * @return `true` if the endpoint is a mchip of the group and the address is associated
 *        with the group, `false` otherwise.
 *****************************************************************************/
bool chipZclIsEndpointAndAddrInGroup(ChipZclEndpointId_t endpointId,
                                      const ChipIpv6Address *dstAddr,
                                      ChipZclGroupId_t groupId);

/**************************************************************************//**
 * This function returns TRUE if the supplied address is associated with one of
 * the groups on the device.
 * @param   dstAddr IPv6 address to be checked
 * @return `true` if the address is associated with one of the groups, `false`
 *          otherwise.
 *****************************************************************************/
bool chipZclIsAddressGroupMulticast(const ChipIpv6Address *dstAddr);

/**************************************************************************//**
 * This function gets a group name and its length.
 *
 * @param endpointId An endpoint identifier
 * @param groupId  A group identifier
 * @param groupName An array pointer which will contain the group name
 * @param groupNameLength A pointer which will contain the group name length
 * @return `true` if group name was retrieved successfully, `false` otherwise.
 *
 *****************************************************************************/
bool chipZclGetGroupName(ChipZclEndpointId_t endpointId,
                          ChipZclGroupId_t groupId,
                          uint8_t *groupName,
                          uint8_t *groupNameLength);

/**************************************************************************//**
 * This function adds an endpoint to a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId  A group identifier
 * @param groupName A pointer to an array containing name of group
 * @param groupNameLength Length of group name array (groupName is ignored if
 * this length is 0)
 * @param assignmentMode Assignment Mode parameter of  the "Add Group" command.
 *        If set to 0xff groupAddress and groupUdpPort should be ingnored.
 * @param groupAddress Pointer to an array containing the "Multicats IPv6 Address"
 *        parameter passed to the "Add Group" command: based on the assignment mode
 *        this could be the full address, flag bits or NULL.
 * @param groupUdpPort UDP port number that group should be listening on, zero value
 *        means the group will listen on CHIP_COAP_PORT and CHIP_COAP_SECURE_PORT.
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the endpoint was added to the group
 * - @ref CHIP_ZCL_STATUS_DUPLICATE_EXISTS if the endpoint is already a mchip
 *   of the group
 * - @ref CHIP_ZCL_STATUS_INSUFFICIENT_SPACE if there is no capacity to store
 *   the endpoint/group association
 * - @ref CHIP_ZCL_STATUS_FAILURE if groupId is @ref CHIP_ZCL_GROUP_ALL_ENDPOINTS,
 *   or if groupId is not a value between @ref CHIP_ZCL_GROUP_MIN and
 *   @ref CHIP_ZCL_GROUP_MAX inclusive, or if groupNameLength is greater than
 *   @ref CHIP_ZCL_MAX_GROUP_NAME_LENGTH, or if groupNameLength is non-zero and
 *   groupName is NULL
 *****************************************************************************/
ChipZclStatus_t chipZclAddEndpointToGroup(ChipZclEndpointId_t endpointId,
                                            ChipZclGroupId_t groupId,
                                            const uint8_t *groupName,
                                            uint8_t groupNameLength,
                                            uint8_t assignmentMode,
                                            const uint8_t *groupAddress,
                                            uint16_t groupUdpPort);
/**************************************************************************//**
 * This function removes an endpoint from a group.
 *
 * @param endpointId An endpoint identifier
 * @param groupId A group identifier
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the endpoint was removed from the group
 * - @ref CHIP_ZCL_STATUS_NOT_FOUND if the endpoint is not a mchip of the group
 * - @ref CHIP_ZCL_STATUS_INVALID_FIELD if the groupId is not a value between
 *   @ref CHIP_ZCL_GROUP_MIN and @ref CHIP_ZCL_GROUP_MAX inclusive
 *****************************************************************************/
ChipZclStatus_t chipZclRemoveEndpointFromGroup(ChipZclEndpointId_t endpointId,
                                                 ChipZclGroupId_t groupId);

/**************************************************************************//**
 * This function removes an endpoint from all groups to which it belongs.
 *
 * @param endpointId An endpoint identifier
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the endpoint was removed from one
 *   or more groups
 * - @ref CHIP_ZCL_STATUS_NOT_FOUND if the endpoint is not a mchip of
 *   any group
 *   (other than the all-endpoints group)
 *****************************************************************************/
ChipZclStatus_t chipZclRemoveEndpointFromAllGroups(ChipZclEndpointId_t endpointId);

/**************************************************************************//**
 * This function removes a group.
 *
 * @param groupId A group identifier
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the group was removed
 * - @ref CHIP_ZCL_STATUS_NOT_FOUND if the group does not exist
 * - @ref CHIP_ZCL_STATUS_INVALID_FIELD if the groupId is not a value between
 *   @ref CHIP_ZCL_GROUP_MIN and @ref CHIP_ZCL_GROUP_MAX inclusive
 *****************************************************************************/
ChipZclStatus_t chipZclRemoveGroup(ChipZclGroupId_t groupId);

/**************************************************************************//**
 * This function removes all groups.
 *
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if all groups were removed (other than the
 *   all-endpoints group)
 * - @ref CHIP_ZCL_STATUS_NOT_FOUND if no groups exist (other than the
 *   all-endpoints group)
 *****************************************************************************/
ChipZclStatus_t chipZclRemoveAllGroups(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
bool chZclHasGroup(ChipZclGroupId_t groupId);
size_t chZclGetGroupsCapacity(void);
void chZclUriGroupHandler(ChZclContext_t *context);
void chZclUriGroupIdHandler(ChZclContext_t *context);
void chZclInsertGroupIdIntoSortedList(const ChipZclGroupId_t groupId,
                                      ChipZclGroupId_t * const pgroups,
                                      uint16_t * const pcount,
                                      const uint16_t maxEntries);
void chZclReregisterAllMcastAddresses(void);
void chZclBuildGroupMcastAddress(const ChipZclGroupEntry_t *entry, uint8_t *addressBytes);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Management.

/**
 * @addtogroup ZCLIP_management Management
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function puts a device in EZ-Mode for a fixed-duration.
 *
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if EZ-Mode started successfully
 * - @ref CHIP_ERR_FATAL if the multicast address is invalid
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * Each time EZ-Mode is invoked, the device extends the window by the same
 * fixed duration. During the window, devices perform EZ-Mode finding and
 * binding with other devices also in EZ-Mode. Multicast messages
 * advertise capabilities of the device to other nodes in the network.
 * Unicast messages communicate binding targets to specific devices.
 * While the timer is active and not expired, including when the window is
 * extended due to subsequent invocations, the device listens on the EZ-Mode
 * multicast address and processes EZ-Mode requests.
 *****************************************************************************/
ChipStatus chipZclStartEzMode(void);

/**************************************************************************//**
 * This function stops EZ-Mode.
 *
 * The device ignores all EZ-Mode requests and stops listening on the EZ-Mode
 * multicast address.
 *****************************************************************************/
void chipZclStopEzMode(void);

/**************************************************************************//**
 * This function checks whether EZ-Mode is currently active.
 *
 * @return `true` if EZ-Mode is active, `false` otherwise.
 *****************************************************************************/
bool chipZclEzModeIsActive(void);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
const ChZclCommandEntry_t *chZclManagementFindCommand(ChipZclCommandId_t commandId);
void chZclManagementHandler(ChZclContext_t *context);
void chZclManagementCommandHandler(ChZclContext_t *context);
void chZclManagementCommandIdHandler(ChZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Clusters.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function compares two cluster specifications.
 *
 * @param s1 A cluster specification to be compared
 * @param s2 A cluster specification to be compared
 * @return
 * - @ref `0` if both cluster specifications are equal
 * - @ref `-1` if `s1`'s ::ChipZclRole_t is not equal to `s2`'s ::ChipZclRole_t
 *   and `s1`'s role is ::CHIP_ZCL_ROLE_CLIENT
 * - @ref `1` if `s1`'s ::ChipZclRole_t is not equal to `s2`'s ::ChipZclRole_t
 *   and `s1`'s role is ::CHIP_ZCL_ROLE_SERVER
 * - @ref difference between ::ChipZclManufacturerCode_t of `s1` and
 *   `s2`, or difference between ::ChipZclClusterId_t of `s1` and `s2`
 *****************************************************************************/
int32_t chipZclCompareClusterSpec(const ChipZclClusterSpec_t *s1,
                                   const ChipZclClusterSpec_t *s2);

/**************************************************************************//**
 * This function compares two cluster specifications.
 *
 * @param s1 A cluster specification to be compared
 * @param s2 A cluster specification to be compared
 * @return `true` if both cluster specifications are equal, `false` otherwise.
 *****************************************************************************/
bool chipZclAreClusterSpecsEqual(const ChipZclClusterSpec_t *s1,
                                  const ChipZclClusterSpec_t *s2);

/**************************************************************************//**
 * This function reverses cluster specifications.
 *
 * @param s1 A cluster specification used for reversing
 * @param s2 A cluster specification to be reversed
 *
 * This function changes ::ChipZclRole_t of `s2` to be opposite of `s1`. It
 * also sets ::ChipZclManufacturerCode_t and ::ChipZclClusterId_t of `s2`
 * to be the same as `s1`.
 *****************************************************************************/
void chipZclReverseClusterSpec(const ChipZclClusterSpec_t *s1,
                                ChipZclClusterSpec_t *s2);

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Attributes.

/**
 * @addtogroup ZCLIP_attributes Attributes
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function resets all local attributes on given endpoint.
 *
 * @param endpointId An endpoint identifier
 *
 * @note ::chipZclPostAttributeChangeCallback is triggered after each attribute
 *       is changed.
 *****************************************************************************/
void chipZclResetAttributes(ChipZclEndpointId_t endpointId);

/**************************************************************************//**
 * This function reads the value of an attibute.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier to read
 * @param buffer A buffer to read the attribute into
 * @param bufferLength Length of the read buffer
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the attribute was read successfully
 * - @ref CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is remote or
 *   if the attribute is not supported on a specified endpoint
 * - @ref CHIP_ZCL_STATUS_INSUFFICIENT_SPACE if not enough space is available in
 *   the passed buffer to store the attribute value
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @note ::chipZclReadExternalAttributeCallback is triggered if attribute
 *       is externally stored. If so, the result of that call is returned.
 *****************************************************************************/
ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId,
                                       const ChipZclClusterSpec_t *clusterSpec,
                                       ChipZclAttributeId_t attributeId,
                                       void *buffer,
                                       size_t bufferLength);

/**************************************************************************//**
 * This function writes the value of an attibute.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier to write
 * @param buffer A buffer to write the attribute from
 * @param bufferLength Length of the write buffer
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the attribute was written successfully
 * - @ref CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is remote or
 *   if the attribute is not supported on a specified endpoint
 * - @ref CHIP_ZCL_STATUS_INSUFFICIENT_SPACE if not enough space is available in
 *   the attribute table to store the attribute value
 * - @ref CHIP_ZCL_STATUS_INVALID_VALUE if the attribute value is invalid
 * - @ref CHIP_ZCL_STATUS_FAILURE if ::chipZclPreAttributeChangeCallback
 *   returned `false`
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @note ::chipZclWriteExternalAttributeCallback is triggered if the attribute
 *       is externally stored. If so, the result of that call is returned.
 * @note ::chipZclPostAttributeChangeCallback is triggered after the attribute
 *       is successfully changed.
 *****************************************************************************/
ChipZclStatus_t chipZclWriteAttribute(ChipZclEndpointId_t endpointId,
                                        const ChipZclClusterSpec_t *clusterSpec,
                                        ChipZclAttributeId_t attributeId,
                                        const void *buffer,
                                        size_t bufferLength);

/**************************************************************************//**
 * This function notifies the core code that an externally stored attribute has changed.
 *
 * @param endpointId An endpoint identifier of the attribute
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeId An attribute identifier that is changed
 * @param buffer A buffer to write the attribute from
 * @param bufferLength Length of the write buffer
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE if the attribute is not
 *   external or if the attribute is not supported on a specified endpoint
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @note ::chipZclPostAttributeChangeCallback is triggered for a successful
 *       call to this function.
 *****************************************************************************/
ChipZclStatus_t chipZclExternalAttributeChanged(ChipZclEndpointId_t endpointId,
                                                  const ChipZclClusterSpec_t *clusterSpec,
                                                  ChipZclAttributeId_t attributeId,
                                                  const void *buffer,
                                                  size_t bufferLength);

/**************************************************************************//**
 * This function sends an attribute read command to a specified destination.
 *
 * @param destination A location to read the attribute from
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeIds An array of attribute IDs to read
 * @param attributeIdsCount A total count of ::ChipZclAttributeId_t elements in a
 * passed array
 * @param handler callback that is triggered for a response
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @sa ChipZclReadAttributeResponseHandler()
 *****************************************************************************/
ChipStatus chipZclSendAttributeRead(const ChipZclDestination_t *destination,
                                      const ChipZclClusterSpec_t *clusterSpec,
                                      const ChipZclAttributeId_t *attributeIds,
                                      size_t attributeIdsCount,
                                      const ChipZclReadAttributeResponseHandler handler);

/**************************************************************************//**
 * This function sends an attribute write command to a specified destination.
 *
 * @param destination A location to write the attribute to
 * @param clusterSpec A cluster specification of the attribute
 * @param attributeWriteData An array containing write data for attributes
 * @param attributeWriteDataCount A total count of ::ChipZclAttributeWriteData_t
 * elements in a passed array
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @sa ChipZclWriteAttributeResponseHandler()
 *****************************************************************************/
ChipStatus chipZclSendAttributeWrite(const ChipZclDestination_t *destination,
                                       const ChipZclClusterSpec_t *clusterSpec,
                                       const ChipZclAttributeWriteData_t *attributeWriteData,
                                       size_t attributeWriteDataCount,
                                       const ChipZclWriteAttributeResponseHandler handler);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const uint8_t chZclAttributeDefaultMinMaxData[];
extern const size_t chZclAttributeDefaultMinMaxLookupTable[];
extern const ChZclAttributeEntry_t chZclAttributeTable[];
#define chZclIsAttributeLocal(attribute)                      \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_STORAGE_MASK) \
   != CH_ZCL_ATTRIBUTE_STORAGE_NONE)
#define chZclIsAttributeRemote(attribute)                     \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_STORAGE_MASK) \
   == CH_ZCL_ATTRIBUTE_STORAGE_NONE)
#define chZclIsAttributeExternal(attribute)                        \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK) \
   == CH_ZCL_ATTRIBUTE_STORAGE_TYPE_EXTERNAL)
#define chZclIsAttributeTokenized(attribute)                       \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_STORAGE_TYPE_MASK) \
   == CH_ZCL_ATTRIBUTE_STORAGE_TYPE_TOKEN)
#define chZclIsAttributeSingleton(attribute)                            \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_STORAGE_SINGLETON_MASK) \
   == CH_ZCL_ATTRIBUTE_STORAGE_SINGLETON_MASK)
#define chZclIsAttributeReadable(attribute)                      \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_ACCESS_READABLE) \
   == CH_ZCL_ATTRIBUTE_ACCESS_READABLE)
#define chZclIsAttributeWritable(attribute)                      \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_ACCESS_WRITABLE) \
   == CH_ZCL_ATTRIBUTE_ACCESS_WRITABLE)
#define chZclIsAttributeReportable(attribute)                      \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_ACCESS_REPORTABLE) \
   == CH_ZCL_ATTRIBUTE_ACCESS_REPORTABLE)
#define chZclIsAttributeBounded(attribute)                    \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_DATA_BOUNDED) \
   == CH_ZCL_ATTRIBUTE_DATA_BOUNDED)
#define chZclIsAttributeAnalog(attribute)                    \
  (READBITS((attribute)->mask, CH_ZCL_ATTRIBUTE_DATA_ANALOG) \
   == CH_ZCL_ATTRIBUTE_DATA_ANALOG)
const ChZclAttributeEntry_t *chZclFindAttribute(const ChipZclClusterSpec_t *clusterSpec,
                                                ChipZclAttributeId_t attributeId,
                                                bool includeRemote);
ChipZclStatus_t chZclReadAttributeEntry(ChipZclEndpointId_t endpointId,
                                         const ChZclAttributeEntry_t *attribute,
                                         void *buffer,
                                         size_t bufferLength);
ChipZclStatus_t chZclWriteAttributeEntry(ChipZclEndpointId_t endpointId,
                                          const ChZclAttributeEntry_t *attribute,
                                          const void *buffer,
                                          size_t bufferLength,
                                          bool enableUpdate);
size_t chZclAttributeSize(const ChZclAttributeEntry_t *attribute,
                          const void *data);
bool chZclReadEncodeAttributeKeyValue(CborState *state,
                                      ChipZclEndpointId_t endpointId,
                                      const ChZclAttributeEntry_t *attribute,
                                      void *buffer,
                                      size_t bufferLength);
bool chZclAttributeUriMetadataQueryParse(ChZclContext_t *context,
                                         void *data,
                                         uint8_t depth);
bool chZclAttributeUriQueryFilterParse(ChZclContext_t *context,
                                       void *data,
                                       uint8_t depth);
bool chZclAttributeUriQueryUndividedParse(ChZclContext_t *context,
                                          void *data,
                                          uint8_t depth);
void chZclUriClusterAttributeHandler(ChZclContext_t *context);
void chZclUriClusterAttributeIdHandler(ChZclContext_t *context);
bool chZclGetAbsDifference(uint8_t *bufferA,
                           uint8_t *bufferB,
                           uint8_t *diffBuffer,
                           size_t size);
void chZclSignExtendAttributeBuffer(uint8_t *outBuffer,
                                    size_t outBufferLength,
                                    const uint8_t *inBuffer,
                                    size_t inBufferLength,
                                    uint8_t attributeType);
uint8_t chZclDirectBufferedZclipType(uint8_t zclipType);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Bindings.

/**
 * @addtogroup ZCLIP_bindings Bindings
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function checks whether a specified binding exists.
 *
 * @param bindingId A binding identifier to check
 * @return
 * - @ref `true` if binding exists
 * - @ref `false` if binding does not exist
 *****************************************************************************/
bool chipZclHasBinding(ChipZclBindingId_t bindingId);

/**************************************************************************//**
 * This function gets a specified binding.
 *
 * @param bindingId A binding identifier of an entry to get
 * @param entry A binding entry to put the retrieved binding into
 * @return
 * - @ref `true` if binding was retrieved successfully
 * - @ref `false` if binding was not retrieved successfully
 *****************************************************************************/
bool chipZclGetBinding(ChipZclBindingId_t bindingId,
                        ChipZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function sets a specified binding.
 *
 * @param bindingId A binding identifier of entry to set
 * @param entry A new entry to set the binding to
 * @return
 * - @ref `true` if binding was set successfully
 * - @ref `false` if binding was not set successfully
 *****************************************************************************/
bool chipZclSetBinding(ChipZclBindingId_t bindingId,
                        const ChipZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function adds a given entry to the binding table.
 *
 * @param entry A binding entry to add to the table
 * @return
 * - @ref A binding identifier of entry if it was added successfully
 * - @ref CHIP_ZCL_BINDING_NULL if entry was not added successfully
 *
 * This function checks the binding table for duplicates. If a duplicate is
 * found, a binding identifier of the previous entry is used. Otherwise, a new one is
 * allocated. This function also validates contents of the given binding entry.
 *****************************************************************************/
ChipZclBindingId_t chipZclAddBinding(const ChipZclBindingEntry_t *entry);

/**************************************************************************//**
 * This function removes a specified entry from the binding table.
 *
 * @param bindingId A binding identifier of an entry to be removed from the table
 * @return
 * - @ref `true` if an entry was removed successfully
 * - @ref `false` if an entry was not removed successfully
 *****************************************************************************/
bool chipZclRemoveBinding(ChipZclBindingId_t bindingId);

/**************************************************************************//**
 * This function removes all entries from the binding table.
 *
 * @return
 * - @ref `true` if all entries were removed successfully
 * - @ref `false` if all entries were not removed successfully
 *****************************************************************************/
bool chipZclRemoveAllBindings(void);

/**************************************************************************//**
 * This function sends a command to a specified destination to add a binding.
 *
 * @param destination A location to send the command to
 * @param entry A binding entry to add to destination's binding table
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if the function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @sa ChipZclBindingResponseHandler()
 *****************************************************************************/
ChipStatus chipZclSendAddBinding(const ChipZclDestination_t *destination,
                                   const ChipZclBindingEntry_t *entry,
                                   const ChipZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function sends a command to a specified destination to update a binding.
 *
 * @param destination A location to send a command to
 * @param entry A new binding entry to use for an update
 * @param bindingId A binding identifier to update in destination's binding table
 * @param handler A callback that is triggered for a response
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @sa ChipZclBindingResponseHandler()
 *****************************************************************************/
ChipStatus chipZclSendUpdateBinding(const ChipZclDestination_t *destination,
                                      const ChipZclBindingEntry_t *entry,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function sends a command to a specified destination to remove a binding.
 *
 * @param destination location to send command to
 * @param clusterSpec cluster specification of binding entry to remove
 * @param bindingId Binding identifier to remove from destination's binding table
 * @param handler callback that is triggered for response
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if function call was successful
 * - @ref ::ChipStatus with failure reason otherwise
 *
 * @sa ChipZclBindingResponseHandler()
 *****************************************************************************/
ChipStatus chipZclSendRemoveBinding(const ChipZclDestination_t *destination,
                                      const ChipZclClusterSpec_t *clusterSpec,
                                      ChipZclBindingId_t bindingId,
                                      const ChipZclBindingResponseHandler handler);

/**************************************************************************//**
 * This function gets destination from specified matching binding
 *
 * @param clusterSpec cluster specification of binding entry to remove
 * @param bindingIdx  index to start searching the binding table (index value
 * is incremented on return if a matching binding is found)
 * @param destination the destination of the matching binding entry
 * @return
 * - @ref `true` if matching binding was found
 * - @ref `false` if matching binding was not found
 *
 * @sa ChipZclGetDestinationFromBinding()
 *****************************************************************************/
bool chipZclGetDestinationFromBinding(const ChipZclClusterSpec_t *clusterSpec,
                                       ChipZclBindingId_t *bindingIdx,
                                       ChipZclDestination_t *destination);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
size_t chZclGetBindingCount(void);
bool chZclHasBinding(const ChZclContext_t *context,
                     ChipZclBindingId_t bindingId);
void chZclUriClusterBindingHandler(ChZclContext_t *context);
void chZclUriClusterBindingIdHandler(ChZclContext_t *context);
void chZclReadDestinationFromBinding(const ChipZclBindingEntry_t *binding,
                                     ChipZclDestination_t *destination);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Commands.

/**
 * @addtogroup ZCLIP_commands Commands
 *
 * See zcl-core.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function sends a default response to a command.
 *
 * @param context A command context for the response
 * @param status A status to respond with
 * @return
 * - @ref CHIP_ZCL_STATUS_SUCCESS if response was sent successfully
 * - @ref CHIP_ZCL_STATUS_ACTION_DENIED if response to a multicast command
 * - @ref ::ChipStatus with failure reason otherwise
 *****************************************************************************/
ChipStatus chipZclSendDefaultResponse(const ChipZclCommandContext_t *context,
                                        ChipZclStatus_t status);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern const ChZclCommandEntry_t chZclCommandTable[];
extern const size_t chZclCommandCount;
const ChZclCommandEntry_t *chZclFindCommand(const ChipZclClusterSpec_t *clusterSpec,
                                            ChipZclCommandId_t commandId);
ChipStatus chZclSendCommandRequest(const ChipZclDestination_t *destination,
                                    const ChipZclClusterSpec_t *clusterSpec,
                                    ChipZclCommandId_t commandId,
                                    const void *request,
                                    const ZclipStructSpec *requestSpec,
                                    const ZclipStructSpec *responseSpec,
                                    const ChZclResponseHandler handler);
ChipStatus chZclSendCommandResponse(const ChipZclCommandContext_t *context,
                                     const void *response,
                                     const ZclipStructSpec *responseSpec);
void chZclUriClusterCommandHandler(ChZclContext_t *context);
void chZclUriClusterCommandIdHandler(ChZclContext_t *context);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Reporting.

/**
 * @addtogroup ZCLIP_reporting Reporting
 *
 * See zcl-core.h for source code.
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS

bool chZclHasReportingConfiguration(ChipZclEndpointId_t endpointId,
                                    const ChipZclClusterSpec_t *clusterSpec,
                                    ChipZclReportingConfigurationId_t reportingConfigurationId);
void chZclUriClusterNotificationHandler(ChZclContext_t *context);
void chZclUriClusterReportingConfigurationHandler(ChZclContext_t *context);
void chZclUriClusterReportingConfigurationIdHandler(ChZclContext_t *context);

#endif

/** @brief This function performs a factory reset of the reporting configurations:-
 * 1. All entries in nv reporting configurations table are erased
 * 2. Default configurations for each endpoint/clusterSpec are restored
 * to their initial values and saved to nv.
 */
void chipZclReportingConfigurationsFactoryReset(ChipZclEndpointId_t endpointId);

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Utilities.

/**
 * @addtogroup ZCLIP_utilities Utilities
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/**************************************************************************//**
 * This function returns the length of the octet or character data in a given string.
 *
 * @param buffer string pointer
 * @return length of string
 *****************************************************************************/
uint8_t chipZclStringLength(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the size of a given string including overhead and data.
 *
 * @param buffer string pointer
 * @return size of string
 *****************************************************************************/
uint8_t chipZclStringSize(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the length of the octet or character data in a given long string.
 *
 * @param buffer string pointer
 * @return length of string
 *****************************************************************************/
uint16_t chipZclLongStringLength(const uint8_t *buffer);

/**************************************************************************//**
 * This function returns the size of a given long string including overhead and data.
 *
 * @param buffer string pointer
 * @return size of string
 *****************************************************************************/
uint16_t chipZclLongStringSize(const uint8_t *buffer);

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define emIsMulticastAddress(ipAddress) ((ipAddress)[0] == 0xFF)

const uint8_t *chZclGetMessageStatusName(ChipZclMessageStatus_t status);

// URI segment matching functions
bool chZclUriPathStringMatch       (ChZclContext_t *context, void *castString, uint8_t depth);
bool chZclUriQueryStringPrefixMatch(ChZclContext_t *context, void *castString, uint8_t depth);

// If a response handler is supplied it must call chZclCoapStatusHandler()
// with the status and response info.  This is so that the system can detect
// broken UID<->address associations.
ChipStatus chZclSendWithOptions(const ChipZclCoapEndpoint_t *destination,
                                 ChipCoapCode code,
                                 const uint8_t *uri,
                                 const ChipCoapOption options[],
                                 uint16_t optionsLength,
                                 const uint8_t *payload,
                                 uint16_t payloadLength,
                                 ChipCoapResponseHandler handler,
                                 void *applicationData,
                                 uint16_t applicationDataLength,
                                 bool skipRetryEvent);
ChipStatus chZclSend(const ChipZclCoapEndpoint_t *destination,
                      ChipCoapCode code,
                      const uint8_t *uri,
                      const uint8_t *payload,
                      uint16_t payloadLength,
                      ChipCoapResponseHandler handler,
                      void *applicationData,
                      uint16_t applicationDataLength,
                      bool skipRetryEvent);
ChipStatus chipZclRequestBlock(const ChipZclCoapEndpoint_t *destination,
                                 const uint8_t *uriPath,
                                 ChipCoapBlockOption *block2Option,
                                 ChipCoapResponseHandler responseHandler);

void chZclCoapStatusHandler(ChipCoapStatus status, ChipCoapResponseInfo *info);

ChipStatus chipZclRespondWithPath(const ChipCoapRequestInfo *requestInfo,
                                    ChipCoapCode code,
                                    const uint8_t *path,
                                    const ChipCoapOption *options,
                                    uint8_t numberOfOptions,
                                    const uint8_t *payload,
                                    uint16_t payloadLength);
#define chipZclRespondWithCode(info, code) \
  (chipZclRespondWithPath((info), (code), NULL, NULL, 0, NULL, 0))
#define chipZclRespondWithPayload(info, code, payload, payloadLength) \
  (chipZclRespondWithPath((info), (code), NULL, NULL, 0, (payload), (payloadLength)))
#define chipZclRespond(info, code, opts, count, pay, len) \
  (chipZclRespondWithPath((info), (code), NULL, (opts), (count), (pay), (len)))

#define chipZclRespondCborState(info, code, locationPath, state) \
  (chipZclRespondCborPayload((info), (code), (locationPath), (state)->start, emCborEncodeSize(state)))
ChipStatus chipZclRespondCborPayload(const ChipCoapRequestInfo *info,
                                       ChipCoapCode code,
                                       const uint8_t *locationPath,
                                       const uint8_t *payload,
                                       uint16_t payloadLength);
#define chZclRespondCborPayloadWithOptions(info, code, options, optionsLength, payload, payloadLength) \
  (chipZclRespondWithPath(info, code, NULL, options, optionsLength, payload, payloadLength))
ChipStatus chipZclRespondLinkFormatPayload(const ChipCoapRequestInfo *info,
                                             ChipCoapCode code,
                                             const uint8_t *payload,
                                             uint16_t payloadLength,
                                             ChipCoapContentFormatType contentFormatType);
ChipStatus chipZclRespondWithStatus(const ChipCoapRequestInfo *info,
                                      ChipCoapCode code,
                                      ChipZclStatus_t status);
bool chZclEncodeDefaultResponse(CborState *state,
                                ChipZclStatus_t status);

#define chZclRespond201Created(info, locationPath) \
  (chipZclRespondWithPath(info, CHIP_COAP_CODE_201_CREATED, locationPath, NULL, 0, NULL, 0))
#define chZclRespond201CreatedCborState(info, locationPath, state) \
  (chipZclRespondCborState(info, CHIP_COAP_CODE_201_CREATED, locationPath, state))
#define chZclRespond202Deleted(info) \
  (chipZclRespondWithStatus(info, CHIP_COAP_CODE_202_DELETED, CHIP_SUCCESS))
#define chZclRespond204Changed(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_204_CHANGED))
#define chZclRespond204ChangedWithStatus(info, status) \
  (chipZclRespondWithStatus(info, CHIP_COAP_CODE_204_CHANGED, status))
#define chZclRespond204ChangedCborState(info, state) \
  (chipZclRespondCborState(info, CHIP_COAP_CODE_204_CHANGED, NULL, state))
#define chZclRespond204ChangedWithPath(info, locationPath) \
  (chipZclRespondWithPath((info), CHIP_COAP_CODE_204_CHANGED, locationPath, NULL, 0, NULL, 0))
#define chZclRespond205ContentCbor(info, payload, payloadLength) \
  (chipZclRespondCborPayload(info, CHIP_COAP_CODE_205_CONTENT, NULL, payload, payloadLength))
#define chZclRespond205ContentCborState(info, state) \
  (chipZclRespondCborState(info, CHIP_COAP_CODE_205_CONTENT, NULL, state))
#define chZclRespond205ContentLinkFormat(info, payload, payloadLength, contentFormat) \
  (chipZclRespondLinkFormatPayload(info, CHIP_COAP_CODE_205_CONTENT, payload, payloadLength, contentFormat))
#define chZclRespond205ContentCborWithOptions(info, options, optionsLength, payload, payloadLength) \
  (chZclRespondCborPayloadWithOptions(info, CHIP_COAP_CODE_205_CONTENT, options, optionsLength, payload, payloadLength))
#define chZclRespond400BadRequest(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_400_BAD_REQUEST))
#define chZclRespond400BadRequestWithStatus(info, status) \
  (chipZclRespondWithStatus(info, CHIP_COAP_CODE_400_BAD_REQUEST, status))
#define chZclRespond400BadRequestCborState(info, state) \
  (chipZclRespondCborState(info, CHIP_COAP_CODE_400_BAD_REQUEST, NULL, state))
#define chZclRespond401Unauthorized(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_401_UNAUTHORIZED))
#define chZclRespond402BadOption(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_402_BAD_OPTION))
#define chZclRespond404NotFound(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_404_NOT_FOUND))
#define chZclRespond405MethodNotAllowed(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_405_METHOD_NOT_ALLOWED))
#define chZclRespond406NotAcceptable(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_406_NOT_ACCEPTABLE))
#define chZclRespond412PreconditionFailedCborState(info, state) \
  (chipZclRespondCborState(info, CHIP_COAP_CODE_412_PRECONDITION_FAILED, NULL, state))
#define chZclRespond413RequestEntityTooLarge(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_413_REQUEST_ENTITY_TOO_LARGE))
#define chZclRespond415UnsupportedContentFormat(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_415_UNSUPPORTED_CONTENT_FORMAT))
#define chZclRespond500InternalServerError(info) \
  (chipZclRespondWithCode(info, CHIP_COAP_CODE_500_INTERNAL_SERVER_ERROR))

uint8_t chZclIntToHexString(uint32_t value, size_t size, uint8_t *result);
bool emHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result);
bool chZclHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result);
bool emHexStringToInt(const uint8_t *chars, size_t length, uintmax_t *result);
size_t chZclClusterToString(const ChipZclClusterSpec_t *clusterSpec,
                            uint8_t *result);
bool chZclStringToCluster(const uint8_t *chars,
                          size_t length,
                          ChipZclClusterSpec_t *clusterSpec);
bool chZclParseUri(const uint8_t *payload, uint16_t payloadLength, uint8_t **incoming, ChipCoapContentFormatType contentFormat, ChZclUriContext_t *context);
size_t chZclThingToUriPath(const ChipZclApplicationDestination_t *destination,
                           const ChipZclClusterSpec_t *clusterSpec,
                           char thing,
                           uint8_t *result);

size_t chZclCoapEndpointToUri(const ChipZclCoapEndpoint_t *network, uint8_t *result);
size_t chZclDestinationToUri(const ChipZclDestination_t *destination, uint8_t *result);
size_t chZclUriToCoapEndpoint(const uint8_t *uri, ChipZclCoapEndpoint_t *network);
bool chZclUriToBindingEntry(const uint8_t *uri,
                            ChipZclBindingEntry_t *result,
                            bool includeCluster);
size_t chZclUidToString(const ChipZclUid_t *uid, uint16_t uidBits, uint8_t *result);
bool chZclStringToUid(const uint8_t *uid,
                      size_t length,
                      ChipZclUid_t *result,
                      uint16_t *resultBits);
bool chZclConvertBase64UrlToCode(const uint8_t *base64Url,
                                 uint16_t length,
                                 uint8_t *code);
size_t chZclUidToBase64Url(const ChipZclUid_t *uid,
                           uint16_t uidBits,
                           uint8_t *base64Url);
bool chZclBase64UrlToUid(const uint8_t *base64Url,
                         size_t length,
                         ChipZclUid_t *result,
                         uint16_t *resultBits);
bool chZclNiUriToUid(const uint8_t *uri, uint16_t uriLength, ChipZclUid_t *uid);
int emDecodeBase64Url(const uint8_t *input,
                      size_t inputLength,
                      uint8_t *output,
                      size_t outputLength);

ChipZclStatus_t chZclCborValueReadStatusToChipStatus(ChZclCoreCborValueReadStatus_t cborValueReadStatus);

#define chZclClusterToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, ' ', result)
#define chZclAttributeToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, 'a', result)
#define chZclBindingToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, 'b', result)
#define chZclCommandToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, 'c', result)
#define chZclNotificationToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, 'n', result)
#define chZclReportingConfigurationToUriPath(address, clusterSpec, result) \
  chZclThingToUriPath(address, clusterSpec, 'r', result)

size_t chZclThingIdToUriPath(const ChipZclApplicationDestination_t *destination,
                             const ChipZclClusterSpec_t *clusterSpec,
                             char thing,
                             uintmax_t thingId,
                             size_t size,
                             uint8_t *result);
#define chZclAttributeIdToUriPath(address, clusterSpec, attributeId, result) \
  chZclThingIdToUriPath(address, clusterSpec, 'a', attributeId, sizeof(ChipZclAttributeId_t), result)
#define chZclBindingIdToUriPath(address, clusterSpec, bindingId, result) \
  chZclThingIdToUriPath(address, clusterSpec, 'b', bindingId, sizeof(ChipZclBindingId_t), result)
#define chZclCommandIdToUriPath(address, clusterSpec, commandId, result) \
  chZclThingIdToUriPath(address, clusterSpec, 'c', commandId, sizeof(ChipZclCommandId_t), result)
#define chZclReportingConfigurationIdToUriPath(address, clusterSpec, reportingConfigurationId, result) \
  chZclThingIdToUriPath(address, clusterSpec, 'r', reportingConfigurationId, sizeof(ChipZclReportingConfigurationId_t), result)

bool chZclGetMulticastAddress(ChipIpv6Address * dst);
#endif

/** @} end addtogroup */

// ----------------------------------------------------------------------------
// CLI.

#ifndef DOXYGEN_SHOULD_SKIP_THIS
bool chZclCliGetUidArgument(uint8_t index, ChipZclUid_t *uid);
void chZclCliResetCliState(void);
void chZclCliSetCurrentRequestCommand(const ChipZclClusterSpec_t *clusterSpec,
                                      ChipZclCommandId_t commandId,
                                      const ZclipStructSpec *structSpec,
                                      ChZclCliRequestCommandFunction function,
                                      const char *cliFormat);
#endif

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Access Control.

/**
 * @addtogroup ZCLIP_access_control Access Control
 *
 * See zcl-core.h for source code.
 * @{
 */

#ifndef DOXYGEN_SHOULD_SKIP_THIS
#define CH_ZCL_ACCESS_TOKEN_KEY 19
#define CH_ZCL_ACCESS_TOKEN_VALUE_MAX_LEN 500
void chZclAccessTokenHandler(ChZclContext_t *context);

bool chZclAddIncomingToken(const ChipZclUid_t *remoteUid,
                           const uint8_t *tokenBytes,
                           uint16_t tokenLength);
ChipZclStatus_t chZclAllowRemoteAccess(const uint8_t sessionId,
                                        const ChipZclClusterSpec_t *clusterSpec,
                                        ChipZclDeviceId_t endpointDeviceId,
                                        uint8_t accessType);
bool chZclIsProtectedResource(const ChipZclClusterSpec_t *clusterSpec,
                              uint8_t accessType);
bool chZclExtractBinaryAccessToken(CborState *state,
                                   uint8_t *output,
                                   uint16_t *outputSize);
void chZclPrintAccessTokens(void);

#endif

/** @} end addtogroup */

#endif // ZCL_CORE_H
