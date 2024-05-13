/*
 * Copyright (c) 2016-2020 Arm Limited. All rights reserved.
 *
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
#include "mpc_sie_drv.h"

#include <stdbool.h>
#include <stddef.h>

#include "cmsis_compiler.h"

/* Values for hardware version in PIDR0 reg */
#define SIE200 0x60
#define SIE300 0x65

#define MPC_SIE_BLK_CFG_OFFSET 5U

/* Defines with numbering (eg: SIE300) are only relevant to the given SIE
 * version. Defines without the numbering are applicable to all SIE versions.
 */

/* CTRL register bit indexes */
#define MPC_SIE200_CTRL_SEC_RESP                                                                                                   \
    (1UL << 4UL) /* MPC fault triggers a                                                                                           \
                  * bus error                                                                                                      \
                  */
#define MPC_SIE300_CTRL_GATE_REQ                                                                                                   \
    (1UL << 6UL) /* Request for gating                                                                                             \
                  * incoming transfers                                                                                             \
                  */
#define MPC_SIE300_CTRL_GATE_ACK                                                                                                   \
    (1UL << 7UL)                                /* Acknowledge for gating                                                          \
                                                 * incoming transfers                                                              \
                                                 */
#define MPC_SIE_CTRL_AUTOINCREMENT (1UL << 8UL) /* BLK_IDX auto increment */
#define MPC_SIE300_CTRL_SEC_RESP                                                                                                   \
    (1UL << 16UL)                                  /* Response type when SW                                                        \
                                                    * asks to gate the transfer                                                    \
                                                    */
#define MPC_SIE300_CTRL_GATE_PRESENT (1UL << 23UL) /* Gating feature present */
#define MPC_SIE_CTRL_SEC_LOCK_DOWN (1UL << 31UL)   /* MPC Security lock down */

/* PIDR register bit masks */
#define MPC_PIDR0_SIE_VERSION_MASK ((1UL << 8UL) - 1UL)

/* ARM MPC interrupt */
#define MPC_SIE_INT_BIT (1UL)

/* Error code returned by the internal driver functions */
enum mpc_sie_intern_error_t
{
    MPC_SIE_INTERN_ERR_NONE                      = MPC_SIE_ERR_NONE,
    MPC_SIE_INTERN_ERR_NOT_IN_RANGE              = MPC_SIE_ERR_NOT_IN_RANGE,
    MPC_SIE_INTERN_ERR_NOT_ALIGNED               = MPC_SIE_ERR_NOT_ALIGNED,
    MPC_SIE_INTERN_ERR_INVALID_RANGE             = MPC_SIE_ERR_INVALID_RANGE,
    MPC_INTERN_ERR_RANGE_SEC_ATTR_NON_COMPATIBLE = MPC_SIE_ERR_RANGE_SEC_ATTR_NON_COMPATIBLE,
    /* Calculated block index
     * is higher than the maximum allowed by the MPC. It should never
     * happen unless the controlled ranges of the MPC are misconfigured
     * in the driver or if the IP has not enough LUTs to cover the
     * range, due to wrong reported block size for example.
     */
    MPC_SIE_INTERN_ERR_BLK_IDX_TOO_HIGH = -1,

};

/* ARM MPC memory mapped register access structure */
struct mpc_sie_reg_map_t
{
    volatile uint32_t ctrl;           /* (R/W) MPC Control */
    volatile uint32_t reserved[3];    /* Reserved */
    volatile uint32_t blk_max;        /* (R/ ) Maximum value of block based index */
    volatile uint32_t blk_cfg;        /* (R/ ) Block configuration */
    volatile uint32_t blk_idx;        /* (R/W) Index value for accessing block
                                       *       based look up table
                                       */
    volatile uint32_t blk_lutn;       /* (R/W) Block based gating
                                       *       Look Up Table (LUT)
                                       */
    volatile uint32_t int_stat;       /* (R/ ) Interrupt state */
    volatile uint32_t int_clear;      /* ( /W) Interrupt clear */
    volatile uint32_t int_en;         /* (R/W) Interrupt enable */
    volatile uint32_t int_info1;      /* (R/ ) Interrupt information 1 */
    volatile uint32_t int_info2;      /* (R/ ) Interrupt information 2 */
    volatile uint32_t int_set;        /* ( /W) Interrupt set. Debug purpose only */
    volatile uint32_t reserved2[998]; /* Reserved */
    volatile uint32_t pidr4;          /* (R/ ) Peripheral ID 4 */
    volatile uint32_t pidr5;          /* (R/ ) Peripheral ID 5 */
    volatile uint32_t pidr6;          /* (R/ ) Peripheral ID 6 */
    volatile uint32_t pidr7;          /* (R/ ) Peripheral ID 7 */
    volatile uint32_t pidr0;          /* (R/ ) Peripheral ID 0 */
    volatile uint32_t pidr1;          /* (R/ ) Peripheral ID 1 */
    volatile uint32_t pidr2;          /* (R/ ) Peripheral ID 2 */
    volatile uint32_t pidr3;          /* (R/ ) Peripheral ID 3 */
    volatile uint32_t cidr0;          /* (R/ ) Component ID 0 */
    volatile uint32_t cidr1;          /* (R/ ) Component ID 1 */
    volatile uint32_t cidr2;          /* (R/ ) Component ID 2 */
    volatile uint32_t cidr3;          /* (R/ ) Component ID 3 */
};

