/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file provides declarations required by the CHIP ZCL Application
 *      layer.
 *
 */
#ifndef CHIP_ZCL_MASTER_HEADER
#define CHIP_ZCL_MASTER_HEADER

#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "chip-zcl-buffer.h"

typedef uint64_t bitmap64_t;
typedef uint8_t enum8_t;
typedef uint16_t enum16_t;
typedef uint32_t utc_time_t;

typedef uint8_t ChipZclStatus_t;

// from platform/base/hal/host/micro-common.h
// Since the STM32 host doesn't have a micro.h add these defines here.
// the number of ticks (as returned from halCommonGetInt32uMillisecondTick)
// that represent an actual second. This can vary on different platforms.
// It must be defined by the host system.
#ifndef MILLISECOND_TICKS_PER_SECOND
#define MILLISECOND_TICKS_PER_SECOND 1024UL
// See bug 10232
//  #error "MILLISECOND_TICKS_PER_SECOND is not defined in micro.h!"
#endif

#ifndef MILLISECOND_TICKS_PER_DECISECOND
#define MILLISECOND_TICKS_PER_DECISECOND (MILLISECOND_TICKS_PER_SECOND / 10)
#endif

#ifndef MILLISECOND_TICKS_PER_QUARTERSECOND
#define MILLISECOND_TICKS_PER_QUARTERSECOND (MILLISECOND_TICKS_PER_SECOND >> 2)
#endif

#ifndef MILLISECOND_TICKS_PER_MINUTE
#define MILLISECOND_TICKS_PER_MINUTE (60UL * MILLISECOND_TICKS_PER_SECOND)
#endif

#ifndef MILLISECOND_TICKS_PER_HOUR
#define MILLISECOND_TICKS_PER_HOUR (60UL * MILLISECOND_TICKS_PER_MINUTE)
#endif

#ifndef MILLISECOND_TICKS_PER_DAY
#define MILLISECOND_TICKS_PER_DAY (24UL * MILLISECOND_TICKS_PER_HOUR)
#endif

// From platform/base/hal/host/generic/compiler/platform-common.h
/**
 * @name  Bit Manipulation Macros
 */
//@{

/**
 * @brief Useful to reference a single bit of a byte.
 */
#define BIT(x) (1U << (x)) // Unsigned avoids compiler warnings re BIT(15)

/**
 * @brief Useful to reference a single bit of an uint32_t type.
 */
#define BIT32(x) (((uint32_t) 1) << (x))

/**
 * @brief Sets \c bit in the \c reg register or byte.
 * @note Assuming \c reg is an IO register, some platforms (such as the
 * AVR) can implement this in a single atomic operation.
 */
#define SETBIT(reg, bit) (reg) |= BIT(bit)

/**
 * @brief Sets the bits in the \c reg register or the byte
 * as specified in the bitmask \c bits.
 * @note This is never a single atomic operation.
 */
#define SETBITS(reg, bits) (reg) |= (bits)

/**
 * @brief Clears a bit in the \c reg register or byte.
 * @note Assuming \c reg is an IO register, some platforms (such as the AVR)
 * can implement this in a single atomic operation.
 */
#define CLEARBIT(reg, bit) (reg) &= ~(BIT(bit))

/**
 * @brief Clears the bits in the \c reg register or byte
 * as specified in the bitmask \c bits.
 * @note This is never a single atomic operation.
 */
#define CLEARBITS(reg, bits) (reg) &= ~(bits)

/**
 * @brief Returns the value of \c bit within the register or byte \c reg.
 */
#define READBIT(reg, bit) ((reg) & (BIT(bit)))

/**
 * @brief Returns the value of the bitmask \c bits within
 * the register or byte \c reg.
 */
#define READBITS(reg, bits) ((reg) & (bits))

//@} \\END Bit Manipulation Macros

