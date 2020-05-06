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
#include <stdbool.h>
#include <stdint.h>

typedef bool boolean; /*To ease adoption of bool instead of boolean.*/
typedef unsigned char int8u;
typedef signed char int8s;
typedef unsigned short int int16u;
typedef signed short int int16s;
typedef unsigned long int int32u;
typedef signed long int int32s;

typedef uint64_t bitmap64_t;
typedef uint8_t enum8_t;
typedef uint16_t enum16_t;
typedef uint32_t utc_time_t;
typedef unsigned long int size_t;

typedef uint8_t ZapStatus_t;

#define TRUE true

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
enum ZapStatus_t
#else
typedef uint8_t ZapStatus_t;
enum
#endif
{
    /** The operation was successful. */
    ZAP_STATUS_SUCCESS = 0x00,
    /** The operation was not successful. */
    ZAP_STATUS_FAILURE = 0x01,
    /**
     * The sender is recognized but forbidden from carrying out this
     * command. */
    ZAP_STATUS_FORBIDDEN = 0x7D,
    /**
     * The sender of the command does not have authorization to carry out this
     * command. */
    ZAP_STATUS_NOT_AUTHORIZED = 0x7E,
    /** A reserved field/subfield/bit contains a non-zero value. */
    ZAP_STATUS_RESERVED_FIELD_NOT_ZERO = 0x7F,
    /**
     * The command appears to contain the wrong fields, as detected either by the
     * presence of one or more invalid field entries or by missing
     * fields. Command not carried out. */
    ZAP_STATUS_MALFORMED_COMMAND = 0x80,
    /**
     * The specified cluster command is not supported on the device. The command is not
     * carried out. */
    ZAP_STATUS_UNSUP_CLUSTER_COMMAND = 0x81,
    /** The specified general ZCL command is not supported on the device. */
    ZAP_STATUS_UNSUP_GENERAL_COMMAND = 0x82,
    /**
     * A manufacturer-specific unicast, cluster specific command was received with
     * an unknown manufacturer code, or the manufacturer code was recognized but
     * the command is not supported. */
    ZAP_STATUS_UNSUP_MANUF_CLUSTER_COMMAND = 0x83,
    /**
     * A manufacturer-specific unicast, ZCL specific command was received with an
     * unknown manufacturer code, or the manufacturer code was recognized but the
     * command is not supported. */
    ZAP_STATUS_UNSUP_MANUF_GENERAL_COMMAND = 0x84,
    /**
     * At least one field of the command contains an incorrect value, according to
     * the specification the device is implemented to. */
    ZAP_STATUS_INVALID_FIELD = 0x85,
    /** The specified attribute does not exist on the device. */
    ZAP_STATUS_UNSUPPORTED_ATTRIBUTE = 0x86,
    /**
     * An out of range error, or set to a reserved value. An attribute keeps its old
     * value. Note that an attribute value may be out of range if the attribute is
     * related to another, e.g., with minimum and maximum attributes. See the
     * individual attribute descriptions in ZCL specification for specific details. */
    ZAP_STATUS_INVALID_VALUE = 0x87,
    /** Attempt to write a read only attribute. */
    ZAP_STATUS_READ_ONLY = 0x88,
    /**
     * An operation (e.g., an attempt to create an entry in a table) failed due to
     * an insufficient amount of free space available. */
    ZAP_STATUS_INSUFFICIENT_SPACE = 0x89,
    /**
     * An attempt to create an entry in a table failed due to a duplicate entry
     * already present in the table. */
    ZAP_STATUS_DUPLICATE_EXISTS = 0x8A,
    /** The requested information (e.g., table entry) could not be found. */
    ZAP_STATUS_NOT_FOUND = 0x8B,
    /** Periodic reports cannot be issued for this attribute. */
    ZAP_STATUS_UNREPORTABLE_ATTRIBUTE = 0x8C,
    /** The data type given for an attribute is incorrect. The command is not carried out. */
    ZAP_STATUS_INVALID_DATA_TYPE = 0x8D,
    /** The selector for an attribute is incorrect. */
    ZAP_STATUS_INVALID_SELECTOR = 0x8E,
    /**
     * A request has been made to read an attribute that the requestor is not
     * authorized to read. No action taken. */
    ZAP_STATUS_WRITE_ONLY = 0x8F,
    /**
     * Setting the requested values puts the device in an inconsistent state
     * on startup. No action taken.*/
    ZAP_STATUS_INCONSISTENT_STARTUP_STATE = 0x90,
    /**
     * An attempt has been made to write an attribute that is present but is
     * defined using an out-of-band method and not over the air. */
    ZAP_STATUS_DEFINED_OUT_OF_BAND = 0x91,
    /** The supplied values (e.g., contents of table cells) are inconsistent. */
    ZAP_STATUS_INCONSISTENT = 0x92,
    /**
     * The credentials presented by the device sending the command are not
     * sufficient to perform this action. */
    ZAP_STATUS_ACTION_DENIED = 0x93,
    /** The exchange was aborted due to excessive response time. */
    ZAP_STATUS_TIMEOUT = 0x94,
    /** Failed case when a client or a server decides to abort the upgrade process. */
    ZAP_STATUS_ABORT = 0x95,
    /**
     * Invalid OTA upgrade image (ex. failed signature validation or signer
     * information check or CRC check). */
    ZAP_STATUS_INVALID_IMAGE = 0x96,
    /** Server does not have the data block available yet. */
    ZAP_STATUS_WAIT_FOR_DATA = 0x97,
    /** No OTA upgrade image available for a particular client. */
    ZAP_STATUS_NO_IMAGE_AVAILABLE = 0x98,
    /**
     * The client still requires more OTA upgrade image files to
     * successfully upgrade. */
    ZAP_STATUS_REQUIRE_MORE_IMAGE = 0x99,
    /** The command has been received and is being processed. */
    ZAP_STATUS_NOTIFICATION_PENDING = 0x9A,
    /** An operation was unsuccessful due to a hardware failure. */
    ZAP_STATUS_HARDWARE_FAILURE = 0xC0,
    /** An operation was unsuccessful due to a software failure. */
    ZAP_STATUS_SOFTWARE_FAILURE = 0xC1,
    /** An error occurred during calibration. */
    ZAP_STATUS_CALIBRATION_ERROR = 0xC2,
    /** Distinguished value that represents a null (invalid) status. */
    ZAP_STATUS_NULL = 0xFF,
};

