/**
 *    @file
 *      Defines the StatusReport class and General Codes for StatusReport messages.
 */

#pragma once

#include <support/BufferWriter.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Protocols {
namespace SecureChannel {

// Placeholder value for the ProtocolCode field when the GeneralCode is Success or Continue.
constexpr uint16_t kProtocolCodeSuccess = 0x0000;

// Placeholder value for the ProtocolCode field when there is no additional protocol-specific code to provide more information.
constexpr uint16_t kProtocolCodeGeneralFailure = 0xFFFF;

/**
 * Secure Channel Protocol Status Codes
 */
enum class GeneralStatusCode
{
    Success           = 0,  /**< Operation completed successfully. */
    Failure           = 1,  /**< Generic failure, additional details may be included in the protocol specific status. */
    BadPrecondition   = 2,  /**< Operation was rejected by the system because the system is in an invalid state. */
    OutOfRange        = 3,  /**< A value was out of a required range. */
    BadRequest        = 4,  /**< A request was unrecognized or malformed. */
    Unsupported       = 5,  /**< An unrecognized or unsupported request was received. */
    Unexpected        = 6,  /**< A request was not expected at this time. */
    ResourceExhausted = 7,  /**< Insufficient resources to process the given request. */
    Busy              = 8,  /**< Device is busy and cannot handle this request at this time. */
    Timeout           = 9,  /**< A timeout occurred. */
    Continue          = 10, /**< Context-specific signal to proceed. */
    Aborted           = 11, /**< Failure, often due to a concurrency error. */
    InvalidArgument   = 12, /**< An invalid/unsupported argument was provided. */
    NotFound          = 13, /**< Some requested entity was not found. */
    AlreadyExists     = 14, /**< The caller attempted to create something that already exists. */
    PermissionDenied  = 15, /**< Caller does not have sufficient permissions to execute the requested operations. */
    DataLoss          = 16, /**< Unrecoverable data loss or corruption has occurred. */
};

/**
 *  Encapsulates the data included in a StatusReport message, and provides methods for writing to and reading from PacketBuffers
 *  that contain StatusReport messages.
 */
class DLL_EXPORT StatusReport
{
public:
    /**
     *  Construct a StatusReport with zero-d out fields (for use before calling \c Parse() ).
     */
    StatusReport();

    /**
     *  Construct a StatusReport with no additional ProtocolData.
     *
     *  @param generalCode Must specify a GeneralCode (see \c GeneralStatusCode )
     *  @param protocolId Must specify a ProtocolId which consists of Vendor Id (upper 16 bits) and ProtocolId (lower 16 bits)
     *  @param protocolCode A code defined by the specified protocol which provides more information about the status
     */
    StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode);

    //
    /**
     *  Construct a StatusReport with additional ProtocolData.
     *
     *  WARNING: The caller MUST ensure that the data referenced by protocolData not be freed before calling \c WriteToBuffer()
     *      This function will not make a copy of the data.
     *
     *  @param generalCode Must specify a GeneralCode (see \c GeneralStatusCode )
     *  @param protocolId Must specify a ProtocolId which consists of Vendor Id (upper 16 bits) and ProtocolId (lower 16 bits)
     *  @param protocolCode A code defined by the specified protocol which provides more information about the status
     *  @param protocolData A pointer to additional (and optional) protocol-specific data
     *  @param dataLen The length of the additional (and optional) data
     */
    StatusReport(uint16_t generalCode, uint32_t protocolId, uint16_t protocolCode, uint8_t * protocolData, uint16_t dataLen);

    /**
     *  Read the contents of a \c PacketBuffer containing a StatusReport message and store the field values in this object.
     *
     *  @note This method assumes that the Header of the message has already been consumed, and that \c PacketBuffer::Start() points
     * to the beginning of the StatusReport data.
     *
     *  @param[in] buf A \c PacketBufferHandle containing the StatusReport message. This method will take ownership.
     *
     *  @return CHIP_ERROR Return an error if the message is malformed or buf is \c NULL
     */
    CHIP_ERROR Parse(System::PacketBufferHandle buf);

    /**
     *  Write the StatusReport contents into a buffer using a \c BufferWriter
     *
     *  @param[out] buf A \c BufferWriter which contains the buffer that will store the message fields.
     *
     *  @return BufferWriter Return a reference to the \c BufferWriter
     */
    Encoding::LittleEndian::BufferWriter & WriteToBuffer(Encoding::LittleEndian::BufferWriter & buf) const;

    /**
     *  Returns the minimum size of the buffer needed to write the message.
     */
    size_t Size() const;

    uint16_t GetGeneralCode() const { return mGeneralCode; }
    uint32_t GetProtocolId() const { return mProtocolId; }
    uint16_t GetProtocolCode() const { return mProtocolCode; }
    void GetProtocolData(uint8_t ** data, uint16_t & length) const
    {
        if (data == nullptr)
            return;
        *data  = mProtocolData;
        length = mProtocolDataLength;
    }

private:
    uint16_t mGeneralCode  = 0;
    uint32_t mProtocolId   = 0;
    uint16_t mProtocolCode = 0;

    uint8_t * mProtocolData      = nullptr;
    uint16_t mProtocolDataLength = 0;

    System::PacketBufferHandle mMsgHandle;
};

} // namespace SecureChannel
} // namespace Protocols
} // namespace chip