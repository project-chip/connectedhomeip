/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#import <Foundation/Foundation.h>

#include <app/ConcreteAttributePath.h>
#include <app/EventLoggingTypes.h>
#include <lib/core/CHIPError.h>
#include <lib/core/TLV.h>

NS_ASSUME_NONNULL_BEGIN

id _Nullable MTRDecodeEventPayload(const chip::app::ConcreteEventPath & aPath, chip::TLV::TLVReader & aReader, CHIP_ERROR * aError);

NS_ASSUME_NONNULL_END