/** An endpoint identifier. */
typedef uint8_t ZapEndpointId_t;

typedef struct
{
    ZapEndpointId_t endpointId;
} ZapCommandContext_t;

/** brief An identifier for a task */
typedef uint8_t ZapTaskId;

/**
 * @brief Either marks an event as inactive or specifies the units for the
 * event execution time.
 */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ZapEventUnits
#else
typedef uint8_t ZapEventUnits;
enum
#endif
{
    /** The event is not scheduled to run. */
    ZAP_EVENT_INACTIVE = 0,
    /** The execution time is in approximate milliseconds.  */
    ZAP_EVENT_MS_TIME,
    /** The execution time is in 'binary' quarter seconds (256 approximate
        milliseconds each). */
    ZAP_EVENT_QS_TIME,
    /** The execution time is in 'binary' minutes (65536 approximate milliseconds
        each). */
    ZAP_EVENT_MINUTE_TIME,
    /** The event is scheduled to run at the earliest opportunity. */
    ZAP_EVENT_ZERO_DELAY
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
 * It holds the event status (one of the @e ZAP_EVENT_ values)
 * and the time left before the event fires.
 */
typedef struct
{
    /** The event's status, either inactive or the units for timeToExecute. */
    ZapEventUnits status;
    /** The ID of the task this event belongs to. */
    ZapTaskId taskid;
    /** How long before the event fires.
     *  Units are always in milliseconds.
     */
    uint32_t timeToExecute;
} ZapEventControl;

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
    ZapEventControl * control;
    /** The procedure to call when the event fires. */
    void (*handler)(void);
} ZapEventData;

/** @brief Control structure for tasks.
 *
 * This structure should not be accessed directly.
 */