// From 07-5123-05, section 2.5.3, table 2-10.
/** A success or failure status, used as a system-wide return type for functions. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclStatus_t
#else
typedef uint8_t ChipZclStatus_t;
enum
#endif
{
    /** The operation was successful. */
    CHIP_ZCL_STATUS_SUCCESS = 0x00,
    /** The operation was not successful. */
    CHIP_ZCL_STATUS_FAILURE = 0x01,
    /** The cluster is not supported. */
    CHIP_ZCL_STATUS_UNSUPPORTED_CLUSTER = 0xC3,
    /**
     * The sender is recognized but forbidden from carrying out this
     * command. */
    CHIP_ZCL_STATUS_FORBIDDEN = 0x7D,
    /**
     * The sender of the command does not have authorization to carry out this
     * command. */
    CHIP_ZCL_STATUS_NOT_AUTHORIZED = 0x7E,
    /** A reserved field/subfield/bit contains a non-zero value. */
    CHIP_ZCL_STATUS_RESERVED_FIELD_NOT_ZERO = 0x7F,
    /**
     * The command appears to contain the wrong fields, as detected either by the
     * presence of one or more invalid field entries or by missing
     * fields. Command not carried out. */
    CHIP_ZCL_STATUS_MALFORMED_COMMAND = 0x80,
    /**
     * The specified cluster command is not supported on the device. The command is not
     * carried out. */
    CHIP_ZCL_STATUS_UNSUP_CLUSTER_COMMAND = 0x81,
    /** The specified general ZCL command is not supported on the device. */
    CHIP_ZCL_STATUS_UNSUP_GENERAL_COMMAND = 0x82,
    /**
     * A manufacturer-specific unicast, cluster specific command was received with
     * an unknown manufacturer code, or the manufacturer code was recognized but
     * the command is not supported. */
    CHIP_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND = 0x83,
    /**
     * A manufacturer-specific unicast, ZCL specific command was received with an
     * unknown manufacturer code, or the manufacturer code was recognized but the
     * command is not supported. */
    CHIP_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND = 0x84,
    /**
     * At least one field of the command contains an incorrect value, according to
     * the specification the device is implemented to. */
    CHIP_ZCL_STATUS_INVALID_FIELD = 0x85,
    /** The specified attribute does not exist on the device. */
    CHIP_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE = 0x86,
    /**
     * An out of range error, or set to a reserved value. An attribute keeps its old
     * value. Note that an attribute value may be out of range if the attribute is
     * related to another, e.g., with minimum and maximum attributes. See the
     * individual attribute descriptions in ZCL specification for specific details. */
    CHIP_ZCL_STATUS_INVALID_VALUE = 0x87,
    /** Attempt to write a read only attribute. */
    CHIP_ZCL_STATUS_READ_ONLY = 0x88,
    /**
     * An operation (e.g., an attempt to create an entry in a table) failed due to
     * an insufficient amount of free space available. */
    CHIP_ZCL_STATUS_INSUFFICIENT_SPACE = 0x89,
    /**
     * An attempt to create an entry in a table failed due to a duplicate entry
     * already present in the table. */
    CHIP_ZCL_STATUS_DUPLICATE_EXISTS = 0x8A,
    /** The requested information (e.g., table entry) could not be found. */
    CHIP_ZCL_STATUS_NOT_FOUND = 0x8B,
    /** Periodic reports cannot be issued for this attribute. */
    CHIP_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE = 0x8C,
    /** The data type given for an attribute is incorrect. The command is not carried out. */
    CHIP_ZCL_STATUS_INVALID_DATA_TYPE = 0x8D,
    /** The selector for an attribute is incorrect. */
    CHIP_ZCL_STATUS_INVALID_SELECTOR = 0x8E,
    /**
     * A request has been made to read an attribute that the requestor is not
     * authorized to read. No action taken. */
    CHIP_ZCL_STATUS_WRITE_ONLY = 0x8F,
    /**
     * Setting the requested values puts the device in an inconsistent state
     * on startup. No action taken.*/
    CHIP_ZCL_STATUS_INCONSISTENT_STARTUP_STATE = 0x90,
    /**
     * An attempt has been made to write an attribute that is present but is
     * defined using an out-of-band method and not over the air. */
    CHIP_ZCL_STATUS_DEFINED_OUT_OF_BAND = 0x91,
    /** The supplied values (e.g., contents of table cells) are inconsistent. */
    CHIP_ZCL_STATUS_INCONSISTENT = 0x92,
    /**
     * The credentials presented by the device sending the command are not
     * sufficient to perform this action. */
    CHIP_ZCL_STATUS_ACTION_DENIED = 0x93,
    /** The exchange was aborted due to excessive response time. */
    CHIP_ZCL_STATUS_TIMEOUT = 0x94,
    /** Failed case when a client or a server decides to abort the upgrade process. */
    CHIP_ZCL_STATUS_ABORT = 0x95,
    /**
     * Invalid OTA upgrade image (ex. failed signature validation or signer
     * information check or CRC check). */
    CHIP_ZCL_STATUS_INVALID_IMAGE = 0x96,
    /** Server does not have the data block available yet. */
    CHIP_ZCL_STATUS_WAIT_FOR_DATA = 0x97,
    /** No OTA upgrade image available for a particular client. */
    CHIP_ZCL_STATUS_NO_IMAGE_AVAILABLE = 0x98,
    /**
     * The client still requires more OTA upgrade image files to
     * successfully upgrade. */
    CHIP_ZCL_STATUS_REQUIRE_MORE_IMAGE = 0x99,
    /** The command has been received and is being processed. */
    CHIP_ZCL_STATUS_NOTIFICATION_PENDING = 0x9A,
    /** An operation was unsuccessful due to a hardware failure. */
    CHIP_ZCL_STATUS_HARDWARE_FAILURE = 0xC0,
    /** An operation was unsuccessful due to a software failure. */
    CHIP_ZCL_STATUS_SOFTWARE_FAILURE = 0xC1,
    /** An error occurred during calibration. */
    CHIP_ZCL_STATUS_CALIBRATION_ERROR = 0xC2,
    /** Distinguished value that represents a null (invalid) status. */
    CHIP_ZCL_STATUS_NULL = 0xFF,
};

