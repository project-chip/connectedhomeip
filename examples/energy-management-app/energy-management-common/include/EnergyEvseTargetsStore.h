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
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/CommonIterator.h>
#include <lib/support/Pool.h>

#include <app-common/zap-generated/cluster-objects.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace EnergyEvse {

class EvseTargetsDelegate
{

public:
    struct EvseChargingTarget
    {
        uint16_t targetTimeMinutesPastMidnight;
        Optional<chip::Percent> targetSoC;
        Optional<int64_t> addedEnergy;
    };

    class EvseTargetEntry
    {
    public:
        chip::BitMask<TargetDayOfWeekBitmap> dayOfWeekMap;
        std::vector<EvseChargingTarget> dailyChargingTargets;
    };

    CHIP_ERROR Init(PersistentStorageDelegate * targetStore);
    CHIP_ERROR IncreaseEntryCount();
    CHIP_ERROR DecreaseEntryCount();
    CHIP_ERROR UpdateEntryCount(bool increase);
    CHIP_ERROR LoadCounter(size_t & count, size_t & targetsSize);
    CHIP_ERROR Load(std::vector<EvseTargetEntry> & targetEntryVector, size_t & targetsSize);
    CHIP_ERROR SerializeToTlv(TLV::TLVWriter & writer, const std::vector<EvseTargetEntry> & targetEntryVector);
    CHIP_ERROR StoreEntry(const EvseTargetEntry & entry);

    CHIP_ERROR CreateEntry(EvseTargetEntry &);
    CHIP_ERROR DeleteEntry(EvseTargetEntry &);
    CHIP_ERROR UpdateEntry(EvseTargetEntry &);

    /**
     * @brief   Copies a ChargingTargetSchedule into our mTargets
     *
     * @param [in] an entry from the SetTargets list containing:
     *             dayOfWeekForSequence and chargingTargets (list)
     *
     * This routine scans the existing mTargets to see if we have a day of week
     * set that matches the new target dayOfWeek bits. If there is an existing
     * matching day then it replaces the days existing targets with the new entry
     */
    CHIP_ERROR
    CopyTarget(const Structs::ChargingTargetScheduleStruct::DecodableType &);

    /**
     *  @brief   This deletes all targets and resets the list to empty
     */
    CHIP_ERROR ClearTargets();

    using EvseTargetIterator = CommonIterator<EvseTargetEntry>;

protected:
    enum class TargetEntryTag : uint8_t
    {
        kTargetEntry           = 1,
        kDayOfWeek             = 2,
        kChargingTargetsList   = 3,
        kChargingTargetsStruct = 4,
        kTargetTime            = 5,
        kTargetSoC             = 6,
        kAddedEnergy           = 7,
    };

    enum class CounterTag : uint8_t
    {
        kCount = 1,
        kSize  = 2,
    };

    class EvseTargetIteratorImpl : public EvseTargetIterator
    {
    public:
        EvseTargetIteratorImpl(EvseTargetsDelegate & aDelegate) : mDelegate(aDelegate)
        {
            mTargetEntryIndex = 0;
            mTargetEntryVector.clear();
        }
        size_t Count() override;
        bool Next(EvseTargetEntry & entry) override;
        void Release() override;

    private:
        EvseTargetsDelegate & mDelegate;
        size_t mTargetEntryIndex = 0;
        std::vector<EvseTargetEntry> mTargetEntryVector;
    };

    EvseTargetIteratorImpl * GetTargetsIterator();

    static constexpr size_t MaxTargetEntryCounterSize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(size_t), sizeof(size_t));
    }

    static constexpr size_t MaxTargetEntrySize()
    {
        // All the fields added together
        return TLV::EstimateStructOverhead(sizeof(chip::BitMask<TargetDayOfWeekBitmap>)) +
            kMaxTargetsPerDay *
            TLV::EstimateStructOverhead(sizeof(uint16_t), sizeof(Optional<chip::Percent>), sizeof(Optional<int64_t>));
    }

