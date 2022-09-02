
#include "device_creds.h"
#include <zigbee_app_framework_common.h>
#include <stack/include/ember.h>
#include <app/util/ezsp/ezsp-protocol.h>
#include <hal/hal.h>
#include <psa/crypto.h>
#include <em_msc.h>
#include <stdio.h>

//------------------------------------------------------------------------------
// Private
//------------------------------------------------------------------------------

typedef enum
{
    CREDS_STATE_START = 0x00,
    CREDS_STATE_CSR_GENERATE = 0x01,
    CREDS_STATE_CSR_READY = 0x02,
    CREDS_STATE_CSR_SENT = 0x03,
    CREDS_STATE_FAILURE = 0xff,

} creds_state_t;

static uint8_t _csr_buff[CREDS_CSR_LENGTH_MAX] = {0};
static creds_file_t _csr;

static sl_zigbee_event_t _creds_event;
static uint32_t _event_period = MILLISECOND_TICKS_PER_SECOND;
static creds_state_t _state = CREDS_STATE_START;

static void credsEventHandler(sl_zigbee_event_t *event)
{
    sl_zigbee_event_set_delay_ms(&_creds_event, _event_period);
}

//------------------------------------------------------------------------------
// Public
//------------------------------------------------------------------------------

int device_creds_init()
{
    // Initialize PSA Crypto
    int err = psa_crypto_init();
    ASSERT(!err, return err, "psa_crypto_init() error %d", err);

    // Initialize CSR buffer
    creds_file_init(&_csr, _csr_buff, sizeof(_csr_buff));

    return 0;
}

int device_creds_action()
{
    return 0;
}

/** @brief Main Init
 *
 * This function is called when the application starts and can be used to
 * perform any additional initialization required at system startup.
 */
void emberAfMainInitCallback(void)
{
    device_creds_init();

    sl_zigbee_event_init(&_creds_event, credsEventHandler);
    sl_zigbee_event_set_active(&_creds_event);
}

/** @brief Incoming Custom EZSP Message Callback
 *
 * This function is called when the NCP receives a custom EZSP message from the
 * HOST.  The message length and payload is passed to the callback in the first
 * two arguments.  The implementation can then fill in the reply and set
 * the replayPayloadLength to the number of bytes in the reply.
 * See documentation for the function ezspCustomFrame on sending these messages
 * from the HOST.
 *
 * @param payload_size The length of the payload.
 * @param payload The custom message that was sent from the HOST.
 * Ver.: always
 * @param replay_size The length of the reply.  This needs to be
 * set by the implementation in order for a properly formed respose to be sent
 * back to the HOST. Ver.: always
 * @param reply The custom message to send back to the HOST in respose
 * to the custom message. Ver.: always
 *
 * @return An ::EmberStatus indicating the result of the custom message
 * handling.  This returned status is always the first byte of the EZSP
 * response.
 */
EmberStatus emberAfPluginXncpIncomingCustomFrameCallback(uint8_t payload_size,
                                                         uint8_t *payload,
                                                         uint8_t *replay_size,
                                                         uint8_t *reply)
{
    EmberStatus status = EMBER_INVALID_CALL;
    int err = 0;

    if (CREDS_STATE_FAILURE == _state)
    {
        _state = CREDS_STATE_START;
        return EMBER_ERR_FATAL;
    }

    // First byte is the command ID.
    creds_command_t command_id = payload[0];

    switch (command_id)
    {
    case CREDS_COMMAND_CSR_GENERATE:
        ASSERT(CREDS_STATE_START == _state || CREDS_STATE_CSR_SENT == _state, return EMBER_INVALID_CALL, "Invalid command %d for state $d\n", command_id, _state);

        err = device_csr_generate(&_csr);
        ASSERT(!err, return EMBER_ERR_FATAL, "CSR generate error %d\n", err);

        // File size in big endian format
        reply[0] = (_csr.size >> 8) & 0xff;
        reply[1] = (_csr.size & 0xff);
        *replay_size = 2;
        status = EMBER_SUCCESS;
        _state = CREDS_STATE_CSR_READY;
        break;

    case CREDS_COMMAND_CSR_READ:
        ASSERT(CREDS_STATE_CSR_READY == _state, return EMBER_INVALID_CALL, "Invalid command %d for state $d\n", command_id, _state);

        if (_csr.offset < _csr.size)
        {
            size_t segment_size = CREDS_FRAME_LENGTH_MAX;
            if (_csr.offset + segment_size > _csr.size)
            {
                // Partial segment
                segment_size = _csr.size - _csr.offset;
            }
            memcpy(reply, &_csr.data[_csr.offset], segment_size);
            _csr.offset += segment_size;
            *replay_size = segment_size;
        }
        if (_csr.offset >= _csr.size)
        {
            _state = CREDS_STATE_CSR_SENT;
        }
        status = EMBER_SUCCESS;
        break;

    default:
        break;
    }

    return status;
}