// -----------------------------------------------------------------------------
// Endpoints.

/** An endpoint identifier. */
typedef uint8_t ChipZclEndpointId_t;
/** A minimum endpoint identifer value. */
#define CHIP_ZCL_ENDPOINT_MIN 0x01
/** A maximum endpoint identifer value. */
#define CHIP_ZCL_ENDPOINT_MAX 0xF0
/** A distinguished value that represents a null (invalid) endpoint identifer. */
#define CHIP_ZCL_ENDPOINT_NULL ((ChipZclEndpointId_t) -1)

/** An endpoint index. */
typedef uint8_t ChipZclEndpointIndex_t;
/** A distinguished value that represents a null (invalid) endpoint index. */
#define CHIP_ZCL_ENDPOINT_INDEX_NULL ((ChipZclEndpointIndex_t) -1)

/** A device identifier. */
typedef uint16_t ChipZclDeviceId_t;
/** A distinguished value that represents a null (invalid) device identifer. */
#define CHIP_ZCL_DEVICE_ID_NULL ((ChipZclDeviceId_t) -1)

typedef struct
{
    ChipZclEndpointId_t endpointId;
    uint16_t clusterId;
    bool clusterSpecific;
    bool mfgSpecific;
    uint16_t mfgCode;
    uint8_t seqNum;
    uint8_t commandId;
    uint8_t payloadStartIndex;
    uint8_t direction;
    void * request;
    void * response;
} ChipZclCommandContext_t;

/** brief An identifier for a task */
typedef uint8_t ChipZclTaskId;

/**
 * @brief Either marks an event as inactive or specifies the units for the
 * event execution time.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclEventUnits
#else
typedef uint8_t ChipZclEventUnits;
enum
#endif
{
    /** The event is not scheduled to run. */
    CHIP_ZCL_EVENT_INACTIVE = 0,
    /** The execution time is in approximate milliseconds.  */
    CHIP_ZCL_EVENT_MS_TIME,
    /** The execution time is in 'binary' quarter seconds (256 approximate
        milliseconds each). */
    CHIP_ZCL_EVENT_QS_TIME,
    /** The execution time is in 'binary' minutes (65536 approximate milliseconds
        each). */
    CHIP_ZCL_EVENT_MINUTE_TIME,
    /** The event is scheduled to run at the earliest opportunity. */
    CHIP_ZCL_EVENT_ZERO_DELAY
};

//----------------------------------------------------------------
// Events and event queues.

// Forward declarations to make up for C's one-pass type checking.
struct Event_s;
struct EventQueue_s;

/** @brief The static part of an event.  Each event can be used with only one
 * event queue.
 */

typedef const struct EventActions_s
{
    struct EventQueue_s * queue;       // the queue this event goes on
    void (*handler)(struct Event_s *); // called when the event fires
    void (*marker)(struct Event_s *);  // marking fuction, can be NULL
    const char * name;                 // event name for debugging purposes
} EventActions;

typedef struct Event_s
{
    EventActions * actions; // static data

    // For internal use only, but the 'next' field must be initialized
    // to NULL.
    struct Event_s * next;
    uint32_t timeToExecute;
} Event;

/** @brief An event queue is currently a list of events ordered by
 * execution time.
 */
typedef struct EventQueue_s
{
    Event * isrEvents; // Events to be run with no delay, protected by ATOMIC.
    Event * events;    // Events scheduled to be run.
    uint32_t runTime;  // These two fields are used to avoid running one
    bool running;      // event multiple times at a single moment in time.
} EventQueue;

/** @brief Control structure for events.
 *
 * This structure should not be accessed directly.
 * It holds the event status (one of the @e CHIP_ZCL_EVENT_ values)
 * and the time left before the event fires.
 */