private:
    static constexpr uint8_t kMaxNumberOfDays  = 7;
    static constexpr uint8_t kMaxTargetsPerDay = 10;
    static constexpr size_t kIteratorsMax      = 1;

    // The array itself has a control byte and an end-of-array marker.
    static constexpr size_t kArrayOverHead = 2;

    ObjectPool<EvseTargetIteratorImpl, kIteratorsMax> mEvseTargetsIterators;
    PersistentStorageDelegate * mpTargetStore = nullptr;
};

#if 0
class EvseTargetsDelegate
{
public:
    /**
     * Used for decoding EVSETarget entries.
     *
     * Typically used temporarily on the stack to decode:
     * - source: TLV
     * - staging: generated cluster level code
     * - destination: EVSETarget entry
     */
    class DecodableEntry
    {
        using Entry        = EvseTarget::EnergyEvseTargetEntry;
        using StagingEntry = Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::DecodableType;

    public:
        DecodableEntry() = default;

        /**
         * Reader decodes into a staging entry, which is then unstaged
         * into a member entry.
         */
        CHIP_ERROR Decode(TLV::TLVReader & reader);

        Entry & GetEntry() { return mEntry; }

        const Entry & GetEntry() const { return mEntry; }

    private:
        CHIP_ERROR Unstage();
        Entry mEntry;

        StagingEntry mStagingEntry;
    };

    /**
     * Used for encoding EVSETarget entries.
     *
     * Typically used temporarily on the stack to encode:
     * - source: EVSETarget
     * - staging: generated cluster level code
     * - destination: TLV
     */
    class EncodableEntry
    {
        using Entry        = EvseTarget::EnergyEvseTargetEntry;
        using StagingEntry = Clusters::EnergyEvse::Structs::ChargingTargetScheduleStruct::Type;

    public:
        EncodableEntry(const Entry & entry) : mEntry(entry) {}

        /**
         * Constructor-provided entry is staged into a staging entry,
         * which is then encoded into a writer.
         */
        CHIP_ERROR EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag) const;

        /**
         * Constructor-provided entry is staged into a staging entry,
         * which is then encoded into a writer.
         */
        CHIP_ERROR EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const;

        /**
         * Constructor-provided entry is staged into a staging entry.
         */
        CHIP_ERROR Stage() const;

        StagingEntry & GetStagingEntry() { return mStagingEntry; }

        const StagingEntry & GetStagingEntry() const { return mStagingEntry; }

    private:
        const Entry & mEntry;

        mutable StagingEntry mStagingEntry;
    };

    ~EvseTargetsDelegate() = default;

    CHIP_ERROR Init(PersistentStorageDelegate & persistentStorage);
};
#endif
class EvseTargetsStore
{
public:
    EvseTargetsStore() {}
    ~EvseTargetsStore() { Finish(); }

    // Non-copyable
    EvseTargetsStore(EvseTargetsStore const &) = delete;
    void operator=(EvseTargetsStore const &)   = delete;

    /**
     * @brief Initialize the EVSE Targets store to map to a given storage delegate.
     *
     * @param storage Pointer to persistent storage delegate to use. Must outlive this instance.
     * @retval CHIP_NO_ERROR on success
     * @retval CHIP_ERROR_INCORRECT_STATE if already initialized
     */
    CHIP_ERROR Init(PersistentStorageDelegate * storage)
    {
        VerifyOrReturnError(mStorage == nullptr, CHIP_ERROR_INCORRECT_STATE);
        mStorage = storage;
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Finalize the EVSE Targets store, so that subsequent operations fail
     */
    void Finish()
    {
        VerifyOrReturn(mStorage != nullptr);
        mStorage = nullptr;
    }

    CHIP_ERROR LoadTargetsFromStore();

protected:
    static constexpr uint8_t kEnergyEvseTargetsTLVMaxSize = 240; // TODO
    PersistentStorageDelegate * mStorage                  = nullptr;
};

} // namespace EnergyEvse
} // namespace Clusters
} // namespace app
} // namespace chip
