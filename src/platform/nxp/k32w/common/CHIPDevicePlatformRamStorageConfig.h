/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    All rights reserved.
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
 *          Configuration of RAM storage metadata: key IDs and NVM IDs.
 */

/* Base key IDs used when creating new keys for RAM storage instances. */
/**
 * @def kKeyId_Factory
 *
 * Base key id used for factory RAM storage.
 */
#ifndef kKeyId_Factory
#define kKeyId_Factory (uint8_t) 0x01
#endif

/**
 * @def kKeyId_Config
 *
 * Base key id used for config RAM storage.
 */
#ifndef kKeyId_Config
#define kKeyId_Config (uint8_t) 0x02
#endif

/**
 * @def kKeyId_Counter
 *
 * Base key id used for counter RAM storage.
 */
#ifndef kKeyId_Counter
#define kKeyId_Counter (uint8_t) 0x03
#endif

/**
 * @def kKeyId_KvsKeys
 *
 * Base key id used for KVS keys RAM storage.
 */
#ifndef kKeyId_KvsKeys
#define kKeyId_KvsKeys (uint8_t) 0x04
#endif

/**
 * @def kKeyId_KvsValues
 *
 * Base key id used for KVS values RAM storage.
 */
#ifndef kKeyId_KvsValues
#define kKeyId_KvsValues (uint8_t) 0x05
#endif

/* PDM IDs used when defining RAM storage instances or RAM buffers (OT). */
/**
 * @def kNvmId_Factory
 *
 * PDM ID used for factory RAM storage.
 */
#ifndef kNvmId_Factory
#define kNvmId_Factory (uint16_t) 0x5001
#endif

/**
 * @def kNvmId_Config
 *
 * PDM ID used for config RAM storage.
 */
#ifndef kNvmId_Config
#define kNvmId_Config (uint16_t) 0x5002
#endif

/**
 * @def kNvmId_Counter
 *
 * PDM ID used for counter RAM storage.
 */
#ifndef kNvmId_Counter
#define kNvmId_Counter (uint16_t) 0x5003
#endif

/**
 * @def kNvmId_KvsKeys
 *
 * PDM ID used for KVS keys RAM storage.
 */
#ifndef kNvmId_KvsKeys
#define kNvmId_KvsKeys (uint16_t) 0x6000
#endif

/**
 * @def kNvmId_KvsValues
 *
 * PDM ID used for KVS values RAM storage.
 */
#ifndef kNvmId_KvsValues
#define kNvmId_KvsValues (uint16_t) 0x6001
#endif

/**
 * @def kNvmId_OTConfigData
 *
 * PDM ID used for OT RAM buffer.
 */
#ifndef kNvmId_OTConfigData
#define kNvmId_OTConfigData (uint16_t) 0x4F00
#endif

/**
 * @def kNvmId_ApplicationBase
 *
 * Base PDM ID to be used by applications to define their own
 * PDM IDs by using an offset.
 */
#ifndef kNvmId_ApplicationBase
#define kNvmId_ApplicationBase (uint16_t) 0xA000
#endif

#if CONFIG_CHIP_K32W0_REAL_FACTORY_DATA
/**
 * @def kNvmId_FactoryDataBackup
 *
 * PDM ID used for factory data backup in K32W0FactoryDataProvider.
 */
#ifndef kNvmId_FactoryDataBackup
#define kNvmId_FactoryDataBackup (uint16_t) 0x7000
#endif
#endif // CONFIG_CHIP_K32W0_REAL_FACTORY_DATA
