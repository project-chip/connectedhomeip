/*
 * Copyright 2016-2020 NXP
 * All rights reserved.
 *
 * 
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */
#ifndef _NTAG_DRIVER_H_
#define _NTAG_DRIVER_H_
/** @file ntag_driver.h
 * \brief Public interface to access a NTAG I2C tag over I2C.
 */

/***********************************************************************/
/* INCLUDES                                                            */
/***********************************************************************/
#include "global_types.h"
#include "ntag_defines.h"
#include "HAL_I2C_driver.h"

/***********************************************************************/
/* DEFINES                                                             */
/***********************************************************************/
#define NTAG_2k
//#define NTAG_1k
#if !defined(NTAG_2k) && !defined(NTAG_1k)
#error "You have to define either NTAG_2k or NTAG_1k"
#elif defined(NTAG_2k) && defined(NTAG_1k)
#error "You can't define NTAG_2k and NTAG_1k"
#endif

#define NTAG_INVALID_HANDLE NULL

#ifndef API_DESCRIPTION
#define NTAG_DEVICE_LIST_BEGIN \
    typedef enum               \
    {
#define NTAG_DEVICE_ENTRY(label, i2c_address, isr) label
#define NTAG_DEVICE_LIST_END \
    , NTAG_ID_MAX_DEVICES    \
    }                        \
    NTAG_ID_T;
#endif /* hide from doxygen */

/***********************************************************************/
/* TYPES                                                               */
/***********************************************************************/
#ifndef API_DESCRIPTION
NTAG_DEVICE_LIST_BEGIN
#include "ntag_device_list.h" /* allowed here - generator header */
NTAG_DEVICE_LIST_END
#endif /* hide from doxygen */

typedef struct NTAG_DEVICE *NTAG_HANDLE_T;

typedef enum
{
    NTAG_OK,
    NTAG_ERROR_TX_FAILED,
    NTAG_ERROR_RX_FAILED,
    NTAG_ERROR_WRITE_TIMEOUT,
    NTAG_ERROR_INVALID_PARAM,
    NTAG_CLOSED,
    NTAG_STATUS_MAX_NUMBER
} NTAG_STATUS_T;

typedef enum
{
    NTAG_FD_PIN_STATE_HI,
    NTAG_FD_PIN_STATE_LO,
} NTAG_FD_STATE_T;

/***********************************************************************/
/* GLOBAL VARIABLES                                                    */
/***********************************************************************/

/***********************************************************************/
/* GLOBAL FUNCTION PROTOTYPES                                          */
/***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief wait till the FD Pin has the selected state
 *
 *	This functions waits until the selected event occurs or the timeout value is
 *	reached. See NTAG_EVENT_T for possible events to be waited on.
 *	If you want to use the Interrupted Events set the FD function accordingly
 *	before calling this function. Notice that some Interrupted Events are
 *	indistinguishable, so it will trigger at a false event.
 *
 * \param	ntag      	handle to identify the NTAG device instance
 * \param	event     	state to be waited for
 * \param	timeout_ms	timeout value in ms
 *
 * \return			TRUE on timeout
 */
BOOL NTAG_WaitForFDPinState(NTAG_HANDLE_T ntag, NTAG_FD_STATE_T state, uint32_t timeout_ms);

/**
 * \brief initialize the selected NTAG device for operation
 *
 *	This function registers the specified I2C bus and returns a
 *	handle to the selected device to be used for further operations.
 *
 * \param	ntag_id	identifier of device to be selected
 * \param	i2cbus	handle to I2C bus instance to be used
 *
 * \return 		 	NTAG_INVALID_HANDLE on failure
 */
NTAG_HANDLE_T NTAG_InitDevice(NTAG_ID_T ntag_id, HAL_I2C_HANDLE_T i2cbus);

/**
 * \brief Set the i2c address
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	address	I2C address to be set
 *
 * \return			TRUE on failure
 */
BOOL NTAG_SetNtagI2cAddress(NTAG_HANDLE_T ntag, uint8_t address);

