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
#include <lib/core/CHIPError.h>

#include <lib/core/CHIPConfig.h>
#include <lib/core/ErrorStr.h>

namespace chip {

static ErrorFormatter sCHIPErrorFormatter = { FormatCHIPError, nullptr };

/**
 * Register a text error formatter for CHIP core errors.
 */
void RegisterCHIPLayerErrorFormatter()
{
    RegisterErrorFormatter(&sCHIPErrorFormatter);
}

/**
 * Deregister a text error formatter for CHIP core errors.
 */
void DeregisterCHIPLayerErrorFormatter()
{
    DeregisterErrorFormatter(&sCHIPErrorFormatter);
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
    case CHIP_ERROR_RECURSION_DEPTH_LIMIT.AsInteger():
        desc = "Recursion depth limit reached";
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
    case CHIP_ERROR_INVALID_UTF8.AsInteger():
        desc = "Character string is not a valid utf-8 encoding";
        break;
    case CHIP_ERROR_INTEGRITY_CHECK_FAILED.AsInteger():
        desc = "Integrity check failed";
        break;
    case CHIP_ERROR_INVALID_SIGNATURE.AsInteger():
        desc = "Invalid signature";
        break;
    case CHIP_ERROR_INVALID_TLV_CHAR_STRING.AsInteger():
        desc = "Invalid TLV Char string encoding.";
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
    case CHIP_ERROR_UNINITIALIZED.AsInteger():
        desc = "Uninitialized";
        break;
    case CHIP_ERROR_INVALID_IPK.AsInteger():
        desc = "Invalid IPK";
        break;
    case CHIP_ERROR_INVALID_STRING_LENGTH.AsInteger():
        desc = "Invalid string length";
        break;
    case CHIP_ERROR_INVALID_LIST_LENGTH.AsInteger():
        desc = "Invalid list length";
        break;
    case CHIP_ERROR_FAILED_DEVICE_ATTESTATION.AsInteger():
        desc = "Failed Device Attestation";
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
    case CHIP_ERROR_INVALID_MESSAGE_TYPE.AsInteger():
        desc = "Invalid message type";
        break;
    case CHIP_ERROR_UNEXPECTED_TLV_ELEMENT.AsInteger():
        desc = "Unexpected TLV element";
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
    case CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB.AsInteger():
        desc = "Malformed Interacton Model Command Data IB";
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
    case CHIP_ERROR_INVALID_SUBSCRIPTION.AsInteger():
        desc = "Invalid Subscription Id";
        break;
    case CHIP_ERROR_TIMEOUT.AsInteger():
        desc = "Timeout";
        break;
    case CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR.AsInteger():
        desc = "Invalid device descriptor";
        break;
    case CHIP_ERROR_INVALID_PASE_PARAMETER.AsInteger():
        desc = "Invalid PASE parameter";
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
    case CHIP_ERROR_HOST_PORT_LIST_EMPTY.AsInteger():
        desc = "Host/port empty";
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
    case CHIP_ERROR_CERT_NOT_FOUND.AsInteger():
        desc = "Certificate not found";
        break;
    case CHIP_ERROR_INVALID_CASE_PARAMETER.AsInteger():
        desc = "Invalid CASE parameter";
        break;
    case CHIP_ERROR_CERT_LOAD_FAILED.AsInteger():
        desc = "Unable to load certificate";
        break;
    case CHIP_ERROR_CERT_NOT_TRUSTED.AsInteger():
        desc = "Certificate not trusted";
        break;
    case CHIP_ERROR_WRONG_CERT_DN.AsInteger():
        desc = "Wrong certificate distinguished name";
        break;
    case CHIP_ERROR_WRONG_NODE_ID.AsInteger():
        desc = "Wrong node ID";
        break;
    case CHIP_ERROR_RETRANS_TABLE_FULL.AsInteger():
        desc = "Retransmit Table is already full";
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
    case CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED.AsInteger():
        desc = "Message counter exhausted";
        break;
    case CHIP_ERROR_FABRIC_EXISTS.AsInteger():
        desc = "Trying to add a NOC for a fabric that already exists";
        break;
    case CHIP_ERROR_ENDPOINT_EXISTS.AsInteger():
        desc = "Trying to add dynamic endpoint that already exists";
        break;
    case CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER.AsInteger():
        desc = "Wrong encryption type error code received from peer";
        break;
    case CHIP_ERROR_INVALID_KEY_ID.AsInteger():
        desc = "Invalid key identifier";
        break;
    case CHIP_ERROR_INVALID_TIME.AsInteger():
        desc = "Valid time value is not available";
        break;
    case CHIP_ERROR_SCHEMA_MISMATCH.AsInteger():
        desc = "Schema mismatch";
        break;
    case CHIP_ERROR_INVALID_INTEGER_VALUE.AsInteger():
        desc = "Invalid integer value";
        break;
    case CHIP_ERROR_BAD_REQUEST.AsInteger():
        desc = "Request cannot be processed or fulfilled";
        break;
    case CHIP_ERROR_WRONG_CERT_TYPE.AsInteger():
        desc = "Wrong certificate type";
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
    case CHIP_ERROR_IN_PROGRESS.AsInteger():
        desc = "The operation is still in progress";
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
    case CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL.AsInteger():
        desc = "The CHIP message's access is restricted by ARL";
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
    case CHIP_ERROR_MDNS_COLLISION.AsInteger():
        desc = "mDNS collision";
        break;
    case CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB.AsInteger():
        desc = "Malformed Interacton Model Attribute Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB.AsInteger():
        desc = "Malformed Interacton Model Event Path IB";
        break;
    case CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB.AsInteger():
        desc = "Malformed Interacton Model Event Data IB";
        break;
    case CHIP_ERROR_PEER_NODE_NOT_FOUND.AsInteger():
        desc = "Unable to find the peer node";
        break;
    case CHIP_ERROR_HSM.AsInteger():
        desc = "Hardware security module";
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
    case CHIP_ERROR_NO_SHARED_TRUSTED_ROOT.AsInteger():
        desc = "No shared trusted root";
        break;
    case CHIP_ERROR_IM_STATUS_CODE_RECEIVED.AsInteger():
        desc = "Interaction Model Error";
        break;
    case CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB.AsInteger():
        desc = "Malformed Interaction Model Data Version Filter IB";
        break;
    case CHIP_ERROR_NOT_FOUND.AsInteger():
        desc = "The item referenced in the function call was not found";
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
    case CHIP_ERROR_INVALID_SCHEME_PREFIX.AsInteger():
        desc = "The scheme field contains an invalid prefix";
        break;
    case CHIP_ERROR_MISSING_URI_SEPARATOR.AsInteger():
        desc = "The URI separator is missing";
        break;
    case CHIP_ERROR_HANDLER_NOT_SET.AsInteger():
        desc = "Callback function or callable object is not set";
        break;
    }
#endif // !CHIP_CONFIG_SHORT_ERROR_STR

    FormatError(buf, bufSize, "CHIP", err, desc);

    return true;
}

} // namespace chip
