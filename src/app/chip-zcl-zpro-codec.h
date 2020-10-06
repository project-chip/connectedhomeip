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

#ifndef CHIP_ZCL_ZPRO_CODEC_H
#define CHIP_ZCL_ZPRO_CODEC_H

#include <stdbool.h>
#include <stdint.h>

typedef uint16_t EmberApsOption;

/** @brief An in-memory representation of a ZigBee APS frame
 * of an incoming or outgoing message. Copy pasted here so that we can compile this unit of code independently.
 */
typedef struct
{
    /** The application profile ID that describes the format of the message. */
    uint16_t profileId;
    /** The cluster ID for this message. */
    uint16_t clusterId;
    /** The source endpoint. */
    uint8_t sourceEndpoint;
    /** The destination endpoint. */
    uint8_t destinationEndpoint;
    /** A bitmask of options from the enumeration above. */
    EmberApsOption options;
    /** The group ID for this message, if it is multicast mode. */
    uint16_t groupId;
    /** The sequence number. */
    uint8_t sequence;
    uint8_t radius;
} EmberApsFrame;

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Encode an on command for on-off server into buffer including the APS frame
 */
uint16_t encodeOnCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/** @brief Encode an off command for on-off server into buffer including the APS frame
 */

uint16_t encodeOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/** @brief Encode a toggle command for on-off server into buffer including the APS frame
 */

uint16_t encodeToggleCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/**
 * @brief Encode a Read Attributes command for the given cluster and the given
 * list of attributes.
 */
uint16_t encodeReadAttributesCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t cluster_id,
                                     const uint16_t * attr_ids, uint16_t attr_id_count);

/**
 * @brief Encode a command to read the OnOff attribute from the on/off
 * cluster.
 */
uint16_t encodeReadOnOffCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/** @brief Extracts an aps frame from buffer into outApsFrame
 * @param buffer Buffer to read from
 * @param buf_length Length of buffer
 * @param outApsFrame Pointer to EmberApsFrame struct to read into
 * @return returns the number of bytes that were consumed to read out the EmberApsFrame. 0 means an error was encountered
 */
uint16_t extractApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * outApsFrame);

/** @brief Populates msg with address of the zcl message within buffer.
 * @return Returns the length of msg buffer. Returns 0 on error e.g. if buffer is too short.
 */
uint16_t extractMessage(uint8_t * buffer, uint16_t buffer_length, uint8_t ** msg);

/** @brief Prints an aps frame struct
 */
void printApsFrame(EmberApsFrame * frame);

/**
 * @brief Encode an APS frame into the given buffer.  Returns the number of
 * bytes of buffer used by the encoding or 0 if the given buffer is not big
 * enough.  If buffer is null, no encoding will happen; the function will
 * instead return the number of bytes that would be needed to encode the APS
 * frame.
 *
 * @param[in] buffer The buffer to write to.  If null, the call is in "count the
 *                   bytes" mode, and no writing will happen.
 * @parem[in] buf_length The size of the buffer.  Ignored if buffer is null.
 * @param[in] apsFrame The frame to encode.
 *
 * @return
 *   - If buffer is null, the number of bytes needed to encode.  If this number
 *     does not fit in a uint16_t, 0 will be returned.
 *   - If buffer is non-null but buf_length is not enough to hold the
 *     EmberApsFrame, 0.
 *   - If buffer is non-null and buf_length is large enough, the number of bytes
 *     placed in buffer.
 */
uint16_t encodeApsFrame(uint8_t * buffer, uint16_t buf_length, EmberApsFrame * apsFrame);

/**
 * Identify cluster commands
 * */

/**
 * @brief Encode an identify query command for the identify cluster
 * @param buffer Buffer to encode into
 * @param buf_length Length of buffer
 * @param destination_endpoint destination endpoint
 * */
