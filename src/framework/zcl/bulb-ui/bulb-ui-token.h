/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#define CREATOR_SHORT_REBOOT_MONITOR (0x000c)
#define NVM3KEY_SHORT_REBOOT_MONITOR (NVM3KEY_DOMAIN_USER | 0x000c)

#ifdef DEFINETOKENS

DEFINE_COUNTER_TOKEN(SHORT_REBOOT_MONITOR, uint32_t, 0)

#endif //DEFINETOKENS