typedef struct
{
    /** The event's status, either inactive or the units for timeToExecute. */
    ChipZclEventUnits status;
    /** The ID of the task this event belongs to. */
    ChipZclTaskId taskid;
    /** How long before the event fires.
     *  Units are always in milliseconds.
     */
    uint32_t timeToExecute;
} ChipZclEventControl;

/** @brief Complete events with a control and a handler procedure.
 *
 * An application typically creates an array of events
 * along with their handlers.
 * The main loop passes the array to system ::runEvents() to call
 * the handlers of any events whose time has arrived.
 */
typedef struct
{
    /** The control structure for the event. */
    ChipZclEventControl * control;
    /** The procedure to call when the event fires. */
    void (*handler)(void);
} ChipZclEventData;

/** @brief Control structure for tasks.
 *
 * This structure should not be accessed directly.
 */
typedef struct
{
    // The time when the next event associated with this task will fire
    uint32_t nextEventTime;
    // The list of events associated with this task
    ChipZclEventData * events;
    // A flag that indicates the task has something to do other than events
    bool busy;
} ChipZclTaskControl;

/**
 * @brief Type for referring to zigbee application profile id
 */
typedef uint16_t ChipZclProfileId;

/**
 * @brief Type for referring to ZCL attribute id
 */
typedef uint16_t ChipZclAttributeId;

/**
 * @brief Type for referring to ZCL cluster id
 */
typedef uint16_t ChipZclClusterId;

/**
 * @brief Type for referring to ZCL attribute type
 */
typedef uint8_t ChipZclAttributeType;

/**
 * @brief Type for the cluster mask
 */
typedef uint8_t ChipZclClusterMask;

/**
 * @brief Type for the attribute mask
 */
typedef uint8_t ChipZclAttributeMask;

/**
 * @brief Generic function type, used for either of the cluster function.
 *
 * This type is used for the array of the cluster functions, and should
 * always be cast into one of the specific functions before being called.
 */
typedef void (*ChipZclGenericClusterFunction)(void);

/**
 * @brief A distinguished manufacturer code that is used to indicate the
 * absence of a manufacturer-specific profile, cluster, command, or attribute.
 */
#define CHIP_ZCL_NULL_MANUFACTURER_CODE 0x0000

/**
 * @brief An invalid profile ID
 * This is a reserved profileId.
 */
#define CHIP_ZCL_INVALID_PROFILE_ID 0xFFFF

/**
 * @brief Type for default values.
 *
 * Default value is either a value itself, if it is 2 bytes or less,
 * or a pointer to the value itself, if attribute type is longer than
 * 2 bytes.
 */
typedef union
{
    /**
     * Points to data if size is more than 2 bytes.
     * If size is more than 2 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    uint8_t * ptrToDefaultValue;
    /**
     * Actual default value if the attribute size is 2 bytes or less.
     */
    uint16_t defaultValue;
} ChipZclDefaultAttributeValue;

/**
 * @brief Type describing the attribute default, min and max values.
 *
 * This struct is required if the attribute mask specifies that this
 * attribute has a known min and max values.
 */
typedef struct
{
    /**
     * Default value of the attribute.
     */
    ChipZclDefaultAttributeValue defaultValue;
    /**
     * Minimum allowed value
     */
    ChipZclDefaultAttributeValue minValue;
    /**
     * Maximum allowed value.
     */
    ChipZclDefaultAttributeValue maxValue;
} ChipZclAttributeMinMaxValue;

/**
 * @brief Union describing the attribute default/min/max values.
 */
typedef union
{
    /**
     * Points to data if size is more than 2 bytes.
     * If size is more than 2 bytes, and this value is NULL,
     * then the default value is all zeroes.
     */
    uint8_t * ptrToDefaultValue;
    /**
     * Actual default value if the attribute size is 2 bytes or less.
     */
    uint16_t defaultValue;
    /**
     * Points to the min max attribute value structure, if min/max is
     * supported for this attribute.
     */
    ChipZclAttributeMinMaxValue * ptrToMinMaxValue;
} ChipZclDefaultOrMinMaxAttributeValue;

/**
 * @brief Each attribute has it's metadata stored in such struct.
 *
 * There is only one of these per attribute across all endpoints.
 */
