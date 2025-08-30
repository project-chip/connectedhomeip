/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#pragma once

#include <cstddef>
#include <cstdint>
namespace CommodityTariffConsts {
static constexpr size_t kDefaultStringValuesMaxBufLength = 128u;
static constexpr size_t kDefaultListAttrMaxLength        = 672u;
constexpr uint16_t kMaxCurrencyValue                     = 999; // From spec

static constexpr size_t kTariffInfoMaxLabelLength      = kDefaultStringValuesMaxBufLength;
static constexpr size_t kTariffInfoMaxProviderLength   = kDefaultStringValuesMaxBufLength;
static constexpr size_t kTariffComponentMaxLabelLength = kDefaultStringValuesMaxBufLength;

static constexpr size_t kDayEntriesAttrMaxLength       = kDefaultListAttrMaxLength;
static constexpr size_t kDayPatternsAttrMaxLength      = 28;
static constexpr size_t kTariffComponentsAttrMaxLength = kDefaultListAttrMaxLength;
static constexpr size_t kTariffPeriodsAttrMaxLength    = kDefaultListAttrMaxLength;

static constexpr size_t kCalendarPeriodsAttrMaxLength = 4;
static constexpr size_t kIndividualDaysAttrMaxLength  = 50;

static constexpr size_t kCalendarPeriodItemMaxDayPatternIDs = 7;
static constexpr size_t kDayStructItemMaxDayEntryIDs        = 96;
static constexpr size_t kDayPatternItemMaxDayEntryIDs       = kDayStructItemMaxDayEntryIDs;
static constexpr size_t kTariffPeriodItemMaxIDs             = 20;
static constexpr uint16_t kDayEntryDurationLimit            = 1500;
static constexpr uint8_t kFullWeekMask                      = 0x7f;
} // namespace CommodityTariffConsts
