/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

#if defined(SL_COMPONENT_CATALOG_PRESENT)
  #include "sl_component_catalog.h"
#endif

#ifdef SL_CATALOG_RAIL_MULTIPLEXER_PRESENT
#include "sl_rail_mux_rename.h"
#endif

#include "rail.h"
#include "rail_ieee802154.h"
#include "sl_rail_util_ieee802154_phy_select.h"
#include "sl_status.h"
#include "em_assert.h"
extern void sl_mac_lower_mac_update_active_radio_config(void);

extern RAIL_Handle_t emPhyRailHandle;

#define RUNTIME_PHY_SELECT (0)

#define RUNTIME_PHY_SELECT_STACK_SUPPORT (0 || SL_ZIGBEE_PHY_SELECT_STACK_SUPPORT)

static sl_rail_util_ieee802154_radio_config_t active_radio_config = SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT;


static sl_rail_util_ieee802154_radio_config_t sl_rail_util_get_desired_radio_config(void)
{
  return (SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT
  );
}

#if RUNTIME_PHY_SELECT
#if !RUNTIME_PHY_SELECT_STACK_SUPPORT
#error "Run time PHY select is currently unsupported on the selected stack."
#endif

static bool stackInitialized = false;

static uint8_t blockPhySwitch = 0U;
#define BLOCK_SWITCH_RX 0x01u
#define BLOCK_SWITCH_TX 0x02u
#define setBlockPhySwitch(dir, boolval)   \
  do {                                    \
    if (boolval) {                        \
      blockPhySwitch |= (dir);            \
    } else {                              \
      blockPhySwitch &= (uint8_t) ~(dir); \
    }                                     \
  } while (false)

static bool checkPhySwitch(void)
{
  sl_rail_util_ieee802154_radio_config_t desired_radio_config = sl_rail_util_get_desired_radio_config();
  if ((active_radio_config != desired_radio_config)
      && (blockPhySwitch == 0U)
      && stackInitialized
      && (emPhyRailHandle != NULL)) {
    //@TODO: Ascertain radio is OFF, RXWARM, or RXSEARCH only.
    active_radio_config = desired_radio_config;
    sl_mac_lower_mac_update_active_radio_config();
    return true;
  }
  return false;
}

#else//!RUNTIME_PHY_SELECT

#define setBlockPhySwitch(dir, boolval) /*no-op*/
#define checkPhySwitch() (false)

#endif//RUNTIME_PHY_SELECT

sl_rail_util_ieee802154_radio_config_t sl_rail_util_ieee802154_get_active_radio_config(void)
{
  return active_radio_config;
}

#define ANT_DIV_PHY_DEFAULT_ENABLED (SL_RAIL_UTIL_ANTENNA_RX_DEFAULT_MODE != SL_RAIL_UTIL_ANTENNA_MODE_DISABLED)
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT_SUPPORTED                   \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || !ANT_DIV_PHY_DEFAULT_ENABLED) \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || !SL_RAIL_UTIL_COEX_PHY_ENABLED) \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || !SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_SUPPORTED                    \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || ANT_DIV_PHY_DEFAULT_ENABLED)  \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || !SL_RAIL_UTIL_COEX_PHY_ENABLED) \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || !SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_SUPPORTED                      \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || !ANT_DIV_PHY_DEFAULT_ENABLED) \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || SL_RAIL_UTIL_COEX_PHY_ENABLED)  \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || !SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_SUPPORTED               \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || ANT_DIV_PHY_DEFAULT_ENABLED)  \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || SL_RAIL_UTIL_COEX_PHY_ENABLED)  \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || !SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))

#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_FEM_SUPPORTED                       \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || !ANT_DIV_PHY_DEFAULT_ENABLED) \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || !SL_RAIL_UTIL_COEX_PHY_ENABLED) \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_FEM_SUPPORTED                \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || ANT_DIV_PHY_DEFAULT_ENABLED)  \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || !SL_RAIL_UTIL_COEX_PHY_ENABLED) \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_FEM_SUPPORTED                  \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || !ANT_DIV_PHY_DEFAULT_ENABLED) \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || SL_RAIL_UTIL_COEX_PHY_ENABLED)  \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))
#define SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_FEM_SUPPORTED           \
  ((SL_RAIL_UTIL_ANT_DIV_RX_RUNTIME_PHY_SELECT || ANT_DIV_PHY_DEFAULT_ENABLED)  \
  && (SL_RAIL_UTIL_COEX_RUNTIME_PHY_SELECT || SL_RAIL_UTIL_COEX_PHY_ENABLED)  \
  && (SL_FEM_UTIL_RUNTIME_PHY_SELECT || SL_FEM_UTIL_OPTIMIZED_PHY_ENABLE))

