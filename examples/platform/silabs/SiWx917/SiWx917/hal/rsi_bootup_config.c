/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
 * Includes */
#include "rsi_api.h"
#include "rsi_driver.h"

/*===========================================================================*/
/** 
 * @fn          int16 rsi_mem_wr(uint32 addr, uint16 len, uint8 *dBuf)
 * @brief       Performs a memory write to the Wi-Fi module
 * @param[in]   uint32 addr, address to write to
 * @param[in]   uint16, len, number of bytes to write
 * @param[in]   uint8 *dBuf, pointer to the buffer of data to write
 * @param[out]  none
 * @return      errCode
 *              -1 = SPI busy / Timeout
 *              -2 = SPI Failure
 *              0  = SUCCESS
 *
 * ABH Master Write (Internal Legacy Name)
 */
int16_t rsi_mem_wr(uint32_t addr, uint16_t len, uint8_t *dBuf)
{
  *(uint32_t *)addr = *(uint32_t *)dBuf;

  return 0;
}

/*===========================================================================*/
/**
 * @fn          int16 rsi_mem_rd(uint32 addr, uint16 len, uint8 *dBuf)
 * @brief       Performs a memory read from the Wi-Fi module
 * @param[in]   uint32, address to read from
 * @param[in]   uint16, len, number of bytes to read
 * @param[in]   uint8 *dBuf, pointer to the buffer to receive the data into
 * @param[out]  none
 * @return      errCode
 *              -1 = SPI busy / Timeout
 *              -2 = SPI Failure
 *              0  = SUCCESS
 *
 * ABH Master Read (Internal Legacy Name)
 */
int16_t rsi_mem_rd(uint32_t addr, uint16_t len, uint8_t *dBuf)
{
  *(uint32_t *)dBuf = *(uint32_t *)addr;
  return 0;
}

/*==============================================*/
/**
 * @fn          int16 rsi_boot_insn(uint8 type, uint16 *data)
 * @brief       Sends boot instructions to WiFi module
 * @param[in]   uint8 type, type of the insruction to perform
 * @param[in]   uint32 *data, pointer to data which is to be read/write
 * @param[out]  none
 * @return      errCode
 *              < 0  = Command issued failure/Invalid command 
 *                0  = SUCCESS
 *              > 0  = Read value
 * @section description 
 * This API is used to send boot instructions to WiFi module.
 */

int16_t rsi_boot_insn(uint8_t type, uint16_t *data)
{
  int16_t retval                = 0;
  uint16_t local                = 0;
  uint32_t j                    = 0;
  uint32_t cmd                  = 0;
  uint16_t read_data            = 0;
  volatile int32_t loop_counter = 0;
#ifdef RSI_DEBUG_PRINT
  RSI_DPRINT(RSI_PL3, "\nBootInsn\n");
#endif

  switch (type) {
    case REG_READ:
      retval = rsi_mem_rd(HOST_INTF_REG_OUT, 2, (uint8_t *)&read_data);
      *data  = read_data;
      break;

    case REG_WRITE:
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)data);
      break;

    case PING_WRITE:

      for (j = 0; j < 2048; j++) {
        retval = rsi_mem_wr(0x19000 + (j * 2), 2, (uint8_t *)((uint32_t)data + (j * 2)));
        if (retval < 0) {
          return retval;
        }
      }

      local  = 0xab49;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&local);
      break;

    case PONG_WRITE:

      for (j = 0; j < 2048; j++) {
        retval = rsi_mem_wr(0x1a000 + (j * 2), 2, (uint8_t *)((uint32_t)data + (j * 2)));
        if (retval < 0) {
          return retval;
        }
      }
      // Perform the write operation
      local  = 0xab4f;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&local);
      break;

    case BURN_NWP_FW:
      cmd    = BURN_NWP_FW | HOST_INTERACT_REG_VALID;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      if (retval < 0) {
        return retval;
      }

      RSI_RESET_LOOP_COUNTER(loop_counter);
      RSI_WHILE_LOOP(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE)
      {
        retval = rsi_mem_rd(HOST_INTF_REG_OUT, 2, (uint8_t *)&read_data);
        if (retval < 0) {
          return retval;
        }
        if (read_data == (RSI_SEND_RPS_FILE | HOST_INTERACT_REG_VALID)) {
          break;
        }
      }
      RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE);
      break;

    case LOAD_NWP_FW:
      cmd    = LOAD_NWP_FW | HOST_INTERACT_REG_VALID;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      break;
    case LOAD_DEFAULT_NWP_FW_ACTIVE_LOW:
      cmd    = LOAD_DEFAULT_NWP_FW_ACTIVE_LOW | HOST_INTERACT_REG_VALID;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      break;
    case RSI_UPGRADE_BL:
      cmd    = RSI_UPGRADE_BL | HOST_INTERACT_REG_VALID;
      retval = rsi_mem_wr(HOST_INTF_REG_IN, 2, (uint8_t *)&cmd);
      if (retval < 0) {
        return retval;
      }
      RSI_RESET_LOOP_COUNTER(loop_counter);
      RSI_WHILE_LOOP(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE)
      {
        retval = rsi_mem_rd(HOST_INTF_REG_OUT, 2, (uint8_t *)&read_data);
        if (retval < 0) {
          return retval;
        }
        if (read_data == (RSI_SEND_RPS_FILE | HOST_INTERACT_REG_VALID)) {
          break;
        }
      }
      RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_UPGRADE_IMAGE);
      break;
    default:
      retval = -2;
      break;
  }
  return retval;
}

