/*
 * Copyright (c) 2023 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ASRFactoryDataParser.h"
#include "stdio.h"
#include "string.h"
#include "tlv_decode.h"
#ifdef CFG_PLF_RV32
#include "asr_flash.h"
#include "asr_rv32.h"
#elif defined CFG_PLF_DUET
#include "duet_cm4.h"
#include "duet_flash.h"
#define asr_flash_read duet_flash_read
#define asr_factory_decrypt_dac_prvkey duet_factory_decrypt_dac_prvkey
#else
#include "lega_cm4.h"
#include "lega_flash.h"
#define asr_flash_read lega_flash_read
#endif

#if CONFIG_ENABLE_ASR_FACTORY_DATA_PROVIDER

/* Logic partition on flash devices for matter */
const matter_partition_t asr_matter_partitions_table[] =
{
    [ASR_VERSION_PARTITION] =
    {
        .partition_name = "version",
    },
    [ASR_CONFIG_PARTITION] =
    {
        .partition_name = "config",
    },
    [ASR_ITERATION_COUNT_PARTITION] =
    {
        .partition_name = "iteration-count",
    },
    [ASR_SALT_PARTITION] =
    {
        .partition_name = "salt",
    },
    [ASR_VERIFIER_PARTITION] =
    {
        .partition_name = "verifier",
    },
    [ASR_DISCRIMINATOR_PARTITION] =
    {
        .partition_name = "discriminator",
    },
    [ASR_DAC_CERT_PARTITION] =
    {
        .partition_name = "dac-cert",
    },
    [ASR_DAC_KEY_PARTITION] =
    {
        .partition_name = "dac-pri-key",
    },
    [ASR_DAC_PUB_KEY_PARTITION] =
    {
        .partition_name = "dac-pub-key",
    },
    [ASR_PAI_CERT_PARTITION] =
    {
        .partition_name = "pai-cert",
    },
    [ASR_CERT_DCLRN_PARTITION] =
    {
        .partition_name = "cert-dclrn",
    },
    [ASR_CHIP_ID_PARTITION] =
    {
        .partition_name = "chip-id",
    },
#if CONFIG_ENABLE_ASR_FACTORY_DEVICE_INFO_PROVIDER
    [ASR_VENDOR_NAME_PARTITION] =
    {
        .partition_name = "vendor-name",
    },
    [ASR_VENDOR_ID_PARTITION] =
    {
        .partition_name = "vendor-id",
    },
    [ASR_PRODUCT_NAME_PARTITION] =
    {
        .partition_name = "product-name",
    },
    [ASR_PRODUCT_ID_PARTITION] =
    {
        .partition_name = "product-id",
    },
    [ASR_ROTATING_UNIQUE_ID_PARTITION] =
    {
        .partition_name = "rd-id-uid",
    },
    [ASR_MANUFACTURY_DATE_PARTITION] =
    {
        .partition_name = "mfg-date",
    },
    [ASR_SERIAL_NUMBER_PARTITION] =
    {
        .partition_name = "serial-num",
    },
    [ASR_HARDWARE_VERSION_PARTITION] =
    {
        .partition_name = "hardware-ver",
    },
    [ASR_HARDWARE_VERSION_STR_PARTITION] =
    {
        .partition_name = "hw-ver-str",
    },
    [ASR_PRODUCT_URL_PARTITION] =
    {
        .partition_name = "product-url",
    },
    [ASR_PRODUCT_LABEL_PARTITION] =
    {
        .partition_name = "product-label",
    },
    [ASR_PART_NUMBER_PARTITION] =
    {
        .partition_name = "part-number",
    },
#endif
    [ASR_MATTER_PARTITION_MAX] =
    {
        .partition_name = NULL, //for end don't change,
    }
};

static asr_tlv_context matter_tlv_ctx;

uint32_t asr_matter_find_by_name(const char * name, uint8_t * buf, uint32_t buf_len, uint32_t * out_len)
{
    tlv_header_t tlv;

    if (asr_tlv_find_by_name(&matter_tlv_ctx, &tlv, MIXCLASS_MATTERCONFIG_TAG,
                             (char *) name)) // parse the data according to the tlv packaging tool
    {
        uint32_t value_len = tlv_htons(tlv->data_len);

        value_len -= MAX_NAME_LEN;

        if (value_len > buf_len)
        {
            // buf_len too small
            return -2;
        }

        *out_len = value_len;

        memcpy(buf, tlv->data + MAX_NAME_LEN, value_len);

        return 0;
    }
    else
    {
        return -1;
    }
}