typedef struct
{
    /**
     * Attribute ID, according to ZCL specs.
     */
    ChipZclAttributeId attributeId;
    /**
     * Attribute type, according to ZCL specs.
     */
    ChipZclAttributeType attributeType;
    /**
     * Size of this attribute in bytes.
     */
    uint8_t size;
    /**
     * Attribute mask, tagging attribute with specific
     * functionality. See ATTRIBUTE_MASK_ macros defined
     * in att-storage.h.
     */
    ChipZclAttributeMask mask;
    /**
     * Pointer to the default value union. Actual value stored
     * depends on the mask.
     */
    ChipZclDefaultOrMinMaxAttributeValue defaultValue;
} ChipZclAttributeMetadata;

/**
 * @brief Struct describing cluster
 */
typedef struct
{
    /**
     *  ID of cluster according to ZCL spec
     */
    ChipZclClusterId clusterId;
    /**
     * Pointer to attribute metadata array for this cluster.
     */
    ChipZclAttributeMetadata * attributes;
    /**
     * Total number of attributes
     */
    uint16_t attributeCount;
    /**
     * Total size of non-external, non-singleton attribute for this cluster.
     */
    uint16_t clusterSize;
    /**
     * Mask with additional functionality for cluster. See CLUSTER_MASK
     * macros.
     */
    ChipZclClusterMask mask;

    /**
     * An array into the cluster functions. The length of the array
     * is determined by the function bits in mask. This may be null
     * if this cluster has no functions.
     */
    const ChipZclGenericClusterFunction * functions;
} ChipZclCluster;

/**
 * @brief Struct used to find an attribute in storage. Together the elements
 * in this search record constitute the "primary key" used to identify a unique
 * attribute value in attribute storage.
 */
typedef struct
{
    /**
     * Endpoint that the attribute is located on
     */
    uint8_t endpoint;

    /**
     * Cluster that the attribute is located on. If the cluster
     * id is inside the manufacturer specific range, 0xfc00 - 0xffff,
     * The manufacturer code should also be set to the code associated
     * with the manufacturer specific cluster.
     */
    ChipZclClusterId clusterId;

    /**
     * Cluster mask for the cluster, used to determine if it is
     * the server or client version of the cluster. See CLUSTER_MASK_
     * macros defined in att-storage.h
     */
    ChipZclClusterMask clusterMask;

    /**
     * The two byte identifier for the attribute. If the cluster id is
     * inside the manufacturer specific range 0xfc00 - 0xffff, or the manufacturer
     * code is NOT 0, the attribute is assumed to be manufacturer specific.
     */
    ChipZclAttributeId attributeId;

    /**
     * Manufacturer Code associated with the cluster and or attribute.
     * If the cluster id is inside the manufacturer specific
     * range, this value should indicate the manufacturer code for the
     * manufacturer specific cluster. Otherwise if this value is non zero,
     * and the cluster id is a standard ZCL cluster,
     * it is assumed that the attribute being sought is a manufacturer specific
     * extension to the standard ZCL cluster indicated by the cluster id.
     */
    uint16_t manufacturerCode;
} ChipZclAttributeSearchRecord;

/**
 * A struct used to construct a table of manufacturer codes for
 * manufacturer specific attributes and clusters.
 */
typedef struct
{
    uint16_t index;
    uint16_t manufacturerCode;
} ChipZclManufacturerCodeEntry;

/**
 * This type is used to compare two ZCL attribute values. The size of this data
 * type depends on the platform.
 */
typedef uint32_t ChipZclDifferenceType;

/**
 * @brief Endpoint type struct describes clusters that are on the endpoint.
 */
typedef struct
{
    /**
     * Pointer to the cluster structs, describing clusters on this
     * endpoint type.
     */
    ChipZclCluster * cluster;
    /**
     * Number of clusters in this endpoint type.
     */
    uint8_t clusterCount;
    /**
     * Size of all non-external, non-singlet attribute in this endpoint type.
     */
    uint16_t endpointSize;
} ChipZclEndpointType;

#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclEndpointBitmask;
#else
typedef uint8_t ChipZclEndpointBitmask;
enum
#endif
{ CHIP_ZCL_ENDPOINT_DISABLED = 0x00,
  CHIP_ZCL_ENDPOINT_ENABLED  = 0x01,
};

/**
 * @brief Struct that maps actual endpoint type, onto a specific endpoint.
 */
typedef struct
{
    /**
     * Actual zigbee endpoint number.
     */
    uint8_t endpoint;
    /**
     * Profile ID of the device on this endpoint.
     */
    ChipZclProfileId profileId;
    /**
     * Device ID of the device on this endpoint.
     */
    uint16_t deviceId;
    /**
     * Version of the device.
     */
    uint8_t deviceVersion;
    /**
     * Endpoint type for this endpoint.
     */
    ChipZclEndpointType * endpointType;
    /**
     * Network index for this endpoint.
     */
    uint8_t networkIndex;
    /**
     * Meta-data about the endpoint
     */
    ChipZclEndpointBitmask bitmask;
} ChipZclDefinedEndpoint;