/*
 * Checks if the address is controlled by the MPC and returns
 * the range index in which it is contained.
 *
 * \param[in]  dev         MPC device to initialize \ref mpc_sie_dev_t
 * \param[in]  addr        Address to check if it is controlled by MPC.
 * \param[out] addr_range  Range index in which it is contained.
 *
 * \return True if the base is controller by the range list, false otherwise.
 */
static uint32_t is_ctrl_by_range_list(struct mpc_sie_dev_t * dev, uint32_t addr, const struct mpc_sie_memory_range_t ** addr_range)
{
    uint32_t i;
    const struct mpc_sie_memory_range_t * range;

    for (i = 0; i < dev->data->nbr_of_ranges; i++)
    {
        range = dev->data->range_list[i];
        if (addr >= range->base && addr <= range->limit)
        {
            *addr_range = range;
            return 1;
        }
    }
    return 0;
}

/*
 * Gets the masks selecting the bits in the LUT of the MPC corresponding
 * to the base address (included) up to the limit address (included)
 *
 * \param[in]   mpc_dev          The MPC device.
 * \param[in]   base             Address in a range controlled by this MPC
 *                               (included), aligned on block size.
 * \param[in]   limit            Address in a range controlled by this MPC
 *                               (included), aligned on block size.
 * \param[out]  range            Memory range in which the base address and
 *                               limit are.
 * \param[out]  first_word_idx   Index of the first touched word in the LUT.
 * \param[out]  nr_words         Number of words used in the LUT. If 1, only
 *                               first_word_mask is valid and limit_word_mask
 *                               must not be used.
 * \param[out]  first_word_mask  First word mask in the LUT will be stored here.
 * \param[out]  limit_word_mask  Limit word mask in the LUT will be stored here.
 *
 * \return Returns error code as specified in \ref mpc_sie_intern_error_t
 */
