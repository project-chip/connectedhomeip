/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

class DimmableLEDIf
{
public:
    /* set to ON or OFF */
    virtual void Set(bool state) = 0;
    /* Get On/Off state */
    virtual bool Get(void) = 0;
    /* change light level */
    virtual void SetLevel(uint8_t level) = 0;
    /* Get current level*/
    virtual uint8_t GetLevel(void)    = 0;
    virtual uint8_t GetMaxLevel(void) = 0;
    virtual uint8_t GetMinLevel(void) = 0;
};