typedef struct
{
    // The time when the next event associated with this task will fire
    uint32_t nextEventTime;
    // The list of events associated with this task
    ZapEventData * events;
    // A flag that indicates the task has something to do other than events
    bool busy;
} ZapTaskControl;

/**
 * Schedule 'event' to run after 'delay' milliseconds.  Delays greater than
 * MAX_EVENT_DELAY_MS will be reduced to MAX_EVENT_DELAY_MS.
 */
void zapEventSetDelayMs(Event * event, uint32_t delay);

/**
 * Returns the first scheduled event that has 'actions' and for which
 * 'predicate' returns true.  The returned event has been cancelled.
 */
#define zapFindEvent(queue, actions, predicate, data) (zapEventFind((queue), (actions), (predicate), (data), false))

/**
 * Returns the scheduled events that have 'actions' and for which
 * 'predicate' returns true.  If 'predicate' is NULL then all events
 * that have 'actions' are returned.  The returned events are linked
 * through the 'next' field and have been cancelled.
 *
 * WARNING: the 'next' field of the returned events must be set to
 * NULL before the event is passed to any event operation.
 */

#define zapFindAllEvents(queue, actions, predicate, data) (zapEventFind((queue), (actions), (predicate), (data), true))

typedef bool (*EventPredicate)(Event *, void *);

Event * zapEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all);

// Buffers
/**
 * @brief For use when declaring a Buffer.
 */
typedef uint16_t Buffer;
/**
 * @brief Denotes a null buffer.
 */
#define NULL_BUFFER 0x0000

Buffer zapReallyAllocateBuffer(uint16_t dataSizeInBytes, bool async);

#define zapAllocateBuffer(dataSizeInBytes) zapReallyAllocateBuffer(dataSizeInBytes, false)

// -----------------------------------------------------------------------------
// Roles.

/**
 * @addtogroup ZCLIP_clusters Clusters
 *
 * See zcl-core-types.h for source code.
 * @{
 */

/** Defines possible roles of a cluster. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ZapRole_t
#else
typedef uint8_t ZapRole_t;
enum
#endif
{
    /** Cluster is a client. */
    ZAP_ROLE_CLIENT = 0,
    /** Cluster is a server. */
    ZAP_ROLE_SERVER = 1,
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
typedef uint16_t ZapManufacturerCode_t;
/** A distinguished value that represents a null (invalid) manufacturer code. */
#define ZAP_MANUFACTURER_CODE_NULL 0x0000

// -----------------------------------------------------------------------------
// Clusters.
/** A cluster identifier. */
typedef uint16_t ZapClusterId_t;
typedef uint8_t ZapRole_t;
typedef uint16_t ZapManufacturerCode_t;
typedef uint16_t ZapAttributeId_t;
/** This structure holds a cluster specification. */
typedef struct
{
    /** Role of a cluster. */
    ZapRole_t role;
    /** Manufacturer code of a cluster. */
    ZapManufacturerCode_t manufacturerCode;
    /** Identifier of a cluster. */
    ZapClusterId_t id;
} ZapClusterSpec_t;

#define zapCorePrintln(...) // TODO make this return the string to be printed

ZapStatus_t zapSendDefaultResponse(const ZapCommandContext_t * context, ZapStatus_t status);

// Attributes
void zapResetAttributes(ZapEndpointId_t endpointId);
void zapReportingConfigurationsFactoryReset(ZapEndpointId_t endpointId);
ZapStatus_t zapReadAttribute(ZapEndpointId_t endpointId, const ZapClusterSpec_t * clusterSpec, ZapAttributeId_t attributeId,
                             void * buffer, size_t bufferLength);
ZapStatus_t zapWriteAttribute(ZapEndpointId_t endpointId, const ZapClusterSpec_t * clusterSpec, ZapAttributeId_t attributeId,
                              const void * buffer, size_t bufferLength);

// Event Mechanism
Event * zapEventFind(EventQueue * queue, EventActions * actions, EventPredicate predicate, void * data, bool all);

void zapEventSetDelayMs(Event * event, uint32_t delay);

// Buffer Management Mechanism
uint8_t * zapGetBufferPointer(Buffer buffer);

Buffer zapReallyAllocateBuffer(uint16_t length, bool unused);
