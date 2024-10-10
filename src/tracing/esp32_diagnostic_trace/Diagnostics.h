/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <lib/core/TLVCircularBuffer.h>

namespace chip {
namespace Tracing {

namespace Diagnostics {
using namespace chip::TLV;

enum class DIAGNOSTICS_TAG
{
    METRIC     = 0,
    TRACE      = 1,
    COUNTER    = 2,
    LABEL      = 3,
    GROUP      = 4,
    VALUE      = 5,
    TIMESTAMP  = 6
};

class DiagnosticEntry {
public:
    virtual ~DiagnosticEntry() = default;
    virtual CHIP_ERROR Encode(CircularTLVWriter &writer) = 0;
};

template<typename T>
class Metric : public DiagnosticEntry {
public:
    Metric(const char* label, T value, uint32_t timestamp)
        : label_(label), value_(value), timestamp_(timestamp) {}

    Metric() {}

    const char* GetLabel() const { return label_; }
    T GetValue() const { return value_; }
    uint32_t GetTimestamp() const { return timestamp_; }

    CHIP_ERROR Encode(CircularTLVWriter &writer) override {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLVType metricContainer;
        err = writer.StartContainer(ContextTag(DIAGNOSTICS_TAG::METRIC), kTLVType_Structure, metricContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to start TLV container for metric : %s", ErrorStr(err)));

        // TIMESTAMP
        err = writer.Put(ContextTag(DIAGNOSTICS_TAG::TIMESTAMP), timestamp_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write TIMESTAMP for METRIC : %s", ErrorStr(err)));

        // LABEL
        err = writer.PutString(ContextTag(DIAGNOSTICS_TAG::LABEL), label_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write LABEL for METRIC : %s", ErrorStr(err)));

        // VALUE
        err = writer.Put(ContextTag(DIAGNOSTICS_TAG::VALUE), value_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write VALUE for METRIC : %s", ErrorStr(err)));

        ChipLogProgress(DeviceLayer, "Metric Value written to storage successfully. label: %s\n", label_);
        err = writer.EndContainer(metricContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to end TLV container for metric : %s", ErrorStr(err)));
        return err;
    }

private:
    const char* label_;
    T value_;
    uint32_t timestamp_;
};

class Trace : public DiagnosticEntry {
public:
    Trace(const char* label, const char* group,  uint32_t timestamp)
        : label_(label), group_(group), timestamp_(timestamp) {}

    Trace() {}

    const char* GetLabel() const { return label_; }
    uint32_t GetTimestamp() const { return timestamp_; }
    const char* GetGroup() const { return group_; }

    CHIP_ERROR Encode(CircularTLVWriter &writer) override {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLVType traceContainer;
        err = writer.StartContainer(ContextTag(DIAGNOSTICS_TAG::TRACE), kTLVType_Structure, traceContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to start TLV container for Trace: %s", ErrorStr(err)));

        // TIMESTAMP
        err = writer.Put(ContextTag(DIAGNOSTICS_TAG::TIMESTAMP), timestamp_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write TIMESTAMP for METRIC : %s", ErrorStr(err)));

        // GROUP
        err = writer.PutString(ContextTag(DIAGNOSTICS_TAG::GROUP), group_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write GROUP for TRACE : %s", ErrorStr(err)));

        // LABEL
        err = writer.PutString(ContextTag(DIAGNOSTICS_TAG::LABEL), label_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write LABEL for TRACE : %s", ErrorStr(err)));

        ChipLogProgress(DeviceLayer, "Trace Value written to storage successfully. label: %s value: %s\n", label_, group_);
        err = writer.EndContainer(traceContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to end TLV container for Trace : %s", ErrorStr(err)));
        return err;
    }

private:
    const char* label_;
    const char* group_;
    uint32_t timestamp_;
};

class Counter : public DiagnosticEntry {
public:
    Counter(const char* label, uint32_t count, uint32_t timestamp)
        : label_(label), count_(count), timestamp_(timestamp) {}

    Counter() {}

    uint32_t GetCount() const { return count_; }

    uint32_t GetTimestamp() const { return timestamp_; }

    CHIP_ERROR Encode(CircularTLVWriter &writer) override {
        CHIP_ERROR err = CHIP_NO_ERROR;
        TLVType counterContainer;
        err = writer.StartContainer(ContextTag(DIAGNOSTICS_TAG::COUNTER), kTLVType_Structure, counterContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to start TLV container for Counter: %s", ErrorStr(err)));

        // TIMESTAMP
        err = writer.Put(ContextTag(DIAGNOSTICS_TAG::TIMESTAMP), timestamp_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write TIMESTAMP for COUNTER : %s", ErrorStr(err)));

        // LABEL
        err = writer.PutString(ContextTag(DIAGNOSTICS_TAG::LABEL), label_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write LABEL for COUNTER : %s", ErrorStr(err)));

        // COUNT
        err = writer.Put(ContextTag(DIAGNOSTICS_TAG::COUNTER), count_);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to write VALUE for COUNTER : %s", ErrorStr(err)));

        ChipLogProgress(DeviceLayer, "Counter Value written to storage successfully  label: %s count: %ld\n", label_, count_);
        err = writer.EndContainer(counterContainer);
        VerifyOrReturnError(err == CHIP_NO_ERROR, err,
                            ChipLogError(DeviceLayer, "Failed to end TLV container for counter : %s", ErrorStr(err)));
        return err;
    }

private:
    const char* label_;
    uint32_t count_;
    uint32_t timestamp_;
};

class DiagnosticStorageInterface {
public:
    virtual ~DiagnosticStorageInterface() = default;

    virtual CHIP_ERROR Store(DiagnosticEntry& diagnostic) = 0;

    virtual CHIP_ERROR Retrieve(MutableByteSpan &payload) = 0;
};

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
