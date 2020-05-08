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

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t bitmap64_t;
typedef uint8_t enum8_t;
typedef uint16_t enum16_t;
typedef uint32_t utc_time_t;
typedef unsigned long int size_t;

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
    uint8_t * buffer;
    uint16_t bufLen;
    uint16_t clusterId;
    bool clusterSpecific;
    bool mfgSpecific;
    uint16_t mfgCode;
    uint8_t seqNum;
    uint8_t commandId;
    uint8_t payloadStartIndex;
    uint8_t direction;
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

#endif // CHIP_ZCL_MASTER_HEADER
