/*
 *   Copyright (c) 2024 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package chip.platform;

public class ChipError {

    /**
     * SDK Core errors
     *
     * These errors are defined in src/lib/core/CHIPError.h
     */

    /**
     * This defines the CHIP error code for success or no error
     */
    public static final long CHIP_NO_ERROR = 0;

    /**
     * A message exceeds the sent limit
     */
    public static final long CHIP_ERROR_SENDING_BLOCKED = 1;

    /**
     * A connection has been aborted
     */
    public static final long CHIP_ERROR_CONNECTION_ABORTED = 2;

    /**
     * An unexpected state was encountered
     */
    public static final long CHIP_ERROR_INCORRECT_STATE = 3;

    /**
     * A message is too long
     */
    public static final long CHIP_ERROR_MESSAGE_TOO_LONG = 4;

    /**
     * Recursion depth overflow
     */
    public static final long CHIP_ERROR_RECURSION_DEPTH_LIMIT = 5;

    /**
     * The attempt to register an unsolicited message handler failed because the unsolicited message handler pool is full
     */
    public static final long CHIP_ERROR_TOO_MANY_UNSOLICITED_MESSAGE_HANDLERS = 6;

    /**
     * The attempt to unregister an unsolicited message handler failed because the target handler was not found in the unsolicited message handler pool
     */
    public static final long CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER = 7;

    /**
     * No callback has been registered for handling a connection
     */
    public static final long CHIP_ERROR_NO_CONNECTION_HANDLER = 8;

    /**
     * The number of peer nodes exceeds the maximum limit of a local node
     */
    public static final long CHIP_ERROR_TOO_MANY_PEER_NODES = 9;

    /**
     * For use locally to mark conditions such as value found or end of iteration
     */
    public static final long CHIP_ERROR_SENTINEL = 10;

    /**
     * The attempt to allocate a buffer or object failed due to a lack of memory
     */
    public static final long CHIP_ERROR_NO_MEMORY = 11;

    /**
     * No callback has been registered for handling a message
     */
    public static final long CHIP_ERROR_NO_MESSAGE_HANDLER = 12;

    /**
     * A message is incomplete
     */
    public static final long CHIP_ERROR_MESSAGE_INCOMPLETE = 13;

    /**
     * The data is not aligned
     */
    public static final long CHIP_ERROR_DATA_NOT_ALIGNED = 14;

    /**
     * The encryption key type is unknown
     */
    public static final long CHIP_ERROR_UNKNOWN_KEY_TYPE = 15;

    /**
     * The encryption key is not found
     */
    public static final long CHIP_ERROR_KEY_NOT_FOUND = 16;

    /**
     * The encryption type is incorrect for the specified key
     */
    public static final long CHIP_ERROR_WRONG_ENCRYPTION_TYPE = 17;

    /**
     * Invalid UTF8 string (contains some characters that are invalid)
     */
    public static final long CHIP_ERROR_INVALID_UTF8 = 18;

    /**
     * The integrity check in the message does not match the expected integrity check
     */
    public static final long CHIP_ERROR_INTEGRITY_CHECK_FAILED = 19;

    /**
     * Invalid signature
     */
    public static final long CHIP_ERROR_INVALID_SIGNATURE = 20;

    /**
     * Invalid TLV character string (eg null terminator)
     */
    public static final long CHIP_ERROR_INVALID_TLV_CHAR_STRING = 21;

    /**
     * Subscription timeout caused by LIT ICD device inactive mode
     */
    public static final long CHIP_ERROR_LIT_SUBSCRIBE_INACTIVE_TIMEOUT = 22;

    /**
     * A signature type is unsupported
     */
    public static final long CHIP_ERROR_UNSUPPORTED_SIGNATURE_TYPE = 23;

    /**
     * A message length is invalid
     */
    public static final long CHIP_ERROR_INVALID_MESSAGE_LENGTH = 24;

    /**
     * A buffer is too small
     */
    public static final long CHIP_ERROR_BUFFER_TOO_SMALL = 25;

    /**
     * A key id is duplicate
     */
    public static final long CHIP_ERROR_DUPLICATE_KEY_ID = 26;

    /**
     * A key type does not match the expected key type
     */
    public static final long CHIP_ERROR_WRONG_KEY_TYPE = 27;

    /**
     * A requested object is uninitialized
     */
    public static final long CHIP_ERROR_UNINITIALIZED = 28;

    /**
     * The IPK is invalid
     */
    public static final long CHIP_ERROR_INVALID_IPK = 29;

    /**
     * A string length is invalid
     */
    public static final long CHIP_ERROR_INVALID_STRING_LENGTH = 30;

    /**
     * A list length is invalid
     */
    public static final long CHIP_ERROR_INVALID_LIST_LENGTH = 31;

    /**
     * Device Attestation failed
     */
    public static final long CHIP_ERROR_FAILED_DEVICE_ATTESTATION = 32;

    /**
     * The end of a TLV encoding, or the end of a TLV container element has been reached
     */
    public static final long CHIP_ERROR_END_OF_TLV = 33;

    /**
     * The TLV encoding ended prematurely
     */
    public static final long CHIP_ERROR_TLV_UNDERRUN = 34;

    /**
     * A TLV element is invalid
     */
    public static final long CHIP_ERROR_INVALID_TLV_ELEMENT = 35;

    /**
     * A TLV tag is invalid
     */
    public static final long CHIP_ERROR_INVALID_TLV_TAG = 36;

    /**
     * An implicitly encoded TLV tag was encountered, but an implicit profile id has not been defined
     */
    public static final long CHIP_ERROR_UNKNOWN_IMPLICIT_TLV_TAG = 37;

    /**
     * A TLV type is wrong
     */
    public static final long CHIP_ERROR_WRONG_TLV_TYPE = 38;

    /**
     * A TLV container is unexpectedly open
     */
    public static final long CHIP_ERROR_TLV_CONTAINER_OPEN = 39;

    /**
     * A message type is invalid
     */
    public static final long CHIP_ERROR_INVALID_MESSAGE_TYPE = 42;

    /**
     * An unexpected TLV element was encountered
     */
    public static final long CHIP_ERROR_UNEXPECTED_TLV_ELEMENT = 43;

    /**
     * A requested function or feature is not implemented
     */
    public static final long CHIP_ERROR_NOT_IMPLEMENTED = 45;

    /**
     * An address is invalid
     */
    public static final long CHIP_ERROR_INVALID_ADDRESS = 46;

    /**
     * An argument is invalid
     */
    public static final long CHIP_ERROR_INVALID_ARGUMENT = 47;

    /**
     * A TLV path list is invalid
     */
    public static final long CHIP_ERROR_INVALID_PATH_LIST = 48;

    /**
     * A TLV data list is invalid
     */
    public static final long CHIP_ERROR_INVALID_DATA_LIST = 49;

    /**
     * A request timed out
     */
    public static final long CHIP_ERROR_TIMEOUT = 50;

    /**
     * A device descriptor is invalid
     */
    public static final long CHIP_ERROR_INVALID_DEVICE_DESCRIPTOR = 51;

    /**
     * A PASE parameter is invalid
     */
    public static final long CHIP_ERROR_INVALID_PASE_PARAMETER = 56;

    /**
     * A use of session key is invalid
     */
    public static final long CHIP_ERROR_INVALID_USE_OF_SESSION_KEY = 59;

    /**
     * A connection is closed unexpectedly
     */
    public static final long CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY = 60;

    /**
     * A TLV element is missing
     */
    public static final long CHIP_ERROR_MISSING_TLV_ELEMENT = 61;

    /**
     * Secure random data is not available
     */
    public static final long CHIP_ERROR_RANDOM_DATA_UNAVAILABLE = 62;

    /**
     * A host/port list is empty
     */
    public static final long CHIP_ERROR_HOST_PORT_LIST_EMPTY = 65;

    /**
     * A service manager is forced to reset
     */
    public static final long CHIP_ERROR_FORCED_RESET = 69;

    /**
     * No endpoint is available
     */
    public static final long CHIP_ERROR_NO_ENDPOINT = 70;

    /**
     * A destination node id is invalid
     */
    public static final long CHIP_ERROR_INVALID_DESTINATION_NODE_ID = 71;

    /**
     * The operation cannot be performed because the underlying object is not connected
     */
    public static final long CHIP_ERROR_NOT_CONNECTED = 72;

    /**
     * CA certificate is not found
     */
    public static final long CHIP_ERROR_CA_CERT_NOT_FOUND = 74;

    /**
     * A certificate path length exceeds the constraint
     */
    public static final long CHIP_ERROR_CERT_PATH_LEN_CONSTRAINT_EXCEEDED = 75;

    /**
     * A certificate path is too long
     */
    public static final long CHIP_ERROR_CERT_PATH_TOO_LONG = 76;

    /**
     * A requested certificate usage is not allowed
     */
    public static final long CHIP_ERROR_CERT_USAGE_NOT_ALLOWED = 77;

    /**
     * A certificate expired
     */
    public static final long CHIP_ERROR_CERT_EXPIRED = 78;

    /**
     * A certificate is not valid yet
     */
    public static final long CHIP_ERROR_CERT_NOT_VALID_YET = 79;

    /**
     * A certificate format is unsupported
     */
    public static final long CHIP_ERROR_UNSUPPORTED_CERT_FORMAT = 80;

    /**
     * An elliptic curve is unsupported
     */
    public static final long CHIP_ERROR_UNSUPPORTED_ELLIPTIC_CURVE = 81;

    /**
     * A certificate is not found
     */
    public static final long CHIP_ERROR_CERT_NOT_FOUND = 83;

    /**
     * A CASE parameter is invalid
     */
    public static final long CHIP_ERROR_INVALID_CASE_PARAMETER = 84;

    /**
     * A certificate load failed
     */
    public static final long CHIP_ERROR_CERT_LOAD_FAILED = 86;

    /**
     * A certificate is not trusted
     */
    public static final long CHIP_ERROR_CERT_NOT_TRUSTED = 87;

    /**
     * A certificate subject/issuer distinguished name is wrong
     */
    public static final long CHIP_ERROR_WRONG_CERT_DN = 89;

    /**
     * A node id is wrong
     */
    public static final long CHIP_ERROR_WRONG_NODE_ID = 92;

    /**
     * A retransmission table is already full
     */
    public static final long CHIP_ERROR_RETRANS_TABLE_FULL = 100;

    /**
     * A transaction is cancelled
     */
    public static final long CHIP_ERROR_TRANSACTION_CANCELED = 104;

    /**
     * A message was received as part of a subscription exchange that has a mis-matching subscription id
     */
    public static final long CHIP_ERROR_INVALID_SUBSCRIPTION = 107;

    /**
     * A CHIP feature is unsupported
     */
    public static final long CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE = 108;

    /**
     * An unsolicited message with the originator bit clear
     */
    public static final long CHIP_ERROR_UNSOLICITED_MSG_NO_ORIGINATOR = 112;

    /**
     * A fabric index is invalid
     */
    public static final long CHIP_ERROR_INVALID_FABRIC_INDEX = 113;

    /**
     * The attempt to allocate a connection object failed because too many connections exist
     */
    public static final long CHIP_ERROR_TOO_MANY_CONNECTIONS = 114;

    /**
     * The operation cancelled because a shut down was initiated
     */
    public static final long CHIP_ERROR_SHUT_DOWN = 115;

    /**
     * The operation has been cancelled, generally by calling a cancel/abort request
     */
    public static final long CHIP_ERROR_CANCELLED = 116;

    /**
     * A specified TLV tag was not found
     */
    public static final long CHIP_ERROR_TLV_TAG_NOT_FOUND = 118;

    /**
     * A secure session is needed to do work, but is missing/is not present
     */
    public static final long CHIP_ERROR_MISSING_SECURE_SESSION = 119;

    /**
     * The CaseAdminSubject field is not valid in AddNOC command
     */
    public static final long CHIP_ERROR_INVALID_ADMIN_SUBJECT = 120;

    /**
     * Required privilege was insufficient during an operation
     */
    public static final long CHIP_ERROR_INSUFFICIENT_PRIVILEGE = 121;

    /**
     * The message counter of the session is exhausted, the session should be closed
     */
    public static final long CHIP_ERROR_MESSAGE_COUNTER_EXHAUSTED = 125;

    /**
     * The fabric with the given fabric id and root public key already exists
     */
    public static final long CHIP_ERROR_FABRIC_EXISTS = 126;

    /**
     * The endpoint with the given endpoint id already exists
     */
    public static final long CHIP_ERROR_ENDPOINT_EXISTS = 127;

    /**
     * The wrong encryption type error received from a peer node
     */
    public static final long CHIP_ERROR_WRONG_ENCRYPTION_TYPE_FROM_PEER = 128;

    /**
     * A key id is invalid
     */
    public static final long CHIP_ERROR_INVALID_KEY_ID = 133;

    /**
     * Time has invalid value
     */
    public static final long CHIP_ERROR_INVALID_TIME = 134;

    /**
     * A mismatch in schema was encountered
     */
    public static final long CHIP_ERROR_SCHEMA_MISMATCH = 142;

    /**
     * An integer does not have the kind of value we expect
     */
    public static final long CHIP_ERROR_INVALID_INTEGER_VALUE = 143;

    /**
     * The request cannot be processed or fulfilled
     */
    public static final long CHIP_ERROR_BAD_REQUEST = 146;

    /**
     * The presented certificate was of the wrong type
     */
    public static final long CHIP_ERROR_WRONG_CERT_TYPE = 157;

    /**
     * Persisted storage memory read/write failure
     */
    public static final long CHIP_ERROR_PERSISTED_STORAGE_FAILED = 159;

    /**
     * The specific value is not found in the persisted storage
     */
    public static final long CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND = 160;

    /**
     * The fabric is deleted, and the corresponding IM resources are released
     */
    public static final long CHIP_ERROR_IM_FABRIC_DELETED = 161;

    /**
     * The operation is still in progress
     */
    public static final long CHIP_ERROR_IN_PROGRESS = 164;

    /**
     * The CHIP message is not granted access for further processing
     */
    public static final long CHIP_ERROR_ACCESS_DENIED = 165;

    /**
     * Unknown resource ID
     */
    public static final long CHIP_ERROR_UNKNOWN_RESOURCE_ID = 166;

    /**
     * The conditional update of a trait instance path has failed because the local changes are based on an obsolete version of the data
     */
    public static final long CHIP_ERROR_VERSION_MISMATCH = 167;

    /**
     * Event ID matching the criteria was found
     */
    public static final long CHIP_ERROR_EVENT_ID_FOUND = 171;

    /**
     * Internal error
     */
    public static final long CHIP_ERROR_INTERNAL = 172;

    /**
     * Open file failed
     */
    public static final long CHIP_ERROR_OPEN_FAILED = 173;

    /**
     * Read from file failed
     */
    public static final long CHIP_ERROR_READ_FAILED = 174;

    /**
     * Write to file failed
     */
    public static final long CHIP_ERROR_WRITE_FAILED = 175;

    /**
     * Decoding failed
     */
    public static final long CHIP_ERROR_DECODE_FAILED = 176;

    /**
     * The application is not authorized to do DNS_SD lookups
     */
    public static final long CHIP_ERROR_DNS_SD_UNAUTHORIZED = 179;

    /**
     * The registered service name has collision on the LAN
     */
    public static final long CHIP_ERROR_MDNS_COLLISION = 180;

    /**
     * The Attribute path IB is malformed: it does not contain the required path
     */
    public static final long CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB = 181;

    /**
     * The Event Path IB is malformed: it does not contain the required elements
     */
    public static final long CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB = 182;

    /**
     * The Command Data IB is malformed: it does not contain the required elements
     */
    public static final long CHIP_ERROR_IM_MALFORMED_COMMAND_DATA_IB = 185;

    /**
     * The Event Data IB is malformed: it does not contain the required elements
     */
    public static final long CHIP_ERROR_IM_MALFORMED_EVENT_DATA_IB = 186;

    /**
     * Caller is trying to add more invoke command paths than what the remote node reports supporting
     */
    public static final long CHIP_ERROR_MAXIMUM_PATHS_PER_INVOKE_EXCEEDED = 187;

    /**
     * Unable to find the peer node
     */
    public static final long CHIP_ERROR_PEER_NODE_NOT_FOUND = 188;

    /**
     * Error in Hardware security module Used for software fallback option
     */
    public static final long CHIP_ERROR_HSM = 189;

    /**
     * The system's real time clock is not synchronized to an accurate time source
     */
    public static final long CHIP_ERROR_REAL_TIME_NOT_SYNCED = 191;

    /**
     * An unexpected event was encountered
     */
    public static final long CHIP_ERROR_UNEXPECTED_EVENT = 192;

    /**
     * No endpoint pool entry is available
     */
    public static final long CHIP_ERROR_ENDPOINT_POOL_FULL = 193;

    /**
     * More inbound message data is pending than available buffer space available to copy it
     */
    public static final long CHIP_ERROR_INBOUND_MESSAGE_TOO_BIG = 194;

    /**
     * More outbound message data is pending than available buffer space available to copy it
     */
    public static final long CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG = 195;

    /**
     * The received message is a duplicate of a previously received message
     */
    public static final long CHIP_ERROR_DUPLICATE_MESSAGE_RECEIVED = 196;

    /**
     * The received public key doesn't match locally generated key
     */
    public static final long CHIP_ERROR_INVALID_PUBLIC_KEY = 197;

    /**
     * The fabric ID in ICA certificate doesn't match the one in NOC
     */
    public static final long CHIP_ERROR_FABRIC_MISMATCH_ON_ICA = 198;

    /**
     * The CASE session could not be established as peer's credentials do not have a common root of trust
     */
    public static final long CHIP_ERROR_NO_SHARED_TRUSTED_ROOT = 201;

    /**
     * The CASE session could not be established as peer's credentials do not have a common root of trust
     */
    public static final long CHIP_ERROR_IM_STATUS_CODE_RECEIVED = 202;

    /**
     * The Data Version Filter IB is malformed: it does not contain the required elements
     */
    public static final long CHIP_ERROR_IM_MALFORMED_DATA_VERSION_FILTER_IB = 215;

    /**
     * The item referenced in the function call was not found
     */
    public static final long CHIP_ERROR_NOT_FOUND = 216;

    /**
     * The file identifier, encoded in the first few bytes of a processed file, has unexpected value
     */
    public static final long CHIP_ERROR_INVALID_FILE_IDENTIFIER = 218;

    /**
     * The Resource is busy and cannot process the request Trying again might work
     */
    public static final long CHIP_ERROR_BUSY = 219;

    /**
     * The maximum retry limit has been exceeded
     */
    public static final long CHIP_ERROR_MAX_RETRY_EXCEEDED = 220;

    /**
     * The provider list has been exhausted
     */
    public static final long CHIP_ERROR_PROVIDER_LIST_EXHAUSTED = 221;

    /**
     * The scheme field contains an invalid prefix
     */
    public static final long CHIP_ERROR_INVALID_SCHEME_PREFIX = 223;

    /**
     * The URI separator is missing
     */
    public static final long CHIP_ERROR_MISSING_URI_SEPARATOR = 224;

    /**
     * Callback function or callable object is not set
     */
    public static final long CHIP_ERROR_HANDLER_NOT_SET = 225;

    /**
     * SDK Inet Layer errors
     *
     * These errors are defined in src/inet/InetError.h
     */

    /**
     * The Internet Protocol (IP) address type or scope does not match the expected type or scope
     */
    public static final long INET_ERROR_WRONG_ADDRESS_TYPE = 257;

    /**
     * A remote connection peer disconnected
     */
    public static final long INET_ERROR_PEER_DISCONNECTED = 258;

    /**
     * A requested host name could not be resolved to an address
     */
    public static final long INET_ERROR_HOST_NOT_FOUND = 265;

    /**
     * A name server returned a temporary failure indication; try again later
     */
    public static final long INET_ERROR_DNS_TRY_AGAIN = 266;

    /**
     * A name server returned an unrecoverable error
     */
    public static final long INET_ERROR_DNS_NO_RECOVERY = 267;

    /**
     * An incorrect or unexpected protocol type was encountered
     */
    public static final long INET_ERROR_WRONG_PROTOCOL_TYPE = 269;

    /**
     * An unknown interface identifier was encountered
     */
    public static final long INET_ERROR_UNKNOWN_INTERFACE = 270;

    /**
     * A requested address type, class, or scope cannot be found
     */
    public static final long INET_ERROR_ADDRESS_NOT_FOUND = 272;

    /**
     * A requested host name is too long
     */
    public static final long INET_ERROR_HOST_NAME_TOO_LONG = 273;

    /**
     * A requested host name and port is invalid
     */
    public static final long INET_ERROR_INVALID_HOST_NAME = 274;

    /**
     * A TCP connection timed out due to inactivity
     */
    public static final long INET_ERROR_IDLE_TIMEOUT = 277;

    /**
     * An IPv6 packet is invalid
     */
    public static final long INET_ERROR_INVALID_IPV6_PKT = 279;

    /**
     * Failure to initialize an interface
     */
    public static final long INET_ERROR_INTERFACE_INIT_FAILURE = 280;

    /**
     * TCP Connection timed out waiting for acknowledgment for transmitted packet
     */
    public static final long INET_ERROR_TCP_USER_TIMEOUT = 281;

    /**
     * TCP Connection timed out waiting for a successful connection or a report of an error
     */
    public static final long INET_ERROR_TCP_CONNECT_TIMEOUT = 282;

    /**
     * The supplied text-form IP address was not compatible with the requested IP address type
     */
    public static final long INET_ERROR_INCOMPATIBLE_IP_ADDRESS_TYPE = 283;

    /**
     * SDK Device Layer errors
     *
     * These errors are defined in src/include/platform/CHIPDeviceError.h
     */

    /**
     * The requested configuration value was not found
     */
    public static final long CHIP_DEVICE_ERROR_CONFIG_NOT_FOUND = 513;

    /**
     * The device has not been service provisioned
     */
    public static final long CHIP_DEVICE_ERROR_NOT_SERVICE_PROVISIONED = 514;

    /**
     * The software update was aborted by application
     */
    public static final long CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_ABORTED = 515;

    /**
     * The software update was ignored by application
     */
    public static final long CHIP_DEVICE_ERROR_SOFTWARE_UPDATE_IGNORED = 516;

    /**
     * ASN1 Layer errors
     *
     * These errors are defined in src/lib/asn1/ASN1Error.h
     */

    /**
     * An end of ASN1 container or stream condition occurred
     */
    public static final long ASN1_END = 768;

    /**
     * The ASN1 encoding ended prematurely
     */
    public static final long ASN1_ERROR_UNDERRUN = 769;

    /**
     * The encoding exceeds the available space required to write it
     */
    public static final long ASN1_ERROR_OVERFLOW = 770;

    /**
     * An unexpected or invalid state was encountered
     */
    public static final long ASN1_ERROR_INVALID_STATE = 771;

    /**
     * The maximum number of container reading contexts was exceeded
     */
    public static final long ASN1_ERROR_MAX_DEPTH_EXCEEDED = 772;

    /**
     * The ASN1 encoding is invalid
     */
    public static final long ASN1_ERROR_INVALID_ENCODING = 773;

    /**
     * An unsupported encoding was requested or encountered
     */
    public static final long ASN1_ERROR_UNSUPPORTED_ENCODING = 774;

    /**
     * An encoded tag exceeds the available or allowed space required for it
     */
    public static final long ASN1_ERROR_TAG_OVERFLOW = 775;

    /**
     * An encoded length exceeds the available or allowed space required for it
     */
    public static final long ASN1_ERROR_LENGTH_OVERFLOW = 776;

    /**
     * An encoded value exceeds the available or allowed space required for it
     */
    public static final long ASN1_ERROR_VALUE_OVERFLOW = 777;

    /**
     * A requested object identifier does not match the list of supported object identifiers
     */
    public static final long ASN1_ERROR_UNKNOWN_OBJECT_ID = 778;

    /**
     * BLE Layer errors
     *
     * These errors are defined in src/ble/BleError.h
     */

    /**
     * Bluetooth LE adapter is (currently) unavailable
     */
    public static final long BLE_ERROR_ADAPTER_UNAVAILABLE = 1025;

    /**
     * No callback was registered to receive a BLE Transport Protocol (BTP) connection
     */
    public static final long BLE_ERROR_NO_CONNECTION_RECEIVED_CALLBACK = 1027;

    /**
     * A BLE central device unsubscribed from a peripheral device's BLE Transport Protocol (BTP) transmit characteristic
     */
    public static final long BLE_ERROR_CENTRAL_UNSUBSCRIBED = 1028;

    /**
     * A BLE central device failed to subscribe to a peripheral device's BLE Transport Protocol (BTP) transmit characteristic
     */
    public static final long BLE_ERROR_GATT_SUBSCRIBE_FAILED = 1029;

    /**
     * A BLE central device failed to unsubscribe from a peripheral device's BLE Transport Protocol (BTP) transmit characteristic
     */
    public static final long BLE_ERROR_GATT_UNSUBSCRIBE_FAILED = 1030;

    /**
     * A General Attribute Profile (GATT) write operation failed
     */
    public static final long BLE_ERROR_GATT_WRITE_FAILED = 1031;

    /**
     * A General Attribute Profile (GATT) indicate operation failed
     */
    public static final long BLE_ERROR_GATT_INDICATE_FAILED = 1032;

    /**
     * A BLE Transport Protocol (BTP) error was encountered
     */
    public static final long BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT = 1035;

    /**
     * A remote BLE connection peer disconnected, either actively or due to the expiration of a BLE connection supervision timeout
     */
    public static final long BLE_ERROR_REMOTE_DEVICE_DISCONNECTED = 1036;

    /**
     * The local application closed a BLE connection, and has informed BleLayer
     */
    public static final long BLE_ERROR_APP_CLOSED_CONNECTION = 1037;

    /**
     * A BLE peripheral device did not expose the General Attribute Profile (GATT) service required by the Bluetooth Transport Protocol (BTP)
     */
    public static final long BLE_ERROR_NOT_CHIP_DEVICE = 1039;

    /**
     * A remote device does not offer a compatible version of the Bluetooth Transport Protocol (BTP)
     */
    public static final long BLE_ERROR_INCOMPATIBLE_PROTOCOL_VERSIONS = 1040;

    /**
     * A remote device selected in invalid Bluetooth Transport Protocol (BTP) fragment size
     */
    public static final long BLE_ERROR_INVALID_FRAGMENT_SIZE = 1043;

    /**
     * A timer failed to start within BleLayer
     */
    public static final long BLE_ERROR_START_TIMER_FAILED = 1044;

    /**
     * A remote BLE peripheral device's Bluetooth Transport Protocol (BTP) connect handshake response timed out
     */
    public static final long BLE_ERROR_CONNECT_TIMED_OUT = 1045;

    /**
     * A remote BLE central device's Bluetooth Transport Protocol (BTP) connect handshake timed out
     */
    public static final long BLE_ERROR_RECEIVE_TIMED_OUT = 1046;

    /**
     * An invalid Bluetooth Transport Protocol (BTP) message was received
     */
    public static final long BLE_ERROR_INVALID_MESSAGE = 1047;

    /**
     * Receipt of an expected Bluetooth Transport Protocol (BTP) fragment acknowledgement timed out
     */
    public static final long BLE_ERROR_FRAGMENT_ACK_TIMED_OUT = 1048;

    /**
     * Receipt of an expected Bluetooth Transport Protocol (BTP) keep-alive fragment timed out
     */
    public static final long BLE_ERROR_KEEP_ALIVE_TIMED_OUT = 1049;

    /**
     * No callback was registered to handle Bluetooth Transport Protocol (BTP) connect completion
     */
    public static final long BLE_ERROR_NO_CONNECT_COMPLETE_CALLBACK = 1050;

    /**
     * A Bluetooth Transport Protcol (BTP) fragment acknowledgement was invalid
     */
    public static final long BLE_ERROR_INVALID_ACK = 1051;

    /**
     * A Bluetooth Transport Protocol (BTP) end-of-message fragment was received, but the total size of the received fragments is less than the indicated size of the original fragmented message
     */
    public static final long BLE_ERROR_REASSEMBLER_MISSING_DATA = 1052;

    /**
     * A set of Bluetooth Transport Protocol (BTP) header flags is invalid
     */
    public static final long BLE_ERROR_INVALID_BTP_HEADER_FLAGS = 1053;

    /**
     * A Bluetooth Transport Protocol (BTP) fragment sequence number is invalid
     */
    public static final long BLE_ERROR_INVALID_BTP_SEQUENCE_NUMBER = 1054;

    /**
     * The Bluetooth Transport Protocol (BTP) message reassembly engine encountered an unexpected state
     */
    public static final long BLE_ERROR_REASSEMBLER_INCORRECT_STATE = 1055;

    /**
     * IM Global errors errors
     *
     * These errors are defined in src/protocols/interaction_model/StatusCodeList.h
     */

    public static final long SUCCESS = 1280;

    public static final long FAILURE = 1281;

    public static final long INVALID_SUBSCRIPTION = 1405;

    public static final long UNSUPPORTED_ACCESS = 1406;

    public static final long UNSUPPORTED_ENDPOINT = 1407;

    public static final long INVALID_ACTION = 1408;

    public static final long UNSUPPORTED_COMMAND = 1409;

    public static final long INVALID_COMMAND = 1413;

    public static final long UNSUPPORTED_ATTRIBUTE = 1414;

    public static final long CONSTRAINT_ERROR = 1415;

    public static final long UNSUPPORTED_WRITE = 1416;

    public static final long RESOURCE_EXHAUSTED = 1417;

    public static final long NOT_FOUND = 1419;

    public static final long UNREPORTABLE_ATTRIBUTE = 1420;

    public static final long INVALID_DATA_TYPE = 1421;

    public static final long UNSUPPORTED_READ = 1423;

    public static final long DATA_VERSION_MISMATCH = 1426;

    public static final long TIMEOUT = 1428;

    public static final long BUSY = 1436;

    public static final long UNSUPPORTED_CLUSTER = 1475;

    public static final long NO_UPSTREAM_SUBSCRIPTION = 1477;

    public static final long NEEDS_TIMED_INTERACTION = 1478;

    public static final long UNSUPPORTED_EVENT = 1479;

    public static final long PATHS_EXHAUSTED = 1480;

    public static final long TIMED_REQUEST_MISMATCH = 1481;

    public static final long FAILSAFE_REQUIRED = 1482;

    public static final long INVALID_IN_STATE = 1483;

    public static final long NO_COMMAND_RESPONSE = 1484;

    public static final long WRITE_IGNORED = 1520;

    /**
     * Returns a string representation of the given error code.
     */
    public static native String toString(long errorCode);
}