uint16_t encodeIdentifyQueryCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/**
 * @brief Encode an identify command for the identify cluster
 * @param buffer Buffer to encode into
 * @param buf_length Length of buffer
 * @param destination_endpoint destination endpoint
 * @param duration The duration for which the device will identify itself
 * */
uint16_t encodeIdentifyCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t duration);

/**
 * @brief Encode a command to read the current temperature attribute from the Temperature Measurement
 * cluster.
 */
uint16_t encodeReadCurrentTemperatureCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

/*
 * Color control cluster commands
 */

/**
 * @brief Encode a move-to-hue command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param hue                   The hue value of the color
 * @param direction             A direction to change the hue to
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveToHueCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t hue, uint8_t direction,
                                uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-hue command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param moveMode              The move mode to apply
 * @param rate                  The rate of the move
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveHueCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t moveMode, uint8_t rate,
                              uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a step-hue command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param stepMode              The step mode to apply
 * @param stepSize              The step size
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeStepHueCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t stepMode,
                              uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-to-saturation command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param saturation            The saturation value of the color
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveToSaturationCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t saturation,
                                       uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-saturation command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param moveMode              The move mode to apply
 * @param rate                  The rate of the move
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveSaturationCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t moveMode,
                                     uint8_t rate, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a step-saturation command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param stepMode              The step mode to apply
 * @param stepSize              The step size
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeStepSaturationCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t stepMode,
                                     uint8_t stepSize, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-to-hue-saturation command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param hue                   The hue value of the color
 * @param saturation            The saturation value of the color
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveToHueSaturationCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t hue,
                                          uint8_t saturation, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-to-color command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param colorX                The x color value
 * @param colorY                The y color value
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveToColorCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t colorX,
                                  uint16_t colorY, uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-color command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param rateX                 The x rate value
 * @param rateY                 The y rate value
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveColorCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t rateX, uint16_t rateY,
                                uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a step-color command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param stepX                 The x step value
 * @param stepY                 The y step value
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeStepColorCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint16_t stepX, uint16_t stepY,
                                uint16_t transitionTime, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a move-to-color-temperature command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param colorTemperature      The temperature of the color
 * @param transitionTime        The number of steps for the color transition
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveToColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint,
                                             uint16_t colorTemperature, uint16_t transitionTime, uint8_t optionMask,
                                             uint8_t optionOverride);

/**
 * @brief Encode a move-color-temperature command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param moveMode              The move mode to apply
 * @param rate                  The rate of the move
 * @param colorTemperatureMin   The min temperature of the color
 * @param colorTemperatureMax   The max temperature of the color
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeMoveColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t moveMode,
                                           uint16_t rate, uint16_t colorTemperatureMin, uint16_t colorTemperatureMax,
                                           uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a step-color-temperature command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param stepMode              The step mode to apply
 * @param stepSize              The step size
 * @param transitionTime        The number of steps for the color transition
 * @param colorTemperatureMin   The min temperature of the color
 * @param colorTemperatureMax   The max temperature of the color
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeStepColorTemperatureCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t stepMode,
                                           uint16_t stepSize, uint16_t transitionTime, uint16_t colorTemperatureMin,
                                           uint16_t colorTemperatureMax, uint8_t optionMask, uint8_t optionOverride);

/**
 * @brief Encode a stop-move-step command for the Color Control cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * @param optionMask            An option mask
 * @param optionOverride        An option override
 * */
uint16_t encodeStopMoveStepCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint, uint8_t optionMask,
                                   uint8_t optionOverride);

/**
 * Basic cluster commands
 */

/**
 * @brief Encode a reset-to-factory command for the Basic cluster
 * @param buffer                Buffer to encode into
 * @param buf_length            Length of buffer
 * @param destination_endpoint  Destination endpoint
 * */
uint16_t encodeResetToFactoryCommand(uint8_t * buffer, uint16_t buf_length, uint8_t destination_endpoint);

#ifdef __cplusplus
}
#endif

#endif // CHIP_ZCL_ZPRO_CODEC_H
