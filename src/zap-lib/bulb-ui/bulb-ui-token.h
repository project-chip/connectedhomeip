/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#define CREATOR_SHORT_REBOOT_MONITOR (0x000c)
#define NVM3KEY_SHORT_REBOOT_MONITOR (NVM3KEY_DOMAIN_USER | 0x000c)

#ifdef DEFINETOKENS

DEFINE_COUNTER_TOKEN(SHORT_REBOOT_MONITOR, uint32_t, 0)

#endif //DEFINETOKENS
