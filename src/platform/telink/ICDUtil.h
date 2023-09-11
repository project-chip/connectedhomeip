/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/ReadHandler.h>

class ICDUtil : public chip::app::ReadHandler::ApplicationCallback
{
    CHIP_ERROR OnSubscriptionRequested(chip::app::ReadHandler & aReadHandler,
                                       chip::Transport::SecureSession & aSecureSession) override;
    friend ICDUtil & GetICDUtil();
    static ICDUtil sICDUtil;
};

inline ICDUtil & GetICDUtil()
{
    return ICDUtil::sICDUtil;
}