/**
 * \brief close handle and unregister I2C bus instance
 *
 *	This function unregisters the associated I2C bus handle and closes
 *	the handle. The handle can no longer be used for further operation.
 *
 * \param	ntag	handle to identify the NTAG device instance
 *
 * \return	none
 */
void NTAG_CloseDevice(NTAG_HANDLE_T ntag);

/**
 * \brief read len number of bytes from the selected NTAG device
 *
 *	This functions reads the specified number of bytes from the selected
 *	NTAG device, starting at the given address. Reading from an invalid address
 *	will result in a failed read.
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	bytes	array of bytes to store read data
 * \param	address	byte address in device memory space to start reading from
 * \param	len 	number of bytes to be read
 *
 * \return			TRUE on failure
 */
BOOL NTAG_ReadBytes(NTAG_HANDLE_T ntag, uint16_t address, uint8_t *bytes, uint16_t len);

/**
 * \brief write len number of bytes to the selected NTAG device
 *
 *	This functions writes the specified number of bytes to the given address of
 *	the selected NTAG device. Writing to an invalid address
 *	is undefined. 
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	bytes	array of bytes to be written
 * \param	address	byte address in device memory space to start writing to
 * \param	len 	number of bytes to be written
 *
 * \return			TRUE on failure
 */
BOOL NTAG_WriteBytes(NTAG_HANDLE_T ntag, uint16_t address, const uint8_t *bytes, uint16_t len);

/**
 * \brief read a register from the selected NTAG device
 *
 *	This functions reads the specified 8-bit register from the selected
 *	NTAG device.
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	reg 	register offset from the start of the register block
 * \param	val 	byte to store read value
 *
 * \return			TRUE on failure
 */
BOOL NTAG_ReadRegister(NTAG_HANDLE_T ntag, uint8_t reg, uint8_t *val);

/**
 * \brief write a register of the selected NTAG device
 *
 *	This functions writes the specified 8-bit register of the selected
 *	NTAG device. Only the mask selected bits will be written.
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	reg 	register offset from the start of the register block
 * \param	mask	only bits set to one will be written
 * \param	val 	8-bit value to be written
 *
 * \return			TRUE on failure
 */
BOOL NTAG_WriteRegister(NTAG_HANDLE_T ntag, uint8_t reg, uint8_t mask, uint8_t val);

/**
 * \brief read the configuration from the selected NTAG device
 *
 *	This functions reads the specified 8-bit value from the selected
 *	NTAG device.
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	reg 	configuration offset from the start of the mem block
 * \param	val 	byte to store read value
 *
 * \return			TRUE on failure
 */
BOOL NTAG_ReadConfiguration(NTAG_HANDLE_T ntag, uint8_t reg, uint8_t *val);

/**
 * \brief write the configuration of the selected NTAG device
 *
 *	This functions writes the specified 8-bit value of the selected
 *	NTAG device. Only the mask selected bits will be written.
 *
 * \param	ntag	handle to identify the NTAG device instance
 * \param	reg 	configuration offset from the start of the mem block
 * \param	mask	only bits set to one will be written
 * \param	val 	8-bit value to be written
 *
 * \return			TRUE on failure
 */
BOOL NTAG_WriteConfiguration(NTAG_HANDLE_T ntag, uint8_t reg, uint8_t mask, uint8_t val);

/**
 * \brief get the error code of the last failure
 *
 *	If the previous call to any of the read/write function failed this function
 *	will return the reason via the corresponding error code. The error code is not
 *	latched, therefore any successful read/write after a failure will reset the
 *	error code.
 *	This function will return NTAG_CLOSED on a closed handle.
 *
 * \param	ntag	handle to identify the NTAG device instance
 *
 * \return			NTAG status code
 */
NTAG_STATUS_T NTAG_GetLastError(NTAG_HANDLE_T ntag);

#ifdef __cplusplus
}
#endif

#endif /* _NTAG_DRIVER_H_ */
