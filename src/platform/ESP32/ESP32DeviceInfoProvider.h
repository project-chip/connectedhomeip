/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <lib/support/EnforceFormat.h>
#include <platform/DeviceInfoProvider.h>

namespace chip {
namespace DeviceLayer {

class ESP32DeviceInfoProvider : public DeviceInfoProvider
{
public:
    ESP32DeviceInfoProvider() = default;
    ~ESP32DeviceInfoProvider() override {}

    // Iterators
    FixedLabelIterator * IterateFixedLabel(EndpointId endpoint) override;
    UserLabelIterator * IterateUserLabel(EndpointId endpoint) override;
    SupportedLocalesIterator * IterateSupportedLocales() override;
    SupportedCalendarTypesIterator * IterateSupportedCalendarTypes() override;

    static ESP32DeviceInfoProvider & GetDefaultInstance();

    struct FixedLabelEntry
    {
        EndpointId endpointId;
        CharSpan label;
        CharSpan value;
    };

    /**
     * @brief API to set the supported calendar types
     *
     * @param[in] supportedCalendarTypes Span of type chip::app::Clusters::TimeFormatLocalization::CalendarTypeEnum
     *              containing the supported calendar types. The underlying data must remain allocated throughout
     *              the lifetime of the device, as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetSupportedCalendarTypes(const Span<CalendarType> & supportedCalendarTypes)
    {
        VerifyOrReturnError(!supportedCalendarTypes.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mSupportedCalendarTypes = supportedCalendarTypes;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief API to set the supported Locales
     *
     * @param[in] supportedLocales Span of type chip::CharSpan containing the supported locales.
     *              The underlying data must remain allocated throughout the lifetime of the device,
     *              as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetSupportedLocales(const Span<CharSpan> & supportedLocales)
    {
        VerifyOrReturnError(!supportedLocales.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mSupportedLocales = supportedLocales;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief API to set the fixed labels
     *
     * @param[in] fixedLabels Span of type chip::DeviceLayer::ESP32DeviceInfoProvider::FixedLabelEntry
     *              containing the fixed labels for supported endpoints.
     *              The underlying data must remain allocated throughout the lifetime of the device,
     *              as the API does not make a copy.
     *
     * @return CHIP_ERROR indicating the success or failure of the operation.
     */
    CHIP_ERROR SetFixedLabels(const Span<FixedLabelEntry> & supportedFixedLabels)
    {
        VerifyOrReturnError(!supportedFixedLabels.empty(), CHIP_ERROR_INVALID_ARGUMENT);
        mFixedLabels = supportedFixedLabels;
        return CHIP_NO_ERROR;
    }

protected:
    class FixedLabelIteratorImpl : public FixedLabelIterator
    {
    public:
        FixedLabelIteratorImpl(EndpointId endpoint, const Span<FixedLabelEntry> & labels);
        size_t Count() override;
        bool Next(FixedLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        EndpointId mEndpoint = 0;
        size_t mIndex        = 0;
        Span<FixedLabelEntry> mLabels;
    };

    class UserLabelIteratorImpl : public UserLabelIterator
    {
    public:
        UserLabelIteratorImpl(ESP32DeviceInfoProvider & provider, EndpointId endpoint);
        size_t Count() override { return mTotal; }
        bool Next(UserLabelType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        ESP32DeviceInfoProvider & mProvider;
        EndpointId mEndpoint = 0;
        size_t mIndex        = 0;
        size_t mTotal        = 0;
        char mUserLabelNameBuf[kMaxLabelNameLength + 1];
        char mUserLabelValueBuf[kMaxLabelValueLength + 1];
    };

    class SupportedLocalesIteratorImpl : public SupportedLocalesIterator
    {
    public:
        SupportedLocalesIteratorImpl(const Span<CharSpan> & locales);
        size_t Count() override;
        bool Next(CharSpan & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        size_t mIndex = 0;
        Span<CharSpan> mLocales;
    };

    class SupportedCalendarTypesIteratorImpl : public SupportedCalendarTypesIterator
    {
    public:
        SupportedCalendarTypesIteratorImpl(const Span<CalendarType> & calendarTypes);
        size_t Count() override;
        bool Next(CalendarType & output) override;
        void Release() override { chip::Platform::Delete(this); }

    private:
        size_t mIndex = 0;
        Span<CalendarType> mCalendarTypes;
    };

    CHIP_ERROR SetUserLabelLength(EndpointId endpoint, size_t val) override;
    CHIP_ERROR GetUserLabelLength(EndpointId endpoint, size_t & val) override;
    CHIP_ERROR SetUserLabelAt(EndpointId endpoint, size_t index, const UserLabelType & userLabel) override;
    CHIP_ERROR DeleteUserLabelAt(EndpointId endpoint, size_t index) override;

private:
    static constexpr size_t UserLabelTLVMaxSize() { return TLV::EstimateStructOverhead(kMaxLabelNameLength, kMaxLabelValueLength); }

    Span<CalendarType> mSupportedCalendarTypes;
    Span<CharSpan> mSupportedLocales;
    Span<FixedLabelEntry> mFixedLabels;
};

} // namespace DeviceLayer
} // namespace chip