/**
 * @brief Type for referring to the tick callback for cluster.
 *
 * Tick function will be called once for each tick for each endpoint in
 * the cluster. The rate of tick is determined by the metadata of the
 * cluster.
 */
typedef void (*ChipZclTickFunction)(uint8_t endpoint);

/**
 * @brief Type for referring to the init callback for cluster.
 *
 * Init function is called when the application starts up, once for
 * each cluster/endpoint combination.
 */
typedef void (*ChipZclInitFunction)(uint8_t endpoint);

/**
 * @brief Type for referring to the attribute changed callback function.
 *
 * This function is called just after an attribute changes.
 */
typedef void (*ChipZclClusterAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId);

/**
 * @brief Type for referring to the manufacturer specific
 *        attribute changed callback function.
 *
 * This function is called just after a manufacturer specific attribute changes.
 */
typedef void (*ChipZclManufacturerSpecificClusterAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId,
                                                                           uint16_t manufacturerCode);

/**
 * @brief Type for referring to the pre-attribute changed callback function.
 *
 * This function is called before an attribute changes.
 */
typedef ChipZclStatus_t (*ChipZclClusterPreAttributeChangedCallback)(uint8_t endpoint, ChipZclAttributeId attributeId,
                                                                     ChipZclAttributeType attributeType, uint8_t size,
                                                                     uint8_t * value);

/**
 * @brief Type for referring to the default response callback function.
 *
 * This function is called when default response is received, before
 * the global callback. Global callback is called immediately afterwards.
 */
typedef void (*ChipZclDefaultResponseFunction)(uint8_t endpoint, uint8_t commandId, ChipZclStatus_t status);

/**
 * @brief A data struct used to keep track of incoming and outgoing
 *   commands for command discovery
 */
typedef struct
{
    uint16_t clusterId;
    uint8_t commandId;
    uint8_t mask;
} ChipZclCommandMetadata;

/**
 * @brief A data structure used to describe the time in a human
 * understandable format (as opposed to 32-bit UTC)
 */

typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
} ChipZclTimeStruct;

/**
 * @brief A data structure used to describe the ZCL Date data type
 */

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t dayOfMonth;
    uint8_t dayOfWeek;
} ChipZclDate;

/**
 * @brief Zigbee Internet Client/Server Remote Cluster Types
 */
typedef uint16_t ChipZclRemoteClusterType;

#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_NONE 0x0000
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_SERVER 0x0001
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_CLIENT 0x0002
#define CHIP_ZCL_REMOTE_CLUSTER_TYPE_INVALID 0xFFFF

/**
 * @brief Zigbee Internet Client/Server remote cluster struct.
 */
typedef struct
{
    ChipZclClusterId clusterId;
    ChipZclProfileId profileId;
    uint16_t deviceId;
    uint8_t endpoint;
    ChipZclRemoteClusterType type;
} ChipZclRemoteClusterStruct;

typedef struct
{
    ChipZclClusterId clusterId;
    bool server;
} ChipZclClusterInfo;

#if !defined(CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT)
#define CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT 3
#endif

/**
 * @brief A struct containing basic information about an endpoint.
 */
typedef struct
{
    ChipZclClusterInfo clusters[CHIP_ZCL_MAX_CLUSTERS_PER_ENDPOINT];
    ChipZclProfileId profileId;
    uint16_t deviceId;
    uint8_t endpoint;
    uint8_t clusterCount;
} ChipZclEndpointInfoStruct;

/**
 * Schedule 'event' to run after 'delay' milliseconds.  Delays greater than
 * MAX_EVENT_DELAY_MS will be reduced to MAX_EVENT_DELAY_MS.
 */
void chipZclEventSetDelayMs(Event * event, uint32_t delay);

void chEventControlSetDelayMS(ChipZclEventControl * event, uint32_t delay);
/** @brief Sets this ::EmberEventControl to run "delay" milliseconds in the future.
 *  NOTE: To avoid rollover errors in event calculation, the delay must be
 *  less than ::EMBER_MAX_EVENT_CONTROL_DELAY_MS.
 */
#define chipZclEventControlSetDelayMS(control, delay)                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        chEventControlSetDelayMS(&(control), (delay));                                                                             \
    } while (0)

/** @brief Sets this ::ChipEventControl as inactive (no pending event).
 */
#define chipZclEventControlSetInactive(control)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        (control).status = CHIP_ZCL_EVENT_INACTIVE;                                                                                \
    } while (0)

