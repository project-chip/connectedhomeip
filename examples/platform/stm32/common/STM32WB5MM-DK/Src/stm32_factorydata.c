/**
 ******************************************************************************
 * @file    stm32_factorydata.c
 * @author  MCD Application Team
 * @brief   Middleware between matter factory data and external flash ,
 *          to manage factory data needed for Matter
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32_factorydata.h"
#include "stm_ext_flash.h"

/* Private defines -----------------------------------------------------------*/
#define HEADER_SIZE 8
#define FACTORY_DATA_START_ADDR 0x901C0000U
#define FACTORY_DATA_END_ADDR 0x901CFFFFU
#define DATA_MAX_LENGTH 603

/* Private variables ---------------------------------------------------------*/
typedef struct
{
    uint32_t tag_id;
    uint32_t data_length;
} header;

/* Private functions prototypes-----------------------------------------------*/
static FACTORYDATA_StatusTypeDef Get_TagLocation(FACTORYDATA_TagId tag, uint32_t * out_Location, uint32_t * out_length);

/*************************************************************
 *
 * PUBLIC FUNCTIONS
 *
 *************************************************************/

FACTORYDATA_StatusTypeDef FACTORYDATA_GetValue(FACTORYDATA_TagId tag, uint8_t * data, uint32_t size, uint32_t * out_datalength)
{

    FACTORYDATA_StatusTypeDef err = DATAFACTORY_DATA_NOT_FOUND;
    uint32_t Location;
    uint32_t datalength;

    if ((data == NULL) || (out_datalength == NULL))
    {
        return DATAFACTORY_PARAM_ERROR;
    }

    // search for tag location
    err = Get_TagLocation(tag, &Location, &datalength);
    if (err != DATAFACTORY_OK)
    {
        return err;
    }
    else
    {
        if (datalength > size)
        {
            return DATAFACTORY_BUFFER_TOO_SMALL;
        }
        // Read data
        err = STM_EXT_FLASH_ReadChunk(Location, data, datalength);
        if (err == DATAFACTORY_OK)
        {
            *out_datalength = datalength;
        }
    }

    return err;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static FACTORYDATA_StatusTypeDef Get_TagLocation(FACTORYDATA_TagId tag, uint32_t * out_Location, uint32_t * out_length)
{
    FACTORYDATA_StatusTypeDef err = DATAFACTORY_OK;
    header Header_tag;
    uint8_t Header_data[HEADER_SIZE];
    uint32_t location_tmp = FACTORY_DATA_START_ADDR;

    // read all header until find the right tag
    do
    {
        memset(Header_data, 0, HEADER_SIZE);
        err = STM_EXT_FLASH_ReadChunk(location_tmp, Header_data, HEADER_SIZE);
        if (err != DATAFACTORY_OK)
        {
            return DATAFACTORY_PARAM_ERROR;
        }

        // retrieve header with tag_id and data_length
        Header_tag.tag_id      = Header_data[0] + ((Header_data[1]) << 8) + ((Header_data[2]) << 16) + ((Header_data[3]) << 24);
        Header_tag.data_length = Header_data[4] + ((Header_data[5]) << 8) + ((Header_data[6]) << 16) + ((Header_data[7]) << 24);

        // check if the length is valid
        if ((Header_tag.data_length > 0) && (Header_tag.data_length < DATA_MAX_LENGTH))
        {
            if (Header_tag.tag_id == tag)
            {
                *out_Location = location_tmp + HEADER_SIZE;
                *out_length   = Header_tag.data_length;
                break;
            }
            // move to the next data
            location_tmp += Header_tag.data_length + HEADER_SIZE;
        }
        else
        {
            return DATAFACTORY_PARAM_ERROR;
        }
    } while (location_tmp < FACTORY_DATA_END_ADDR);

    if (location_tmp > FACTORY_DATA_END_ADDR)
    {
        err = DATAFACTORY_DATA_NOT_FOUND;
    }

    return err;
}