/*==============================================*/
/**
 * @fn          int16 rsi_waitfor_boardready(void)
 * @brief       Waits to receive board ready from WiFi module
 * @param[in]   none
 * @param[out]  none
 * @return      errCode
 *              0  = SUCCESS
 *              < 0 = Failure 
 *              -3 = Board ready not received
 *              -4 = Bootup options last configuration not saved
 *              -5 = Bootup options checksum failed
 *              -6 = Bootloader version mismatch 
 * @section description 
 * This API is used to check board ready from WiFi module.
 */
int16_t rsi_waitfor_boardready(void)
{
  int16_t retval      = 0;
  uint16_t read_value = 0;

  retval = rsi_boot_insn(REG_READ, &read_value);

  if (retval < 0) {
    return retval;
  }
  if (read_value == 0) {
    return -7;
  }
  if ((read_value & 0xFF00) == (HOST_INTERACT_REG_VALID_READ & 0xFF00)) {
    if ((read_value & 0xFF) == RSI_BOOTUP_OPTIONS_LAST_CONFIG_NOT_SAVED) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTUP OPTIOINS LAST CONFIGURATION NOT SAVED\n");
#endif
      return -4;
    } else if ((read_value & 0xFF) == RSI_BOOTUP_OPTIONS_CHECKSUM_FAIL) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTUP OPTIONS CHECKSUM FAIL\n");
#endif
      return -5;
    }
#if BOOTLOADER_VERSION_CHECK
    else if ((read_value & 0xFF) == BOOTLOADER_VERSION) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTLOADER VERSION CORRECT\n");
#endif
    } else {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "BOOTLOADER VERSION NOT MATCHING\n");
#endif

      return -6;
    }
#endif

#ifdef RSI_DEBUG_PRINT
    RSI_DPRINT(RSI_PL3, "RECIEVED BOARD READY\n");
#endif
    return 0;
  }

#ifdef RSI_DEBUG_PRINT
  RSI_DPRINT(RSI_PL3, "WAITING FOR BOARD READY\n");
#endif
  return -3;
}

/*==============================================*/
/**
 * @fn          int16 rsi_select_option(uint8 cmd)
 * @brief       Sends cmd to select option to load or update configuration 
 * @param[in]   uint8 cmd, type of configuration to be saved
 * @param[out]  none
 * @return      errCode
                < 0 = Command issue failed
 *              0  = SUCCESS
 * @section description 
 * This API is used to send firmware load request to WiFi module or update default configurations.
 */
int16_t rsi_select_option(uint8_t cmd)
{
  uint16_t boot_cmd             = 0;
  int16_t retval                = 0;
  uint16_t read_value           = 0;
  uint8_t image_number          = 0;
  volatile int32_t loop_counter = 0;

  boot_cmd = HOST_INTERACT_REG_VALID | cmd;
  if (cmd == CHECK_NWP_INTEGRITY) {
    boot_cmd &= 0xF0FF;
    boot_cmd |= (image_number << 8);
  }
  retval = rsi_boot_insn(REG_WRITE, &boot_cmd);
  if (retval < 0) {
    return retval;
  }

  if ((cmd != LOAD_NWP_FW) && (cmd != LOAD_DEFAULT_NWP_FW_ACTIVE_LOW) && (cmd != RSI_JUMP_TO_PC)) {
    RSI_RESET_LOOP_COUNTER(loop_counter);
    RSI_WHILE_LOOP(loop_counter, RSI_LOOP_COUNT_SELECT_OPTION)
    {
      retval = rsi_boot_insn(REG_READ, &read_value);
      if (retval < 0) {
        return retval;
      }
      if (cmd == CHECK_NWP_INTEGRITY) {
        if ((read_value & 0xFF) == RSI_CHECKSUM_SUCCESS) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "CHECKSUM SUCCESS\n");
#endif
        } else if (read_value == RSI_CHECKSUM_FAILURE) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "CHECKSUM FAIL\n");
#endif
        } else if (read_value == RSI_CHECKSUM_INVALID_ADDRESS) {
#ifdef RSI_DEBUG_PRINT
          RSI_DPRINT(RSI_PL3, "Invalid Address \n");
#endif
        }
      }
      if (read_value == (HOST_INTERACT_REG_VALID | cmd)) {
        break;
      }
    }
    RSI_CHECK_LOOP_COUNTER(loop_counter, RSI_LOOP_COUNT_SELECT_OPTION);
  } else if ((cmd == LOAD_NWP_FW) || (cmd == LOAD_DEFAULT_NWP_FW_ACTIVE_LOW) || (cmd == RSI_JUMP_TO_PC)) {
    retval = rsi_boot_insn(REG_READ, &read_value);
    if (retval < 0) {
      return retval;
    }
    if ((read_value & 0xFF) == VALID_FIRMWARE_NOT_PRESENT) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "VALID_FIRMWARE_NOT_PRESENT\n");
#endif
      return -1;
    }
    if ((read_value & 0xFF) == RSI_INVALID_OPTION) {
#ifdef RSI_DEBUG_PRINT
      RSI_DPRINT(RSI_PL3, "INVALID CMD\n");
#endif
      return -1;
    }
  }
  return retval;
}