static uint32_t asr_factory_toltal_len()
{
    tlv_area_header_t tlv_headr_p = (tlv_area_header_t) MATTER_FLASH_START_ADDR;
    return tlv_headr_p->data_len + 3 * sizeof(uint32_t); // magic_num,crc32_value,data_len
}

#if defined(CFG_PLF_RV32) || defined(CFG_PLF_DUET)
extern int32_t asr_factory_decrypt_dac_prvkey(uint8_t * pDacCipher, uint8_t ucDacKeyLen, uint8_t * pRdBuf, uint32_t * pOutLen);
#endif
static uint32_t matter_config_value = 0;
static int32_t asr_factory_dac_prvkey_get(uint8_t * pRdBuf, uint32_t * pOutLen)
{
#if defined(CFG_PLF_RV32) || defined(CFG_PLF_DUET)
    uint32_t off_set = asr_factory_toltal_len();
    uint8_t ucCipher[DAC_PRIKEY_LEN];
#endif
    int32_t ret = 0;

    if (NULL == pRdBuf || NULL == pOutLen)
    {
        return -1;
    }
    if (matter_config_value & MATTER_NO_KEY)
    {
#if defined(CFG_PLF_RV32) || defined(CFG_PLF_DUET)
        // get DAC cipher
        ret = asr_flash_read(ASR_CONFIG_BASE, (uint32_t *) &off_set, (void *) ucCipher, DAC_PRIKEY_LEN);
        if (ret == 0)
        {
            ret = asr_factory_decrypt_dac_prvkey(ucCipher, 32, pRdBuf, pOutLen);
        }
#endif
    }
    else
    {
        ret = asr_matter_find_by_name(asr_matter_partitions_table[ASR_DAC_KEY_PARTITION].partition_name, pRdBuf, DAC_PRIKEY_LEN,
                                      pOutLen);
    }
    return ret;
}

int32_t asr_factory_config_read(asr_matter_partition_t matter_partition, uint8_t * buf, uint32_t buf_len, uint32_t * out_len)
{
    int32_t ret = 0;
    if (matter_partition < ASR_MATTER_PARTITION_MAX)
    {
        if (matter_partition == ASR_DAC_KEY_PARTITION)
        {
            ret = asr_factory_dac_prvkey_get(buf, out_len);
            if (ret != 0)
            {
                return ret;
            }
        }
        else
        {
            ret = asr_matter_find_by_name(asr_matter_partitions_table[matter_partition].partition_name, buf, buf_len, out_len);
            if (ret != 0)
            {
                return ret;
            }
        }
    }
    else
    {
        // can't find matter partition
        ret = -3;
    }

    return ret;
}

static int asr_partition_members_count_cb(tlv_header_t tlv, void * arg1, void * arg2)
{
    int * count = arg1;

    *count = *count + 1;

    return 0;
}

static int32_t asr_partition_table_check(void)
{
    int32_t ret = 0;
    uint8_t buf[4];
    size_t outlen = 0;
    uint32_t val;
    ret = asr_matter_find_by_name(asr_matter_partitions_table[ASR_VERSION_PARTITION].partition_name, buf, sizeof(uint32_t),
                                  (uint32_t *) &outlen);
    if (ret == 0)
    {
        val = *(uint32_t *) buf;
        if (val == MATTER_FACTORY_VERSION)
        {
            ret = asr_matter_find_by_name(asr_matter_partitions_table[ASR_CONFIG_PARTITION].partition_name, buf, sizeof(uint32_t),
                                          (uint32_t *) &outlen);
            if (ret == 0)
            {
                matter_config_value = *(uint32_t *) buf;
            }
        }
        else
        {
            ret = -2;
        }
    }
    return ret;
}

static int32_t asr_partition_table_load(void)
{
    int table_members = 0;

    if (asr_tlv_poll_class_members(&matter_tlv_ctx, MIXCLASS_MATTERCONFIG_TAG, asr_partition_members_count_cb, &table_members,
                                   NULL))
    {
        if (table_members > 0)
        {
            return asr_partition_table_check();
        }
    }

    return -1;
}

uint8_t asr_factory_check()
{
    if (asr_tlv_init(&matter_tlv_ctx, MATTER_FLASH_START_ADDR) != 0)
    {
        return -1;
    }

    return asr_partition_table_load();
}

#endif