/** @brief Returns true if the event is active, false otherwise.
 */
#define chipZclEventControlGetActive(control) ((control).status != CHIP_ZCL_EVENT_INACTIVE)

/**
 * Returns the first scheduled event that has 'actions' and for which
 * 'predicate' returns true.  The returned event has been cancelled.
 */
#define chipZclFindEvent(queue, actions, predicate, data) (chipZclEventFind((queue), (actions), (predicate), (data), false))

/**
 * Returns the scheduled events that have 'actions' and for which
 * 'predicate' returns true.  If 'predicate' is NULL then all events
 * that have 'actions' are returned.  The returned events are linked
 * through the 'next' field and have been cancelled.
 *
 * WARNING: the 'next' field of the returned events must be set to
 * NULL before the event is passed to any event operation.
 */

#define chipZclFindAllEvents(queue, actions, predicate, data) (chipZclEventFind((queue), (actions), (predicate), (data), true))

typedef bool (*EventPredicate)(Event *, void *);

Event * chipZclEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all);

// Buffers
/**
 * @brief For use when declaring a Buffer.
 */
typedef uint16_t Buffer;
/**
 * @brief Denotes a null buffer.
 */
#define NULL_BUFFER 0x0000

Buffer chipZclReallyAllocateBuffer(uint16_t dataSizeInBytes, bool async);

#define chipZclAllocateBuffer(dataSizeInBytes) chipZclReallyAllocateBuffer(dataSizeInBytes, false)

// -----------------------------------------------------------------------------
// Roles.

/**
 * @addtogroup CHIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** Defines possible roles of a cluster. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclRole_t
#else
typedef uint8_t ChipZclRole_t;
enum
#endif
{
    /** Cluster is a client. */
    CHIP_ZCL_ROLE_CLIENT = 0,
    /** Cluster is a server. */
    CHIP_ZCL_ROLE_SERVER = 1,
};

/** @} end addtogroup */

// -----------------------------------------------------------------------------
// Manufacturer codes.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** A manufacturer code. */
typedef uint16_t ChipZclManufacturerCode_t;
/** A distinguished value that represents a null (invalid) manufacturer code. */
#define CHIP_ZCL_MANUFACTURER_CODE_NULL 0x0000

// -----------------------------------------------------------------------------
// Clusters.
/** A cluster identifier. */
typedef uint16_t ChipZclClusterId_t;
typedef uint8_t ChipZclRole_t;
typedef uint16_t ChipZclManufacturerCode_t;
typedef uint16_t ChipZclAttributeId_t;
/** This structure holds a cluster specification. */
typedef struct
{
    /** Role of a cluster. */
    ChipZclRole_t role;
    /** Manufacturer code of a cluster. */
    ChipZclManufacturerCode_t manufacturerCode;
    /** Identifier of a cluster. */
    ChipZclClusterId_t id;
} ChipZclClusterSpec_t;

int32_t chipZclCompareClusterSpec(const ChipZclClusterSpec_t * s1, const ChipZclClusterSpec_t * s2);
bool chipZclAreClusterSpecsEqual(const ChipZclClusterSpec_t * s1, const ChipZclClusterSpec_t * s2);
void chipZclReverseClusterSpec(const ChipZclClusterSpec_t * s1, ChipZclClusterSpec_t * s2);

void chipZclCorePrintln(const char * formatString, ...);

ChipZclStatus_t chipZclSendDefaultResponse(const ChipZclCommandContext_t * context, ChipZclStatus_t status);

// Attributes
void chipZclResetAttributes(ChipZclEndpointId_t endpointId);
void chipZclReportingConfigurationsFactoryReset(ChipZclEndpointId_t endpointId);
ChipZclStatus_t chipZclReadAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                     ChipZclAttributeId_t attributeId, void * buffer, size_t bufferLength);
ChipZclStatus_t chipZclWriteAttribute(ChipZclEndpointId_t endpointId, const ChipZclClusterSpec_t * clusterSpec,
                                      ChipZclAttributeId_t attributeId, const void * buffer, size_t bufferLength);

// Event Mechanism
Event * chipZclEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all);

void chipZclEventSetDelayMs(Event * event, uint32_t delay);

// Buffer Management Mechanism
uint8_t * chipZclGetBufferPointer(Buffer buffer);

Buffer chipZclReallyAllocateBuffer(uint16_t length, bool unused);

// Endpoint Management
ChipZclEndpointId_t chipZclEndpointIndexToId(ChipZclEndpointIndex_t index, const ChipZclClusterSpec_t * clusterSpec);

