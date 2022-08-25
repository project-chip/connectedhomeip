/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
 *      This file contains functions for working with CHIP errors.
 */

#include <stddef.h>

#include <lib/core/CHIPCore.h>
#include <lib/support/ErrorStr.h>

namespace chip {

/**
 * Register a text error formatter for CHIP core errors.
 */
void RegisterCHIPLayerErrorFormatter()
{
    static ErrorFormatter sCHIPErrorFormatter = { FormatCHIPError, nullptr };

    RegisterErrorFormatter(&sCHIPErrorFormatter);
}

/**
 * Given a CHIP error, returns a human-readable NULL-terminated C string
 * describing the error.
 *
 * @param[in] buf                   Buffer into which the error string will be placed.
 * @param[in] bufSize               Size of the supplied buffer in bytes.
 * @param[in] err                   The error to be described.
 *
 * @return true                     If a description string was written into the supplied buffer.
 * @return false                    If the supplied error was not a CHIP error.
 *
 */
bool FormatCHIPError(char * buf, uint16_t bufSize, CHIP_ERROR err)
{
    const char * desc = nullptr;

    if (!err.IsPart(ChipError::SdkPart::kCore))
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err.AsInteger())
    {
    case CHIP_ERROR_SENDING_BLOCKED.AsInteger():
        desc = "Sending blocked";
        break;
    case CHIP_ERROR_CONNECTION_ABORTED.AsInteger():
        desc = "Connection aborted";
        break;
    case CHIP_ERROR_INCORRECT_STATE.AsInteger():
        desc = "Incorrect state";
        break;
    case CHIP_ERROR_MESSAGE_TOO_LONG.AsInteger():
        desc = "Message too long";
        break;
    case CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION.AsInteger():
        desc = "Unsupported exchange version";
        break;
    case CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS.AsInteger():
        desc = "Too many unsolicited message handlers";
        break;
    case CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER.AsInteger():
        desc = "No unsolicited message handler";
        break;
    case CHIP_ERROR_NO_CONNECTION_HANDLER.AsInteger():
        desc = "No connection handler";
        break;
    case CHIP_ERROR_TOO_MANY_PEER_NODES.AsInteger():
        desc = "Too many peer nodes";
        break;
    case CHIP_ERROR_SENTINEL.AsInteger():
        desc = "Internal sentinel";
        break;
    case CHIP_ERROR_NO_MEMORY.AsInteger():
        desc = "No memory";
        break;
    case CHIP_ERROR_NO_MESSAGE_HANDLER.AsInteger():
        desc = "No message handler";
        break;
    case CHIP_ERROR_MESSAGE_INCOMPLETE.AsInteger():
        desc = "Message incomplete";
        break;
    case CHIP_ERROR_DATA_NOT_ALIGNED.AsInteger():
        desc = "Data not aligned";
        break;
    case CHIP_ERROR_UNKNOWN_KEY_TYPE.AsInteger():
        desc = "Unknown key type";
        break;
    case CHIP_ERROR_KEY_NOT_FOUND.AsInteger():
        desc = "Key not found";
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE.AsInteger():
        desc = "Wrong encryption type";
        break;
    case CHIP_ERROR_TOO_MANY_KEYS.AsInteger():
        desc = "Too many keys";
        break;
    case CHIP_ERROR_INTEGRITY_CHECK_FAILED.AsInteger():
        desc = "Integrity check failed";
        break;
    case CHIP_ERROR_INVALID_SIGNATURE.AsInteger():
        desc = "Invalid signature";
        break;
    case CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION.AsInteger():
        desc = "Unsupported message version";
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE.AsInteger():
        desc = "Unsupported encryption type";
        break;
    case CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE.AsInteger():
        desc = "Unsupported signature type";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_LENGTH.AsInteger():
        desc = "Invalid message length";
        break;
    case CHIP_ERROR_BUFFER_TOO_SMALL.AsInteger():
        desc = "Buffer too small";
        break;
    case CHIP_ERROR_DUPLICATE_KEY_ID.AsInteger():
        desc = "Duplicate key id";
        break;
    case CHIP_ERROR_WRONG_KEY_TYPE.AsInteger():
        desc = "Wrong key type";
        break;
    case CHIP_ERROR_WELL_UNINITIALIZED.AsInteger():
        desc = "Well uninitialized";
        break;
    case CHIP_ERROR_WELL_EMPTY.AsInteger():
        desc = "Well empty";
        break;
    case CHIP_ERROR_INVALID_STRING_LENGTH.AsInteger():
        desc = "Invalid string length";
        break;
    case CHIP_ERROR_INVALID_LIST_LENGTH.AsInteger():
        desc = "invalid list length";
        break;
    case CHIP_ERROR_INVALID_INTEGRITY_TYPE.AsInteger():
        desc = "Invalid integrity type";
        break;
    case CHIP_END_OF_TLV.AsInteger():
        desc = "End of TLV";
        break;
    case CHIP_ERROR_TLV_UNDERRUN.AsInteger():
        desc = "TLV underrun";
        break;
    case CHIP_ERROR_INVALID_TLV_ELEMENT.AsInteger():
        desc = "Invalid TLV element";
        break;
    case CHIP_ERROR_INVALID_TLV_TAG.AsInteger():
        desc = "Invalid TLV tag";
        break;
    case CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG.AsInteger():
        desc = "Unknown implicit TLV tag";
        break;
    case CHIP_ERROR_WRONG_TLV_TYPE.AsInteger():
        desc = "Wrong TLV type";
        break;
    case CHIP_ERROR_TLV_CONTAINER_OPEN.AsInteger():
        desc = "TLV container open";
        break;
    case CHIP_ERROR_INVALID_TRANSFER_MODE.AsInteger():
        desc = "Invalid transfer mode";
        break;
    case CHIP_ERROR_INVALID_PROFILE_ID.AsInteger():
        desc = "Invalid profile id";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_TYPE.AsInteger():
        desc = "Invalid message type";
        break;
    case CHIP_ERROR_UNEXPECTED_TLV_ELEMENT.AsInteger():
        desc = "Unexpected TLV element";
        break;
    case CHIP_ERROR_STATUS_REPORT_RECEIVED.AsInteger():
        desc = "Status Report received from peer";
        break;
    case CHIP_ERROR_NOT_IMPLEMENTED.AsInteger():
        desc = "Not Implemented";
        break;
    case CHIP_ERROR_INVALID_ADDRESS.AsInteger():
        desc = "Invalid address";
        break;
    case CHIP_ERROR_INVALID_ARGUMENT.AsInteger():
        desc = "Invalid argument";
        break;
    case CHIP_ERROR_TLV_TAG_NOT_FOUND.AsInteger():
        desc = "TLV tag not found";
        break;
    case CHIP_ERROR_MISSING_SECURE_SESSION.AsInteger():
        desc = "Missing secure session";
        break;
    case CHIP_ERROR_INVALID_ADMIN_SUBJECT.AsInteger():
        desc = "CaseAdminSubject is not valid";
        break;
    case CHIP_ERROR_INSUFFICIENT_PRIVILEGE.AsInteger():
        desc = "Required privilege was insufficient during an operation";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_REPORT_IB.AsInteger():
        desc = "Malformed Interacton Model Attribute Report IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB.AsInteger():
        desc = "Malformed Interacton Model Command Data IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_STATUS_IB.AsInteger():
        desc = "Malformed Interacton Model Event Status IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_STATUS_RESPONSE_MESSAGE.AsInteger():
        desc = "Malformed Interacton Model Status Response IB";
        break;
    case CHIP_ERROR_INVALID_PATH_LIST.AsInteger():
        desc = "Invalid TLV path list";
        break;
    case CHIP_ERROR_INVALID_DATA_LIST.AsInteger():
        desc = "Invalid TLV data list";
        break;
    case CHIP_ERROR_TRANSACTION_CANCELED.AsInteger():
        desc = "Transaction canceled";
        break;
    case CHIP_ERROR_LISTENER_ALREADY_STARTED.AsInteger():
        desc = "Listener already started";
        break;
    case CHIP_ERROR_LISTENER_ALREADY_STOPPED.AsInteger():
        desc = "Listener already stopped";
        break;
    case CHIP_ERROR_INVALID_SUBSCRIPTION.AsInteger():
        desc = "Invalid Subscription Id";
        break;
    case CHIP_ERROR_TIMEOUT.AsInteger():
        desc = "Timeout";
        break;
    case CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR.AsInteger():
        desc = "Invalid device descriptor";
        break;
    case CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION.AsInteger():
        desc = "Unsupported device descriptor version";
        break;
    case CHIP_END_OF_INPUT.AsInteger():
        desc = "End of input";
        break;
    case CHIP_ERROR_RATE_LIMIT_EXCEEDED.AsInteger():
        desc = "Rate limit exceeded";
        break;
    case CHIP_ERROR_SECURITY_MANAGER_BUSY.AsInteger():
        desc = "Security manager busy";
        break;
    case CHIP_ERROR_INVALID_PASE_PARAMETER.AsInteger():
        desc = "Invalid PASE parameter";
        break;
    case CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1.AsInteger():
        desc = "PASE supports only Config1";
        break;
    case CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS.AsInteger():
        desc = "No supported PASE configurations in common";
        break;
    case CHIP_ERROR_INVALID_PASE_CONFIGURATION.AsInteger():
        desc = "Invalid PASE configuration";
        break;
    case CHIP_ERROR_KEY_CONFIRMATION_FAILED.AsInteger():
        desc = "Key confirmation failed";
        break;
    case CHIP_ERROR_INVALID_USE_OF_SESSION_KEY.AsInteger():
        desc = "Invalid use of session key";
        break;
    case CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY.AsInteger():
        desc = "Connection closed unexpectedly";
        break;
    case CHIP_ERROR_MISSING_TLV_ELEMENT.AsInteger():
        desc = "Missing TLV element";
        break;
    case CHIP_ERROR_RANDOM_DATA_UNAVAILABLE.AsInteger():
        desc = "Random data unavailable";
        break;
    case CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT.AsInteger():
        desc = "Unsupported type in host/port list";
        break;
    case CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX.AsInteger():
        desc = "Invalid suffix index in host/port list";
        break;
    case CHIP_ERROR_HOST_PORT_LIST_EMPTY.AsInteger():
        desc = "Host/port empty";
        break;
    case CHIP_ERROR_UNSUPPORTED_AUTH_MODE.AsInteger():
        desc = "Unsupported authentication mode";
        break;
    case CHIP_ERROR_INVALID_SERVICE_EP.AsInteger():
        desc = "Invalid service endpoint";
        break;
    case CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE.AsInteger():
        desc = "Invalid directory entry type";
        break;
    case CHIP_ERROR_FORCED_RESET.AsInteger():
        desc = "Service manager forced reset";
        break;
    case CHIP_ERROR_NO_ENDPOINT.AsInteger():
        desc = "No endpoint was available to send the message";
        break;
    case CHIP_ERROR_INVALID_DESTINATION_NODE_ID.AsInteger():
        desc = "Invalid destination node id";
        break;
    case CHIP_ERROR_NOT_CONNECTED.AsInteger():
        desc = "Not connected";
        break;
    case CHIP_ERROR_NO_SW_UPDATE_AVAILABLE.AsInteger():
        desc = "No SW update available";
        break;
    case CHIP_ERROR_CA_CERT_NOT_FOUND.AsInteger():
        desc = "CA certificate not found";
        break;
    case CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED.AsInteger():
        desc = "Certificate path length constraint exceeded";
        break;
    case CHIP_ERROR_CERT_PATH_TOO_LONG.AsInteger():
        desc = "Certificate path too long";
        break;
    case CHIP_ERROR_CERT_USAGE_NOT_ALLOWED.AsInteger():
        desc = "Requested certificate usage is not allowed";
        break;
    case CHIP_ERROR_CERT_EXPIRED.AsInteger():
        desc = "Certificate expired";
        break;
    case CHIP_ERROR_CERT_NOT_VALID_YET.AsInteger():
        desc = "Certificate not yet valid";
        break;
    case CHIP_ERROR_UNSUPPORTED_CERT_FORMAT.AsInteger():
        desc = "Unsupported certificate format";
        break;
    case CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE.AsInteger():
        desc = "Unsupported elliptic curve";
        break;
    case CHIP_CERT_NOT_USED.AsInteger():
        desc = "Certificate was not used in chain validation";
        break;
    case CHIP_ERROR_CERT_NOT_FOUND.AsInteger():
        desc = "Certificate not found";
        break;
    case CHIP_ERROR_INVALID_CASE_PARAMETER.AsInteger():
        desc = "Invalid CASE parameter";
        break;
    case CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION.AsInteger():
        desc = "Unsupported CASE configuration";
        break;
    case CHIP_ERROR_CERT_LOAD_FAILED.AsInteger():
        desc = "Unable to load certificate";
        break;
    case CHIP_ERROR_CERT_NOT_TRUSTED.AsInteger():
        desc = "Certificate not trusted";
        break;
    case CHIP_ERROR_INVALID_ACCESS_TOKEN.AsInteger():
        desc = "Invalid access token";
        break;
    case CHIP_ERROR_WRONG_CERT_DN.AsInteger():
        desc = "Wrong certificate distinguished name";
        break;
    case CHIP_ERROR_INVALID_PROVISIONING_BUNDLE.AsInteger():
        desc = "Invalid provisioning bundle";
        break;
    case CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR.AsInteger():
        desc = "Provisioning bundle decryption error";
        break;
    case CHIP_ERROR_PASE_RECONFIGURE_REQUIRED.AsInteger():
        desc = "PASE reconfiguration required";
        break;
    case CHIP_ERROR_WRONG_NODE_ID.AsInteger():
        desc = "Wrong node ID";
        break;
    case CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT.AsInteger():
        desc = "Connection accepted on wrong port";
        break;
    case CHIP_ERROR_CALLBACK_REPLACED.AsInteger():
        desc = "Application callback replaced";
        break;
    case CHIP_ERROR_NO_CASE_AUTH_DELEGATE.AsInteger():
        desc = "No CASE auth delegate set";
        break;
    case CHIP_ERROR_DEVICE_LOCATE_TIMEOUT.AsInteger():
        desc = "Timeout attempting to locate device";
        break;
    case CHIP_ERROR_DEVICE_CONNECT_TIMEOUT.AsInteger():
        desc = "Timeout connecting to device";
        break;
    case CHIP_ERROR_DEVICE_AUTH_TIMEOUT.AsInteger():
        desc = "Timeout authenticating device";
        break;
    case CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED.AsInteger():
        desc = "Message not acknowledged after max retries";
        break;
    case CHIP_ERROR_RETRANS_TABLE_FULL.AsInteger():
        desc = "Retransmit Table is already full";
        break;
    case CHIP_ERROR_INVALID_ACK_MESSAGE_COUNTER.AsInteger():
        desc = "Invalid acknowledged message counter";
        break;
    case CHIP_ERROR_SEND_THROTTLED.AsInteger():
        desc = "Sending to peer is throttled on this Exchange";
        break;
    case CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE.AsInteger():
        desc = "Message version not supported by current exchange context";
        break;
    case CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE.AsInteger():
        desc = "Required feature not supported by this configuration";
        break;
    case CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR.AsInteger():
        desc = "Unsolicited msg with originator bit clear";
        break;
    case CHIP_ERROR_INVALID_FABRIC_INDEX.AsInteger():
        desc = "Invalid Fabric Index";
        break;
    case CHIP_ERROR_TOO_MANY_CONNECTIONS.AsInteger():
        desc = "Too many connections";
        break;
    case CHIP_ERROR_SHUT_DOWN.AsInteger():
        desc = "The operation was cancelled because a shut down was initiated";
        break;
    case CHIP_ERROR_CANCELLED.AsInteger():
        desc = "The operation has been cancelled";
        break;
    case CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED.AsInteger():
        desc = "DRBG entropy source failed to generate entropy data";
        break;
    case CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED.AsInteger():
        desc = "Message counter exhausted";
        break;
    case CHIP_ERROR_FABRIC_EXISTS.AsInteger():
        desc = "Trying to add a NOC for a fabric that already exists";
        break;
    case CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER.AsInteger():
        desc = "Key not found error code received from peer";
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER.AsInteger():
        desc = "Wrong encryption type error code received from peer";
        break;
    case CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER.AsInteger():
        desc = "Unknown key type error code received from peer";
        break;
    case CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER.AsInteger():
        desc = "Invalid use of session key error code received from peer";
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER.AsInteger():
        desc = "Unsupported encryption type error code received from peer";
        break;
    case CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER.AsInteger():
        desc = "Internal key error code received from peer";
        break;
    case CHIP_ERROR_INVALID_KEY_ID.AsInteger():
        desc = "Invalid key identifier";
        break;
    case CHIP_ERROR_INVALID_TIME.AsInteger():
        desc = "Valid time value is not available";
        break;
    case CHIP_ERROR_LOCKING_FAILURE.AsInteger():
        desc = "Failure to lock/unlock OS-provided lock";
        break;
    case CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG.AsInteger():
        desc = "Unsupported passcode encryption configuration";
        break;
    case CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED.AsInteger():
        desc = "Passcode authentication failed";
        break;
    case CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED.AsInteger():
        desc = "Passcode fingerprint failed";
        break;
    case CHIP_ERROR_SERIALIZATION_ELEMENT_NULL.AsInteger():
        desc = "Element requested is null";
        break;
    case CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM.AsInteger():
        desc = "Certificate not signed with required signature algorithm";
        break;
    case CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM.AsInteger():
        desc = "CHIP signature not signed with required signature algorithm";
        break;
    case CHIP_ERROR_SCHEMA_MISMATCH.AsInteger():
        desc = "Schema mismatch";
        break;
    case CHIP_ERROR_INVALID_INTEGER_VALUE.AsInteger():
        desc = "Invalid integer value";
        break;
    case CHIP_ERROR_CASE_RECONFIG_REQUIRED.AsInteger():
        desc = "CASE reconfiguration required";
        break;
    case CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS.AsInteger():
        desc = "Too many CASE reconfigurations were received";
        break;
    case CHIP_ERROR_BAD_REQUEST.AsInteger():
        desc = "Request cannot be processed or fulfilled";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_FLAG.AsInteger():
        desc = "Invalid message flag";
        break;
    case CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED.AsInteger():
        desc = "Key export protocol required to reconfigure";
        break;
    case CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS.AsInteger():
        desc = "No supported key export protocol configurations in common";
        break;
    case CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION.AsInteger():
        desc = "Invalid key export protocol configuration";
        break;
    case CHIP_ERROR_NO_KEY_EXPORT_DELEGATE.AsInteger():
        desc = "No key export protocol delegate set";
        break;
    case CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST.AsInteger():
        desc = "Unauthorized key export request";
        break;
    case CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE.AsInteger():
        desc = "Unauthorized key export response";
        break;
    case CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED.AsInteger():
        desc = "Exported key authentication failed";
        break;
    case CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES.AsInteger():
        desc = "Too many shared session end nodes";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_IB.AsInteger():
        desc = "Malformed Interaction Model Attribute Data IB";
        break;
    case CHIP_ERROR_WRONG_CERT_TYPE.AsInteger():
        desc = "Wrong certificate type";
        break;
    case CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED.AsInteger():
        desc = "Default event handler not called";
        break;
    case CHIP_ERROR_PERSISTED_STORAGE_FAILED.AsInteger():
        desc = "Persisted storage failed";
        break;
    case CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND.AsInteger():
        desc = "Value not found in the persisted storage";
        break;
    case CHIP_ERROR_IM_FABRIC_DELETED.AsInteger():
        desc = "The fabric is deleted, and the corresponding IM resources are released";
        break;
    case CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED.AsInteger():
        desc = "String context not registered";
        break;
    case CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION.AsInteger():
        desc = "Incompatible data schema version";
        break;
    case CHIP_ERROR_ACCESS_DENIED.AsInteger():
        desc = "The CHIP message is not granted access";
        break;
    case CHIP_ERROR_UNKNOWN_RESOURCE_ID.AsInteger():
        desc = "Unknown resource ID";
        break;
    case CHIP_ERROR_VERSION_MISMATCH.AsInteger():
        desc = "Version mismatch";
        break;
    case CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE.AsInteger():
        desc = "Legacy device doesn't support standalone Thread network creation";
        break;
    case CHIP_ERROR_INCONSISTENT_CONDITIONALITY.AsInteger():
        desc = "The Trait Instance is already being updated with a different conditionality";
        break;
    case CHIP_ERROR_LOCAL_DATA_INCONSISTENT.AsInteger():
        desc = "The local data does not match any known version of the Trait Instance";
        break;
    case CHIP_EVENT_ID_FOUND.AsInteger():
        desc = "Event ID matching criteria was found";
        break;
    case CHIP_ERROR_INTERNAL.AsInteger():
        desc = "Internal error";
        break;
    case CHIP_ERROR_OPEN_FAILED.AsInteger():
        desc = "Open file failed";
        break;
    case CHIP_ERROR_READ_FAILED.AsInteger():
        desc = "Read from file failed";
        break;
    case CHIP_ERROR_WRITE_FAILED.AsInteger():
        desc = "Write to file failed";
        break;
    case CHIP_ERROR_DECODE_FAILED.AsInteger():
        desc = "Decoding failed";
        break;
    case CHIP_ERROR_SESSION_KEY_SUSPENDED.AsInteger():
        desc = "Session key suspended";
        break;
    case CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN.AsInteger():
        desc = "Unsupported wireless regulatory domain";
        break;
    case CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION.AsInteger():
        desc = "Unsupported wireless operating location";
        break;
    case CHIP_ERROR_MDNS_COLLISION.AsInteger():
        desc = "mDNS collision";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB.AsInteger():
        desc = "Malformed Interacton Model Attribute Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB.AsInteger():
        desc = "Malformed Interacton Model Event Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_COMMAND_PATH_IB.AsInteger():
        desc = "Malformed Interacton Model Command Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_IB.AsInteger():
        desc = "Malformed Interacton Model Attribute Status IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB.AsInteger():
        desc = "Malformed Interacton Model Event Data IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_STATUS_IB.AsInteger():
        desc = "Malformed Interacton Model Status IB";
        break;
    case CHIP_ERROR_PEER_NODE_NOT_FOUND.AsInteger():
        desc = "Unable to find the peer node";
        break;
    case CHIP_ERROR_HSM.AsInteger():
        desc = "Hardware security module";
        break;
    case CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED.AsInteger():
        desc = "Intermediate CA not required";
        break;
    case CHIP_ERROR_REAL_TIME_NOT_SYNCED.AsInteger():
        desc = "Real time not synchronized";
        break;
    case CHIP_ERROR_UNEXPECTED_EVENT.AsInteger():
        desc = "Unexpected event";
        break;
    case CHIP_ERROR_ENDPOINT_POOL_FULL.AsInteger():
        desc = "Endpoint pool full";
        break;
    case CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG.AsInteger():
        desc = "Inbound message too big";
        break;
    case CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG.AsInteger():
        desc = "Outbound message too big";
        break;
    case CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED.AsInteger():
        desc = "Duplicate message received";
        break;
    case CHIP_ERROR_INVALID_PUBLIC_KEY.AsInteger():
        desc = "Invalid public key";
        break;
    case CHIP_ERROR_FABRIC_MISMATCH_ON_ICA.AsInteger():
        desc = "Fabric mismatch on ICA";
        break;
    case CHIP_ERROR_MESSAGE_COUNTER_OUT_OF_WINDOW.AsInteger():
        desc = "Message id out of window";
        break;
    case CHIP_ERROR_REBOOT_SIGNAL_RECEIVED.AsInteger():
        desc = "Termination signal is received";
        break;
    case CHIP_ERROR_NO_SHARED_TRUSTED_ROOT.AsInteger():
        desc = "No shared trusted root";
        break;
    case CHIP_ERROR_IM_STATUS_CODE_RECEIVED.AsInteger():
        desc = "Interaction Model Error";
        break;
    case CHIP_ERROR_IM_MALFORMED_COMMAND_STATUS_IB.AsInteger():
        desc = "Malformed Interaction Model Command Status IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_INVOKE_RESPONSE_IB.AsInteger():
        desc = "Malformed Interaction Model Invoke Response IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_INVOKE_REQUEST_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Invoke Request Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_INVOKE_RESPONSE_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Invoke Response Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_REPORT_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Attribute Report Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_WRITE_REQUEST_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Write Request Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_FILTER_IB.AsInteger():
        desc = "Malformed Interaction Model Event Filter IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_READ_REQUEST_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Read Request Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_SUBSCRIBE_REQUEST_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Subscribe Request Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_SUBSCRIBE_RESPONSE_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Subscribe Response Message";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_REPORT_IB.AsInteger():
        desc = "Malformed Interaction Model Event Report IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_CLUSTER_PATH_IB.AsInteger():
        desc = "Malformed Interaction Model Cluster Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB.AsInteger():
        desc = "Malformed Interaction Model Data Version Filter IB";
        break;
    case CHIP_ERROR_NOT_FOUND.AsInteger():
        desc = "The item referenced in the function call was not found";
        break;
    case CHIP_ERROR_IM_MALFORMED_TIMED_REQUEST_MESSAGE.AsInteger():
        desc = "Malformed Interaction Model Timed Request Message";
        break;
    case CHIP_ERROR_INVALID_FILE_IDENTIFIER.AsInteger():
        desc = "The file identifier, encoded in the first few bytes of a processed file, has unexpected value";
        break;
    case CHIP_ERROR_BUSY.AsInteger():
        desc = "The Resource is busy and cannot process the request";
        break;
    case CHIP_ERROR_MAX_RETRY_EXCEEDED.AsInteger():
        desc = "The maximum retry limit has been exceeded";
        break;
    case CHIP_ERROR_PROVIDER_LIST_EXHAUSTED.AsInteger():
        desc = "The provider list has been exhausted";
        break;
    case CHIP_ERROR_ANOTHER_COMMISSIONING_IN_PROGRESS.AsInteger():
        desc = "Another commissioning in progress";
        break;
    case CHIP_ERROR_INVALID_SCHEME_PREFIX.AsInteger():
        desc = "The scheme field contains an invalid prefix";
        break;
    case CHIP_ERROR_MISSING_URI_SEPARATOR.AsInteger():
        desc = "The URI separator is missing";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "CHIP", err, desc);

    return true;
}

} // namespace chip