static enum mpc_sie_intern_error_t get_lut_masks(struct mpc_sie_dev_t * dev, const uint32_t base, const uint32_t limit,
                                                 const struct mpc_sie_memory_range_t ** range, uint32_t * first_word_idx,
                                                 uint32_t * nr_words, uint32_t * first_word_mask, uint32_t * limit_word_mask)
{
    const struct mpc_sie_memory_range_t * base_range;
    uint32_t block_size;
    uint32_t base_block_idx;
    uint32_t base_word_idx;
    uint32_t blk_max;
    const struct mpc_sie_memory_range_t * limit_range;
    uint32_t limit_block_idx;
    uint32_t limit_word_idx;
    uint32_t mask;
    uint32_t norm_base;
    uint32_t norm_limit;
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    /*
     * Check that the addresses are within the controlled regions
     * of this MPC
     */
    if (!is_ctrl_by_range_list(dev, base, &base_range) || !is_ctrl_by_range_list(dev, limit, &limit_range))
    {
        return MPC_SIE_INTERN_ERR_NOT_IN_RANGE;
    }

    /* Base and limit should be part of the same range */
    if (base_range != limit_range)
    {
        return MPC_SIE_INTERN_ERR_INVALID_RANGE;
    }
    *range = base_range;

    block_size = (1 << (p_mpc->blk_cfg + MPC_SIE_BLK_CFG_OFFSET));

    /* Base and limit+1 addresses must be aligned on the MPC block size */
    if (base % block_size || (limit + 1) % block_size)
    {
        return MPC_SIE_INTERN_ERR_NOT_ALIGNED;
    }

    /*
     * Get a normalized address that is an offset from the beginning
     * of the lowest range controlled by the MPC
     */
    norm_base  = (base - base_range->base) + base_range->range_offset;
    norm_limit = (limit - base_range->base) + base_range->range_offset;

    /*
     * Calculate block index and to which 32 bits word it belongs
     */
    limit_block_idx = norm_limit / block_size;
    limit_word_idx  = limit_block_idx / 32;

    base_block_idx = norm_base / block_size;
    base_word_idx  = base_block_idx / 32;

    if (base_block_idx > limit_block_idx)
    {
        return MPC_SIE_INTERN_ERR_INVALID_RANGE;
    }

    /* Transmit the information to the caller */
    *nr_words       = limit_word_idx - base_word_idx + 1;
    *first_word_idx = base_word_idx;

    /* Limit to the highest block that can be configured */
    blk_max = p_mpc->blk_max;

    if ((limit_word_idx > blk_max) || (base_word_idx > blk_max))
    {
        return MPC_SIE_INTERN_ERR_BLK_IDX_TOO_HIGH;
    }

    /*
     * Create the mask for the first word to only select the limit N bits
     */
    *first_word_mask = ~((1 << (base_block_idx % 32)) - 1);

    /*
     * Create the mask for the limit word to select only the first M bits.
     */
    *limit_word_mask = (1 << ((limit_block_idx + 1) % 32)) - 1;
    /*
     * If limit_word_mask is 0, it means that the limit touched block index is
     * the limit in its word, so the limit word mask has all its bits selected
     */
    if (*limit_word_mask == 0)
    {
        *limit_word_mask = 0xFFFFFFFF;
    }

    /*
     * If the blocks to configure are all packed in one word, only
     * touch this word.
     * Code using the computed masks should test if this mask
     * is non-zero, and if so, only use this one instead of the limit_word_mask
     * and first_word_mask.
     * As the only bits that are the same in both masks are the 1 that we want
     * to select, just use XOR to extract them.
     */
    if (base_word_idx == limit_word_idx)
    {
        mask             = ~(*first_word_mask ^ *limit_word_mask);
        *first_word_mask = mask;
        *limit_word_mask = mask;
    }

    return MPC_SIE_INTERN_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_init(struct mpc_sie_dev_t * dev, const struct mpc_sie_memory_range_t ** range_list,
                                  uint8_t nbr_of_ranges)
{
    if ((range_list == NULL) || (nbr_of_ranges == 0))
    {
        return MPC_SIE_INVALID_ARG;
    }

    dev->data->sie_version = get_sie_version(dev);

    if ((dev->data->sie_version != SIE200) && (dev->data->sie_version != SIE300))
    {
        return MPC_SIE_UNSUPPORTED_HARDWARE_VERSION;
    }

    dev->data->range_list     = range_list;
    dev->data->nbr_of_ranges  = nbr_of_ranges;
    dev->data->is_initialized = true;

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_get_block_size(struct mpc_sie_dev_t * dev, uint32_t * blk_size)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (blk_size == 0)
    {
        return MPC_SIE_INVALID_ARG;
    }

    /* Calculate the block size in byte according to the manual */
    *blk_size = (1 << (p_mpc->blk_cfg + MPC_SIE_BLK_CFG_OFFSET));

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_config_region(struct mpc_sie_dev_t * dev, const uint32_t base, const uint32_t limit,
                                           enum mpc_sie_sec_attr_t attr)
{
    enum mpc_sie_intern_error_t error;
    uint32_t first_word_idx;
    uint32_t first_word_mask;
    uint32_t i;
    uint32_t limit_word_mask;
    uint32_t limit_word_idx;
    uint32_t nr_words;
    const struct mpc_sie_memory_range_t * range;
    uint32_t word_value;
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    /* Get the bitmasks used to select the bits in the LUT */
    error = get_lut_masks(dev, base, limit, &range, &first_word_idx, &nr_words, &first_word_mask, &limit_word_mask);

    limit_word_idx = first_word_idx + nr_words - 1;

    if (error != MPC_SIE_INTERN_ERR_NONE)
    {
        /* Map internal error code lower than 0 to a generic errpr */
        if (error < 0)
        {
            return MPC_SIE_ERR_INVALID_RANGE;
        }
        return (enum mpc_sie_error_t) error;
    }

    /*
     * The memory range should allow accesses in with the wanted security
     * attribute if it requires special attribute for successful accesses
     */
    if (range->attr != attr)
    {
        return MPC_SIE_ERR_RANGE_SEC_ATTR_NON_COMPATIBLE;
    }

    /*
     * Starts changing actual configuration so issue DMB to ensure every
     * transaction has completed by now
     */
    __DMB();

    /* Set the block index to the first word that will be updated */
    p_mpc->blk_idx = first_word_idx;

    /* If only one word needs to be touched in the LUT */
    if (nr_words == 1)
    {
        word_value = p_mpc->blk_lutn;
        if (attr == MPC_SIE_SEC_ATTR_NONSECURE)
        {
            word_value |= first_word_mask;
        }
        else
        {
            word_value &= ~first_word_mask;
        }

        /*
         * Set the index again because full word read or write could have
         * incremented it
         */
        p_mpc->blk_idx  = first_word_idx;
        p_mpc->blk_lutn = word_value;

        /* Commit the configuration change */
        __DSB();
        __ISB();

        return MPC_SIE_ERR_NONE;
    }

    /* First word */
    word_value = p_mpc->blk_lutn;
    if (attr == MPC_SIE_SEC_ATTR_NONSECURE)
    {
        word_value |= first_word_mask;
    }
    else
    {
        word_value &= ~first_word_mask;
    }
    /*
     * Set the index again because full word read or write could have
     * incremented it
     */
    p_mpc->blk_idx = first_word_idx;
    /* Partially configure the first word */
    p_mpc->blk_lutn = word_value;

    /* Fully configure the intermediate words if there are any */
    for (i = first_word_idx + 1; i < limit_word_idx; i++)
    {
        p_mpc->blk_idx = i;
        if (attr == MPC_SIE_SEC_ATTR_NONSECURE)
        {
            p_mpc->blk_lutn = 0xFFFFFFFF;
        }
        else
        {
            p_mpc->blk_lutn = 0x00000000;
        }
    }

    /* Partially configure the limit word */
    p_mpc->blk_idx = limit_word_idx;
    word_value     = p_mpc->blk_lutn;
    if (attr == MPC_SIE_SEC_ATTR_NONSECURE)
    {
        word_value |= limit_word_mask;
    }
    else
    {
        word_value &= ~limit_word_mask;
    }
    p_mpc->blk_idx  = limit_word_idx;
    p_mpc->blk_lutn = word_value;

    /* Commit the configuration change */
    __DSB();
    __ISB();

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_get_region_config(struct mpc_sie_dev_t * dev, uint32_t base, uint32_t limit,
                                               enum mpc_sie_sec_attr_t * attr)
{
    enum mpc_sie_sec_attr_t attr_prev;
    uint32_t block_size;
    uint32_t block_size_mask;
    enum mpc_sie_intern_error_t error;
    uint32_t first_word_idx;
    uint32_t first_word_mask;
    uint32_t i;
    uint32_t limit_word_idx;
    uint32_t limit_word_mask;
    uint32_t nr_words;
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;
    const struct mpc_sie_memory_range_t * range;
    uint32_t word_value;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (attr == 0)
    {
        return MPC_SIE_INVALID_ARG;
    }

    /*
     * Initialize the security attribute to mixed in case of early
     * termination of this function. A caller that does not check the
     * returned error will act as if it does not know anything about the
     * region queried, which is the safest bet
     */
    *attr = MPC_SIE_SEC_ATTR_MIXED;

    /*
     * If the base and limit are not aligned, align them and make sure
     * that the resulting region fully includes the original region
     */
    block_size = (1 << (p_mpc->blk_cfg + MPC_SIE_BLK_CFG_OFFSET));

    block_size_mask = block_size - 1;
    base &= ~(block_size_mask);
    limit &= ~(block_size_mask);
    limit += block_size - 1; /* Round to the upper block address,
                              * and then remove one to get the preceding
                              * address.
                              */

    /* Get the bitmasks used to select the bits in the LUT */
    error = get_lut_masks(dev, base, limit, &range, &first_word_idx, &nr_words, &first_word_mask, &limit_word_mask);

    limit_word_idx = first_word_idx + nr_words - 1;

    if (error != MPC_SIE_INTERN_ERR_NONE)
    {
        /* Map internal error code lower than 0 to generic error */
        if (error < 0)
        {
            return MPC_SIE_ERR_INVALID_RANGE;
        }
        return (enum mpc_sie_error_t) error;
    }

    /* Set the block index to the first word that will be updated */
    p_mpc->blk_idx = first_word_idx;

    /* If only one word needs to be touched in the LUT */
    if (nr_words == 1)
    {
        word_value = p_mpc->blk_lutn;
        word_value &= first_word_mask;
        if (word_value == 0)
        {
            *attr = MPC_SIE_SEC_ATTR_SECURE;
            /*
             * If there are differences between the mask and the word value,
             * it means that the security attributes of blocks are mixed
             */
        }
        else if (word_value ^ first_word_mask)
        {
            *attr = MPC_SIE_SEC_ATTR_MIXED;
        }
        else
        {
            *attr = MPC_SIE_SEC_ATTR_NONSECURE;
        }
        return MPC_SIE_ERR_NONE;
    }

    /* Get the partial configuration of the first word */
    word_value = p_mpc->blk_lutn & first_word_mask;
    if (word_value == 0x00000000)
    {
        *attr = MPC_SIE_SEC_ATTR_SECURE;
    }
    else if (word_value ^ first_word_mask)
    {
        *attr = MPC_SIE_SEC_ATTR_MIXED;
        /*
         * Bail out as the security attribute will be the same regardless
         * of the configuration of other blocks
         */
        return MPC_SIE_ERR_NONE;
    }
    else
    {
        *attr = MPC_SIE_SEC_ATTR_NONSECURE;
    }
    /*
     * Store the current found attribute, to check that all the blocks indeed
     * have the same security attribute.
     */
    attr_prev = *attr;

    /* Get the configuration of the intermediate words if there are any */
    for (i = first_word_idx + 1; i < limit_word_idx; i++)
    {
        p_mpc->blk_idx = i;
        word_value     = p_mpc->blk_lutn;
        if (word_value == 0x00000000)
        {
            *attr = MPC_SIE_SEC_ATTR_SECURE;
        }
        else if (word_value == 0xFFFFFFFF)
        {
            *attr = MPC_SIE_SEC_ATTR_NONSECURE;
        }
        else
        {
            *attr = MPC_SIE_SEC_ATTR_MIXED;
            return MPC_SIE_ERR_NONE;
        }

        /* If the attribute is different than the one found before, bail out */
        if (*attr != attr_prev)
        {
            *attr = MPC_SIE_SEC_ATTR_MIXED;
            return MPC_SIE_ERR_NONE;
        }
        attr_prev = *attr;
    }

    /* Get the partial configuration of the limit word */
    p_mpc->blk_idx = limit_word_idx;
    word_value     = p_mpc->blk_lutn & limit_word_mask;
    if (word_value == 0x00000000)
    {
        *attr = MPC_SIE_SEC_ATTR_SECURE;
    }
    else if (word_value ^ first_word_mask)
    {
        *attr = MPC_SIE_SEC_ATTR_MIXED;
        return MPC_SIE_ERR_NONE;
    }
    else
    {
        *attr = MPC_SIE_SEC_ATTR_NONSECURE;
    }

    if (*attr != attr_prev)
    {
        *attr = MPC_SIE_SEC_ATTR_MIXED;
        return MPC_SIE_ERR_NONE;
    }

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_get_ctrl(struct mpc_sie_dev_t * dev, uint32_t * ctrl_val)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (ctrl_val == 0)
    {
        return MPC_SIE_INVALID_ARG;
    }

    *ctrl_val = p_mpc->ctrl;

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_set_ctrl(struct mpc_sie_dev_t * dev, uint32_t mpc_ctrl)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    p_mpc->ctrl = mpc_ctrl;

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_get_sec_resp(struct mpc_sie_dev_t * dev, enum mpc_sie_sec_resp_t * sec_rep)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;
    bool gating_present              = false;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (sec_rep == NULL)
    {
        return MPC_SIE_INVALID_ARG;
    }

    if (dev->data->sie_version == SIE200)
    {
        if (p_mpc->ctrl & MPC_SIE200_CTRL_SEC_RESP)
        {
            *sec_rep = MPC_SIE_RESP_BUS_ERROR;
        }
        else
        {
            *sec_rep = MPC_SIE_RESP_RAZ_WI;
        }
    }
    else if (dev->data->sie_version == SIE300)
    {
        mpc_sie_is_gating_present(dev, &gating_present);
        if (!gating_present)
        {
            return MPC_SIE_ERR_GATING_NOT_PRESENT;
        }

        if (p_mpc->ctrl & MPC_SIE300_CTRL_SEC_RESP)
        {
            /* MPC returns a BUS ERROR response */
            *sec_rep = MPC_SIE_RESP_BUS_ERROR;
        }
        else
        {
            /* MPC sets the ready signals LOW, which stalls any transactions */
            *sec_rep = MPC_SIE_RESP_WAIT_GATING_DISABLED;
        }
    }
    else
    {
        return MPC_SIE_UNSUPPORTED_HARDWARE_VERSION;
    }

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_set_sec_resp(struct mpc_sie_dev_t * dev, enum mpc_sie_sec_resp_t sec_rep)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;
    bool gating_present              = false;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (dev->data->sie_version == SIE200)
    {
        if (sec_rep == MPC_SIE_RESP_BUS_ERROR)
        {
            p_mpc->ctrl |= MPC_SIE200_CTRL_SEC_RESP;
        }
        else if (sec_rep == MPC_SIE_RESP_RAZ_WI)
        {
            p_mpc->ctrl &= ~MPC_SIE200_CTRL_SEC_RESP;
        }
        else
        {
            return MPC_SIE_INVALID_ARG;
        }
    }
    else if (dev->data->sie_version == SIE300)
    {
        mpc_sie_is_gating_present(dev, &gating_present);
        if (!gating_present)
        {
            return MPC_SIE_ERR_GATING_NOT_PRESENT;
        }

        if (sec_rep == MPC_SIE_RESP_BUS_ERROR)
        {
            p_mpc->ctrl |= MPC_SIE300_CTRL_SEC_RESP;
        }
        else if (sec_rep == MPC_SIE_RESP_WAIT_GATING_DISABLED)
        {
            p_mpc->ctrl &= ~MPC_SIE300_CTRL_SEC_RESP;
        }
        else
        {
            return MPC_SIE_INVALID_ARG;
        }
    }
    else
    {
        return MPC_SIE_UNSUPPORTED_HARDWARE_VERSION;
    }

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_irq_enable(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    p_mpc->int_en |= MPC_SIE_INT_BIT;

    return MPC_SIE_ERR_NONE;
}

void mpc_sie_irq_disable(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    p_mpc->int_en &= ~MPC_SIE_INT_BIT;
}

void mpc_sie_clear_irq(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    p_mpc->int_clear = MPC_SIE_INT_BIT;
}

uint32_t mpc_sie_irq_state(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    return (p_mpc->int_stat & MPC_SIE_INT_BIT);
}

enum mpc_sie_error_t mpc_sie_lock_down(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    p_mpc->ctrl |= (MPC_SIE_CTRL_AUTOINCREMENT | MPC_SIE_CTRL_SEC_LOCK_DOWN);

    return MPC_SIE_ERR_NONE;
}

enum mpc_sie_error_t mpc_sie_is_gating_present(struct mpc_sie_dev_t * dev, bool * gating_present)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    if (dev->data->is_initialized != true)
    {
        return MPC_SIE_NOT_INIT;
    }

    if (dev->data->sie_version != SIE300)
    {
        return MPC_SIE_UNSUPPORTED_HARDWARE_VERSION;
    }

    *gating_present = (bool) (p_mpc->ctrl & MPC_SIE300_CTRL_GATE_PRESENT);

    return MPC_SIE_ERR_NONE;
}

uint32_t get_sie_version(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    return p_mpc->pidr0 & MPC_PIDR0_SIE_VERSION_MASK;
}

bool mpc_sie_get_gate_ack(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    return (bool) (p_mpc->ctrl & MPC_SIE300_CTRL_GATE_ACK);
}

void mpc_sie_request_gating(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    p_mpc->ctrl |= MPC_SIE300_CTRL_GATE_REQ;
}

void mpc_sie_release_gating(struct mpc_sie_dev_t * dev)
{
    struct mpc_sie_reg_map_t * p_mpc = (struct mpc_sie_reg_map_t *) dev->cfg->base;

    p_mpc->ctrl &= ~MPC_SIE300_CTRL_GATE_REQ;
}