// Some platform CHIP_ZCL_STATUS_INSUFFICIENT_SPACE
#define MEMSET(d, v, l) memset(d, v, l)
#define MEMCOPY(d, s, l) memcpy(d, s, l)
#define MEMMOVE(d, s, l) memmove(d, s, l)
#define MEMCOMPARE(s0, s1, l) memcmp(s0, s1, l)
#define MEMPGMCOMPARE(s0, s1, l) memcmp(s0, s1, l)
#define LOW_BYTE(n) ((uint8_t)((n) &0xFF))
#define HIGH_BYTE(n) ((uint8_t)(LOW_BYTE((n) >> 8)))
#define IS_BIG_ENDIAN() false
/**
 * @brief Returns the value built from the two \c uint8_t
 * values \c high and \c low.
 */
#define HIGH_LOW_TO_INT(high, low) (((uint16_t)(((uint16_t)(high)) << 8)) + ((uint16_t)((low) &0xFF)))

#define CHIP_ZCL_INVALID_CLUSTER_ID 0xFFFF

#define CHIP_ZCL_INVALID_ENDPOINT 0xFF

#define CHIP_ZCL_INVALID_PAN_ID 0xFFFF

/**
 * @brief macro that returns true if the cluster is in the manufacturer specific range
 *
 * @param cluster EmberAfCluster* to consider
 */
#define chipZclClusterIsManufacturerSpecific(cluster) ((cluster)->clusterId >= 0xFC00)

/**
 * @brief macro that returns true if attribute is read only.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsReadOnly(metadata) (((metadata)->mask & ATTRIBUTE_MASK_WRITABLE) == 0)

/**
 * @brief macro that returns true if client attribute, and false if server.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsClient(metadata) (((metadata)->mask & ATTRIBUTE_MASK_CLIENT) != 0)

/**
 * @brief macro that returns true if attribute is saved to token.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsTokenized(metadata) (((metadata)->mask & ATTRIBUTE_MASK_TOKENIZE) != 0)

/**
 * @brief macro that returns true if attribute is saved in external storage.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsExternal(metadata) (((metadata)->mask & ATTRIBUTE_MASK_EXTERNAL_STORAGE) != 0)

/**
 * @brief macro that returns true if attribute is a singleton
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsSingleton(metadata) (((metadata)->mask & ATTRIBUTE_MASK_SINGLETON) != 0)

/**
 * @brief macro that returns true if attribute is manufacturer specific
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeIsManufacturerSpecific(metadata) (((metadata)->mask & ATTRIBUTE_MASK_MANUFACTURER_SPECIFIC) != 0)

/**
 * @brief macro that returns size of attribute in bytes.
 *
 * @param metadata EmberAfAttributeMetadata* to consider.
 */
#define chipZclAttributeSize(metadata) ((metadata)->size)

/**
 *@brief Returns true if type is signed, false otherwise.
 */
bool chipZclIsTypeSigned(ChipZclAttributeType dataType);

/**
 * @brief Simple integer comparison function.
 * Compares two values of a known length as integers.
 * Signed integer comparison are supported for numbers with length of
 * 4 (bytes) or less.
 * The integers are in native endianness.
 *
 * @return -1, if val1 is smaller
 *          0, if they are the same or if two negative numbers with length
 *          greater than 4 is being compared
 *          1, if val2 is smaller.
 */
int8_t chipZclCompareValues(uint8_t * val1, uint8_t * val2, uint8_t len, bool signedNumber);

/**
 * @brief Returns the total number of endpoints (dynamic and pre-compiled).
 */
uint8_t chipZclEndpointCount(void);

/*
 * @brief Function that determines the length of a zigbee Cluster Library string
 *   (where the first byte is assumed to be the length).
 */
uint8_t chipZclStringLength(const uint8_t * buffer);
/*
 * @brief Function that determines the length of a zigbee Cluster Library long string.
 *   (where the first two bytes are assumed to be the length).
 */
uint16_t chipZclLongStringLength(const uint8_t * buffer);

/*
 * @brief Function that returns a metadata structure for the given attribute.
 * *metadata is set to null if none is found,
 */
ChipZclStatus_t chipZclLocateAttributeMetadata(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId,
                                               uint8_t mask, uint16_t manufacturerCode, ChipZclAttributeMetadata ** metadata);

/*
 * @brief Initialization of the endpoint structure. Before this call, endpoints are not enabled. After this call endpoints
 * are enabled and linked together properly for use.
 */
void chipZclEndpointInit(void);

#endif // CHIP_ZCL_MASTER_HEADER
