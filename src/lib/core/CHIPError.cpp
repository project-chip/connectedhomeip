/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <core/CHIPCore.h>
#include <support/ErrorStr.h>

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

    if (err < CHIP_CONFIG_CORE_ERROR_MIN || err > CHIP_CONFIG_CORE_ERROR_MAX)
    {
        return false;
    }

#if !CHIP_CONFIG_SHORT_ERROR_STR
    switch (err)
    {
    case CHIP_ERROR_TOO_MANY_CONNECTIONS:
        desc = "Too many connections";
        break;
    case CHIP_ERROR_SENDING_BLOCKED:
        desc = "Sending blocked";
        break;
    case CHIP_ERROR_CONNECTION_ABORTED:
        desc = "Connection aborted";
        break;
    case CHIP_ERROR_INCORRECT_STATE:
        desc = "Incorrect state";
        break;
    case CHIP_ERROR_MESSAGE_TOO_LONG:
        desc = "Message too long";
        break;
    case CHIP_ERROR_UNSUPPORTED_EXCHANGE_VERSION:
        desc = "Unsupported exchange version";
        break;
    case CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS:
        desc = "Too many unsolicited message handlers";
        break;
    case CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER:
        desc = "No unsolicited message handler";
        break;
    case CHIP_ERROR_NO_CONNECTION_HANDLER:
        desc = "No connection handler";
        break;
    case CHIP_ERROR_TOO_MANY_PEER_NODES:
        desc = "Too many peer nodes";
        break;
    case CHIP_ERROR_SENTINEL:
        desc = "Internal sentinel";
        break;
    case CHIP_ERROR_NO_MEMORY:
        desc = "No memory";
        break;
    case CHIP_ERROR_NO_MESSAGE_HANDLER:
        desc = "No message handler";
        break;
    case CHIP_ERROR_MESSAGE_INCOMPLETE:
        desc = "Message incomplete";
        break;
    case CHIP_ERROR_DATA_NOT_ALIGNED:
        desc = "Data not aligned";
        break;
    case CHIP_ERROR_UNKNOWN_KEY_TYPE:
        desc = "Unknown key type";
        break;
    case CHIP_ERROR_KEY_NOT_FOUND:
        desc = "Key not found";
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE:
        desc = "Wrong encryption type";
        break;
    case CHIP_ERROR_TOO_MANY_KEYS:
        desc = "Too many keys";
        break;
    case CHIP_ERROR_INTEGRITY_CHECK_FAILED:
        desc = "Integrity check failed";
        break;
    case CHIP_ERROR_INVALID_SIGNATURE:
        desc = "Invalid signature";
        break;
    case CHIP_ERROR_UNSUPPORTED_MESSAGE_VERSION:
        desc = "Unsupported message version";
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE:
        desc = "Unsupported encryption type";
        break;
    case CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE:
        desc = "Unsupported signature type";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_LENGTH:
        desc = "Invalid message length";
        break;
    case CHIP_ERROR_BUFFER_TOO_SMALL:
        desc = "Buffer too small";
        break;
    case CHIP_ERROR_DUPLICATE_KEY_ID:
        desc = "Duplicate key id";
        break;
    case CHIP_ERROR_WRONG_KEY_TYPE:
        desc = "Wrong key type";
        break;
    case CHIP_ERROR_WELL_UNINITIALIZED:
        desc = "Well uninitialized";
        break;
    case CHIP_ERROR_WELL_EMPTY:
        desc = "Well empty";
        break;
    case CHIP_ERROR_INVALID_STRING_LENGTH:
        desc = "Invalid string length";
        break;
    case CHIP_ERROR_INVALID_LIST_LENGTH:
        desc = "invalid list length";
        break;
    case CHIP_ERROR_INVALID_INTEGRITY_TYPE:
        desc = "Invalid integrity type";
        break;
    case CHIP_END_OF_TLV:
        desc = "End of TLV";
        break;
    case CHIP_ERROR_TLV_UNDERRUN:
        desc = "TLV underrun";
        break;
    case CHIP_ERROR_INVALID_TLV_ELEMENT:
        desc = "Invalid TLV element";
        break;
    case CHIP_ERROR_INVALID_TLV_TAG:
        desc = "Invalid TLV tag";
        break;
    case CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG:
        desc = "Unknown implicit TLV tag";
        break;
    case CHIP_ERROR_WRONG_TLV_TYPE:
        desc = "Wrong TLV type";
        break;
    case CHIP_ERROR_TLV_CONTAINER_OPEN:
        desc = "TLV container open";
        break;
    case CHIP_ERROR_INVALID_TRANSFER_MODE:
        desc = "Invalid transfer mode";
        break;
    case CHIP_ERROR_INVALID_PROFILE_ID:
        desc = "Invalid profile id";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_TYPE:
        desc = "Invalid message type";
        break;
    case CHIP_ERROR_UNEXPECTED_TLV_ELEMENT:
        desc = "Unexpected TLV element";
        break;
    case CHIP_ERROR_STATUS_REPORT_RECEIVED:
        desc = "Status Report received from peer";
        break;
    case CHIP_ERROR_NOT_IMPLEMENTED:
        desc = "Not Implemented";
        break;
    case CHIP_ERROR_INVALID_ADDRESS:
        desc = "Invalid address";
        break;
    case CHIP_ERROR_INVALID_ARGUMENT:
        desc = "Invalid argument";
        break;
    case CHIP_ERROR_TLV_TAG_NOT_FOUND:
        desc = "TLV tag not found";
        break;

    case CHIP_ERROR_INVALID_PATH_LIST:
        desc = "Invalid TLV path list";
        break;
    case CHIP_ERROR_INVALID_DATA_LIST:
        desc = "Invalid TLV data list";
        break;
    case CHIP_ERROR_TRANSACTION_CANCELED:
        desc = "Transaction canceled";
        break;
    case CHIP_ERROR_LISTENER_ALREADY_STARTED:
        desc = "Listener already started";
        break;
    case CHIP_ERROR_LISTENER_ALREADY_STOPPED:
        desc = "Listener already stopped";
        break;
    case CHIP_ERROR_UNKNOWN_TOPIC:
        desc = "Unknown Topic";
        break;
    case CHIP_ERROR_TIMEOUT:
        desc = "Timeout";
        break;
    case CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR:
        desc = "Invalid device descriptor";
        break;
    case CHIP_ERROR_UNSUPPORTED_DEVICE_DESCRIPTOR_VERSION:
        desc = "Unsupported device descriptor version";
        break;
    case CHIP_END_OF_INPUT:
        desc = "End of input";
        break;
    case CHIP_ERROR_RATE_LIMIT_EXCEEDED:
        desc = "Rate limit exceeded";
        break;
    case CHIP_ERROR_SECURITY_MANAGER_BUSY:
        desc = "Security manager busy";
        break;
    case CHIP_ERROR_INVALID_PASE_PARAMETER:
        desc = "Invalid PASE parameter";
        break;
    case CHIP_ERROR_PASE_SUPPORTS_ONLY_CONFIG1:
        desc = "PASE supports only Config1";
        break;
    case CHIP_ERROR_NO_COMMON_PASE_CONFIGURATIONS:
        desc = "No supported PASE configurations in common";
        break;
    case CHIP_ERROR_INVALID_PASE_CONFIGURATION:
        desc = "Invalid PASE configuration";
        break;
    case CHIP_ERROR_KEY_CONFIRMATION_FAILED:
        desc = "Key confirmation failed";
        break;
    case CHIP_ERROR_INVALID_USE_OF_SESSION_KEY:
        desc = "Invalid use of session key";
        break;
    case CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY:
        desc = "Connection closed unexpectedly";
        break;
    case CHIP_ERROR_MISSING_TLV_ELEMENT:
        desc = "Missing TLV element";
        break;
    case CHIP_ERROR_RANDOM_DATA_UNAVAILABLE:
        desc = "Random data unavailable";
        break;
    case CHIP_ERROR_UNSUPPORTED_HOST_PORT_ELEMENT:
        desc = "Unsupported type in host/port list";
        break;
    case CHIP_ERROR_INVALID_HOST_SUFFIX_INDEX:
        desc = "Invalid suffix index in host/port list";
        break;
    case CHIP_ERROR_HOST_PORT_LIST_EMPTY:
        desc = "Host/port empty";
        break;
    case CHIP_ERROR_UNSUPPORTED_AUTH_MODE:
        desc = "Unsupported authentication mode";
        break;
    case CHIP_ERROR_INVALID_SERVICE_EP:
        desc = "Invalid service endpoint";
        break;
    case CHIP_ERROR_INVALID_DIRECTORY_ENTRY_TYPE:
        desc = "Invalid directory entry type";
        break;
    case CHIP_ERROR_FORCED_RESET:
        desc = "Service manager forced reset";
        break;
    case CHIP_ERROR_NO_ENDPOINT:
        desc = "No endpoint was available to send the message";
        break;
    case CHIP_ERROR_INVALID_DESTINATION_NODE_ID:
        desc = "Invalid destination node id";
        break;
    case CHIP_ERROR_NOT_CONNECTED:
        desc = "Not connected";
        break;
    case CHIP_ERROR_NO_SW_UPDATE_AVAILABLE:
        desc = "No SW update available";
        break;
    case CHIP_ERROR_CA_CERT_NOT_FOUND:
        desc = "CA certificate not found";
        break;
    case CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED:
        desc = "Certificate path length constraint exceeded";
        break;
    case CHIP_ERROR_CERT_PATH_TOO_LONG:
        desc = "Certificate path too long";
        break;
    case CHIP_ERROR_CERT_USAGE_NOT_ALLOWED:
        desc = "Requested certificate usage is not allowed";
        break;
    case CHIP_ERROR_CERT_EXPIRED:
        desc = "Certificate expired";
        break;
    case CHIP_ERROR_CERT_NOT_VALID_YET:
        desc = "Certificate not yet valid";
        break;
    case CHIP_ERROR_UNSUPPORTED_CERT_FORMAT:
        desc = "Unsupported certificate format";
        break;
    case CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE:
        desc = "Unsupported elliptic curve";
        break;
    case CHIP_CERT_NOT_USED:
        desc = "Certificate was not used in chain validation";
        break;
    case CHIP_ERROR_CERT_NOT_FOUND:
        desc = "Certificate not found";
        break;
    case CHIP_ERROR_INVALID_CASE_PARAMETER:
        desc = "Invalid CASE parameter";
        break;
    case CHIP_ERROR_UNSUPPORTED_CASE_CONFIGURATION:
        desc = "Unsupported CASE configuration";
        break;
    case CHIP_ERROR_CERT_LOAD_FAILED:
        desc = "Unable to load certificate";
        break;
    case CHIP_ERROR_CERT_NOT_TRUSTED:
        desc = "Certificate not trusted";
        break;
    case CHIP_ERROR_INVALID_ACCESS_TOKEN:
        desc = "Invalid access token";
        break;
    case CHIP_ERROR_WRONG_CERT_SUBJECT:
        desc = "Wrong certificate subject";
        break;
    case CHIP_ERROR_INVALID_PROVISIONING_BUNDLE:
        desc = "Invalid provisioning bundle";
        break;
    case CHIP_ERROR_PROVISIONING_BUNDLE_DECRYPTION_ERROR:
        desc = "Provisioning bundle decryption error";
        break;
    case CHIP_ERROR_PASE_RECONFIGURE_REQUIRED:
        desc = "PASE reconfiguration required";
        break;
    case CHIP_ERROR_WRONG_NODE_ID:
        desc = "Wrong node ID";
        break;
    case CHIP_ERROR_CONN_ACCEPTED_ON_WRONG_PORT:
        desc = "Connection accepted on wrong port";
        break;
    case CHIP_ERROR_CALLBACK_REPLACED:
        desc = "Application callback replaced";
        break;
    case CHIP_ERROR_NO_CASE_AUTH_DELEGATE:
        desc = "No CASE auth delegate set";
        break;
    case CHIP_ERROR_DEVICE_LOCATE_TIMEOUT:
        desc = "Timeout attempting to locate device";
        break;
    case CHIP_ERROR_DEVICE_CONNECT_TIMEOUT:
        desc = "Timeout connecting to device";
        break;
    case CHIP_ERROR_DEVICE_AUTH_TIMEOUT:
        desc = "Timeout authenticating device";
        break;
    case CHIP_ERROR_MESSAGE_NOT_ACKNOWLEDGED:
        desc = "Message not acknowledged after max retries";
        break;
    case CHIP_ERROR_RETRANS_TABLE_FULL:
        desc = "Retransmit Table is already full";
        break;
    case CHIP_ERROR_INVALID_ACK_ID:
        desc = "Invalid Acknowledgment Id";
        break;
    case CHIP_ERROR_SEND_THROTTLED:
        desc = "Sending to peer is throttled on this Exchange";
        break;
    case CHIP_ERROR_WRONG_MSG_VERSION_FOR_EXCHANGE:
        desc = "Message version not supported by current exchange context";
        break;
    case CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE:
        desc = "Required feature not supported by this configuration";
        break;
    case CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR:
        desc = "Unsolicited msg with originator bit clear";
        break;
    case CHIP_ERROR_INVALID_FABRIC_ID:
        desc = "Invalid Fabric Id";
        break;
    case CHIP_ERROR_DRBG_ENTROPY_SOURCE_FAILED:
        desc = "DRBG entropy source failed to generate entropy data";
        break;
    case CHIP_ERROR_NO_TAKE_AUTH_DELEGATE:
        desc = "No TAKE auth delegate set";
        break;
    case CHIP_ERROR_TAKE_RECONFIGURE_REQUIRED:
        desc = "TAKE requires a reconfigure";
        break;
    case CHIP_ERROR_TAKE_REAUTH_POSSIBLE:
        desc = "TAKE can do a reauthentication";
        break;
    case CHIP_ERROR_INVALID_TAKE_PARAMETER:
        desc = "TAKE received an invalid parameter";
        break;
    case CHIP_ERROR_UNSUPPORTED_TAKE_CONFIGURATION:
        desc = "TAKE Unsupported configuration";
        break;
    case CHIP_ERROR_TAKE_TOKEN_IDENTIFICATION_FAILED:
        desc = "TAKE token identification failed";
        break;
    case CHIP_ERROR_INVALID_TOKENPAIRINGBUNDLE:
        desc = "Invalid Token Pairing Bundle";
        break;
    case CHIP_ERROR_UNSUPPORTED_TOKENPAIRINGBUNDLE_VERSION:
        desc = "Unsupported Token Pairing Bundle version";
        break;
    case CHIP_ERROR_KEY_NOT_FOUND_FROM_PEER:
        desc = "Key not found error code received from peer";
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER:
        desc = "Wrong encryption type error code received from peer";
        break;
    case CHIP_ERROR_UNKNOWN_KEY_TYPE_FROM_PEER:
        desc = "Unknown key type error code received from peer";
        break;
    case CHIP_ERROR_INVALID_USE_OF_SESSION_KEY_FROM_PEER:
        desc = "Invalid use of session key error code received from peer";
        break;
    case CHIP_ERROR_UNSUPPORTED_ENCRYPTION_TYPE_FROM_PEER:
        desc = "Unsupported encryption type error code received from peer";
        break;
    case CHIP_ERROR_INTERNAL_KEY_ERROR_FROM_PEER:
        desc = "Internal key error code received from peer";
        break;
    case CHIP_ERROR_INVALID_KEY_ID:
        desc = "Invalid key identifier";
        break;
    case CHIP_ERROR_INVALID_TIME:
        desc = "Valid time value is not available";
        break;
    case CHIP_ERROR_LOCKING_FAILURE:
        desc = "Failure to lock/unlock OS-provided lock";
        break;
    case CHIP_ERROR_UNSUPPORTED_PASSCODE_CONFIG:
        desc = "Unsupported passcode encryption configuration";
        break;
    case CHIP_ERROR_PASSCODE_AUTHENTICATION_FAILED:
        desc = "Passcode authentication failed";
        break;
    case CHIP_ERROR_PASSCODE_FINGERPRINT_FAILED:
        desc = "Passcode fingerprint failed";
        break;
    case CHIP_ERROR_SERIALIZATION_ELEMENT_NULL:
        desc = "Element requested is null";
        break;
    case CHIP_ERROR_WRONG_CERT_SIGNATURE_ALGORITHM:
        desc = "Certificate not signed with required signature algorithm";
        break;
    case CHIP_ERROR_WRONG_CHIP_SIGNATURE_ALGORITHM:
        desc = "CHIP signature not signed with required signature algorithm";
        break;
    case CHIP_ERROR_SCHEMA_MISMATCH:
        desc = "Schema mismatch";
        break;
    case CHIP_ERROR_INVALID_INTEGER_VALUE:
        desc = "Invalid integer value";
        break;
    case CHIP_ERROR_CASE_RECONFIG_REQUIRED:
        desc = "CASE reconfiguration required";
        break;
    case CHIP_ERROR_TOO_MANY_CASE_RECONFIGURATIONS:
        desc = "Too many CASE reconfigurations were received";
        break;
    case CHIP_ERROR_BAD_REQUEST:
        desc = "Request cannot be processed or fulfilled";
        break;
    case CHIP_ERROR_INVALID_MESSAGE_FLAG:
        desc = "Invalid message flag";
        break;
    case CHIP_ERROR_KEY_EXPORT_RECONFIGURE_REQUIRED:
        desc = "Key export protocol required to reconfigure";
        break;
    case CHIP_ERROR_NO_COMMON_KEY_EXPORT_CONFIGURATIONS:
        desc = "No supported key export protocol configurations in common";
        break;
    case CHIP_ERROR_INVALID_KEY_EXPORT_CONFIGURATION:
        desc = "Invalid key export protocol configuration";
        break;
    case CHIP_ERROR_NO_KEY_EXPORT_DELEGATE:
        desc = "No key export protocol delegate set";
        break;
    case CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_REQUEST:
        desc = "Unauthorized key export request";
        break;
    case CHIP_ERROR_UNAUTHORIZED_KEY_EXPORT_RESPONSE:
        desc = "Unauthorized key export response";
        break;
    case CHIP_ERROR_EXPORTED_KEY_AUTHENTICATION_FAILED:
        desc = "Exported key authentication failed";
        break;
    case CHIP_ERROR_TOO_MANY_SHARED_SESSION_END_NODES:
        desc = "Too many shared session end nodes";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_DATA_ELEMENT:
        desc = "Malformed Interaction Model Attribute DataElement";
        break;
    case CHIP_ERROR_WRONG_CERT_TYPE:
        desc = "Wrong certificate type";
        break;
    case CHIP_ERROR_DEFAULT_EVENT_HANDLER_NOT_CALLED:
        desc = "Default event handler not called";
        break;
    case CHIP_ERROR_PERSISTED_STORAGE_FAILED:
        desc = "Persisted storage failed";
        break;
    case CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND:
        desc = "Value not found in the persisted storage";
        break;
    case CHIP_ERROR_PROFILE_STRING_CONTEXT_ALREADY_REGISTERED:
        desc = "String context already registered";
        break;
    case CHIP_ERROR_PROFILE_STRING_CONTEXT_NOT_REGISTERED:
        desc = "String context not registered";
        break;
    case CHIP_ERROR_INCOMPATIBLE_SCHEMA_VERSION:
        desc = "Incompatible data schema version";
        break;
    case CHIP_ERROR_MISMATCH_UPDATE_REQUIRED_VERSION:
        desc = "Update Required Version mismatch";
        break;
    case CHIP_ERROR_ACCESS_DENIED:
        desc = "The CHIP message is not granted access";
        break;
    case CHIP_ERROR_UNKNOWN_RESOURCE_ID:
        desc = "Unknown resource ID";
        break;
    case CHIP_ERROR_VERSION_MISMATCH:
        desc = "Version mismatch";
        break;
    case CHIP_ERROR_UNSUPPORTED_THREAD_NETWORK_CREATE:
        desc = "Legacy device doesn't support standalone Thread network creation";
        break;
    case CHIP_ERROR_INCONSISTENT_CONDITIONALITY:
        desc = "The Trait Instance is already being updated with a different conditionality";
        break;
    case CHIP_ERROR_LOCAL_DATA_INCONSISTENT:
        desc = "The local data does not match any known version of the Trait Instance";
        break;
    case CHIP_EVENT_ID_FOUND:
        desc = "Event ID matching criteria was found";
        break;
    case CHIP_ERROR_INTERNAL:
        desc = "Internal error";
        break;
    case CHIP_ERROR_OPEN_FAILED:
        desc = "Open file failed";
        break;
    case CHIP_ERROR_READ_FAILED:
        desc = "Read from file failed";
        break;
    case CHIP_ERROR_WRITE_FAILED:
        desc = "Write to file failed";
        break;
    case CHIP_ERROR_DECODE_FAILED:
        desc = "Decoding failed";
        break;
    case CHIP_ERROR_SESSION_KEY_SUSPENDED:
        desc = "Session key suspended";
        break;
    case CHIP_ERROR_UNSUPPORTED_WIRELESS_REGULATORY_DOMAIN:
        desc = "Unsupported wireless regulatory domain";
        break;
    case CHIP_ERROR_UNSUPPORTED_WIRELESS_OPERATING_LOCATION:
        desc = "Unsupported wireless operating location";
        break;
    case CHIP_ERROR_MDNS_COLLISSION:
        desc = "mDNS collission";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH:
        desc = "Malformed Interacton Model Attribute Path";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_PATH:
        desc = "Malformed Interacton Model Event Path";
        break;
    case CHIP_ERROR_IM_MALFORMED_COMMAND_PATH:
        desc = "Malformed Interacton Model Command Path";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_STATUS_ELEMENT:
        desc = "Malformed Interacton Model Attribute DataElement";
        break;
    case CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_ELEMENT:
        desc = "Malformed Interacton Model Attribute DataElement";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_DATA_ELEMENT:
        desc = "Malformed Interacton Model Event DataElement";
        break;
    case CHIP_ERROR_IM_MALFORMED_STATUS_CODE:
        desc = "Malformed Interacton Model Status Code";
        break;
    case CHIP_ERROR_PEER_NODE_NOT_FOUND:
        desc = "Unable to find the peer node";
        break;
    case CHIP_ERROR_HSM:
        desc = "Hardware security module";
        break;
    case CHIP_ERROR_INTERMEDIATE_CA_NOT_REQUIRED:
        desc = "Intermediate CA not required";
        break;
    case CHIP_ERROR_REAL_TIME_NOT_SYNCED:
        desc = "Real time not synchronized";
        break;
    case CHIP_ERROR_UNEXPECTED_EVENT:
        desc = "Unexpected event";
        break;
    case CHIP_ERROR_ENDPOINT_POOL_FULL:
        desc = "Endpoint pool full";
        break;
    case CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG:
        desc = "Inbound message too big";
        break;
    case CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG:
        desc = "Outbound message too big";
        break;
    case CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED:
        desc = "Duplicate message received";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "CHIP", err, desc);

    return true;
}

} // namespace chip