RAIL_Status_t sl_rail_util_ieee802154_config_radio(RAIL_Handle_t railHandle)
{
  // Establish the proper radio config
  RAIL_Status_t status = RAIL_STATUS_INVALID_STATE;
  sl_rail_util_ieee802154_radio_config_t desired_radio_config = sl_rail_util_get_desired_radio_config();
  switch (desired_radio_config) {
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT:
      status = RAIL_IEEE802154_Config2p4GHzRadio(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_DEFAULT_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV:
      status = RAIL_IEEE802154_Config2p4GHzRadioAntDiv(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX:
      status = RAIL_IEEE802154_Config2p4GHzRadioCoex(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX:
      status = RAIL_IEEE802154_Config2p4GHzRadioAntDivCoex(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_FEM_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_FEM:
      status = RAIL_IEEE802154_Config2p4GHzRadioFem(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_FEM_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_FEM_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_FEM:
      status = RAIL_IEEE802154_Config2p4GHzRadioAntDivFem(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_FEM_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_FEM_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_FEM:
      status = RAIL_IEEE802154_Config2p4GHzRadioCoexFem(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_COEX_FEM_SUPPORTED
#if SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_FEM_SUPPORTED
    case SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_FEM:
      status = RAIL_IEEE802154_Config2p4GHzRadioAntDivCoexFem(railHandle);
    break;
#endif //SL_RAIL_UTIL_IEEE802154_RADIO_CONFIG_2P4_ANTDIV_COEX_FEM_SUPPORTED
    default:
      break;
  }
  EFM_ASSERT(status == RAIL_STATUS_NO_ERROR);
  active_radio_config = desired_radio_config;

  return status;
}

#if RUNTIME_PHY_SELECT
sl_rail_util_ieee802154_stack_status_t sl_rail_util_ieee802154_phy_select_on_event(
  sl_rail_util_ieee802154_stack_event_t stack_event,
  uint32_t supplement)
{
  bool isReceivingFrame = false;

  switch (stack_event) {
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TICK:
      stackInitialized = true;
      (void) checkPhySwitch();
      break;

    // RX events:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_STARTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACCEPTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACKING:
      setBlockPhySwitch(BLOCK_SWITCH_RX, true);
      break;
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_CORRUPTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_BLOCKED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_ABORTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_FILTERED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ENDED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_ACK_SENT:
      isReceivingFrame = (bool) supplement;
    // FALLTHROUGH
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_RX_IDLED:
      setBlockPhySwitch(BLOCK_SWITCH_RX, isReceivingFrame);
      break;

    // TX events:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_MAC:
      break;
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_PENDED_PHY:
      setBlockPhySwitch(BLOCK_SWITCH_TX, true);
      break;
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_SOON:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_CCA_BUSY:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_STARTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_WAITING:
      break;
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_RECEIVED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ACK_TIMEDOUT:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_BLOCKED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ABORTED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_ENDED:
    case SL_RAIL_UTIL_IEEE802154_STACK_EVENT_TX_IDLED:
      setBlockPhySwitch(BLOCK_SWITCH_TX, false);
      break;
    default:
      break;
  }
  return SL_RAIL_UTIL_IEEE802154_STACK_STATUS_SUCCESS;
}
#else
sl_rail_util_ieee802154_stack_status_t sl_rail_util_ieee802154_phy_select_on_event(
  sl_rail_util_ieee802154_stack_event_t stack_event,
  uint32_t supplement)
{
  (void)stack_event;
  (void)supplement;
  return SL_RAIL_UTIL_IEEE802154_STACK_STATUS_UNSUPPORTED;
}
#endif //RUNTIME_PHY_SELECT
