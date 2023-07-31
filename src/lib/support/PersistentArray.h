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

#include <lib/support/PersistentData.h>

namespace chip {

enum class PersistentTags : uint8_t
{
    kCount = 1,
    kList  = 2,
};

static constexpr size_t kPersistentArraySerializedMinSize =
    7; // ContainerTag(1) + CountTag(1) + Count(2) + ListTag(1) + EndListTag(1) + EndContainerTag(1)

template <size_t kMaxListSize, size_t kMaxSerializedEntrySize, typename EntryType>
struct PersistentArray : public PersistentData<kPersistentArraySerializedMinSize + kMaxListSize * kMaxSerializedEntrySize>
{

    PersistentArray(PersistentStorageDelegate * storage) :
        PersistentData<kPersistentArraySerializedMinSize + kMaxListSize * kMaxSerializedEntrySize>(storage), mLimit(kMaxListSize)
    {}

    size_t Limit() { return this->mLimit; }
    uint16_t Count() { return this->mCount; }

    /**
     * Stores an entry at a given position. This method does not validate the order
     * in which the entries are stored. New entries must be inserted with index == count,
     * if index < count, the existing entry is replaced, generating an implicit remove of
     * the existing entry.
     * The Compare() method is used to ensure that the entry is unique. If the new entry matches
     * an existing entry, the index must also match, otherwise CHIP_ERROR_DUPLICATE_KEY_ID is
     * returned.
     * @param index Zero-based position within the table.
     * @param entry Value to be store in the table.
     * @return CHIP_NO_ERROR on success. CHIP_ERROR_DUPLICATE_KEY_ID if the entry is already
     * stored in different index. CHIP_ERROR_INVALID_ARGUMENT if index >= limit.
     */
    CHIP_ERROR Set(size_t index, const EntryType & value)
    {
        CHIP_ERROR err = this->Load(true);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (size_t i = 0; i < this->mCount; ++i)
        {
            EntryType & e = mEntries[i];
            if (e.Compare(value))
            {
                // Existing value, index must match
                VerifyOrReturnError(i == index, CHIP_ERROR_DUPLICATE_KEY_ID);
                EntryType old;
                ReturnErrorOnFailure(old.Copy(e));
                ReturnErrorOnFailure(e.Copy(value));
                ReturnErrorOnFailure(this->Save());
                OnEntryModified(old, value);
                return CHIP_NO_ERROR;
            }
        }

        // New value
        if (index < this->mCount)
        {
            // Override (implicit remove)
            EntryType old;
            old.Copy(mEntries[index]);
            ReturnErrorOnFailure(mEntries[index].Copy(value));
            ReturnErrorOnFailure(this->Save());
            OnEntryRemoved(old);
        }
        else
        {
            // Insert last
            VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(mEntries[this->mCount++].Copy(value));
            ReturnErrorOnFailure(this->Save());
        }
        OnEntryAdded(value);
        return CHIP_NO_ERROR;
    }

    /**
     * Stores a new entry. The Compare() method is used to ensure that the entry is unique.
     * If the new entry matches an existing entry, do_override must be true, otherwise
     * CHIP_ERROR_DUPLICATE_KEY_ID is returned.
     * @param index Zero-based position within the table.
     * @param entry Value to be store in the table.
     * @return CHIP_NO_ERROR on success. CHIP_ERROR_DUPLICATE_KEY_ID is the entry is
     * already stored in the table. CHIP_ERROR_INVALID_ARGUMENT if index >= limit.
     */
    CHIP_ERROR Set(const EntryType & value, bool do_override = true)
    {
        CHIP_ERROR err = this->Load(true);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (size_t i = 0; i < this->mCount; ++i)
        {
            EntryType & e = mEntries[i];
            if (e.Compare(value))
            {
                // Already registered
                VerifyOrReturnError(do_override, CHIP_ERROR_DUPLICATE_KEY_ID);
                EntryType old;
                ReturnErrorOnFailure(old.Copy(e));
                ReturnErrorOnFailure(e.Copy(value));
                ReturnErrorOnFailure(this->Save());
                OnEntryModified(old, value);
                return CHIP_NO_ERROR;
            }
        }
        // Insert last
        VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_LIST_LENGTH);
        ReturnErrorOnFailure(mEntries[this->mCount++].Copy(value));
        ReturnErrorOnFailure(this->Save());
        OnEntryAdded(value);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Returns the stored entry at a given index. Is up to the implementation
     *        to handle how the de-serialized values are copied into the given output.
     *        The output should be considered valid only as the lifetime of the
     *        PersistentArray itself, and no other Get, Set or Remove operation is performed.
     * @param index Zero-based position within the table.
     * @param entry [out] On success, contains the value matching the given index.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if index is greater than the index of the last entry on the table.
     */
    CHIP_ERROR Get(size_t index, EntryType & value)
    {
        ReturnErrorOnFailure(this->Load(true));
        VerifyOrReturnError(index < this->mLimit, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(index < this->mCount, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(value.Copy(mEntries[index]));
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Returns the value matching the given entry. The Compare() method
     *        is used to identify the matching entry within the table.
     *        Is up to the implementation to handle how the de-serialized values
     *        are copied into the given output.
     *        The output should be considered valid only as the lifetime of the
     *        PersistentArray itself, and no other Get, Set or Remove operation is performed.
     *
     * @param value [in, out] Used as an input for comparison, and output for the rest of the values.
     * @param do_create When true, if no matching entry is found, the entire value will be stored
     *                  as a new entry in the table.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if no entry matches the given input.
     */
    CHIP_ERROR Get(EntryType & value, bool do_create = false)
    {
        CHIP_ERROR err = this->Load(true);
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (size_t i = 0; i < this->mCount; ++i)
        {
            EntryType & e = mEntries[i];
            if (e.Compare(value))
            {
                ReturnErrorOnFailure(value.Copy(e));
                return CHIP_NO_ERROR;
            }
        }
        // Insert last ?
        VerifyOrReturnError(do_create, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_LIST_LENGTH);
        ReturnErrorOnFailure(mEntries[this->mCount++].Copy(value));
        return this->Save();
    }

    /**
     * @brief Removes the stored entry at a given index. Remaining entries are
     *        shifted one position down.
     * @param index Zero-based position within the table.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if index is greater than the index of the last entry on the table.
     */
    CHIP_ERROR Remove(size_t index)
    {
        ReturnErrorOnFailure(this->Load(true));
        VerifyOrReturnError(index < this->mLimit, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(index < this->mCount, CHIP_ERROR_NOT_FOUND);
        EntryType old;
        old.Copy(mEntries[index]);
        this->mCount--;
        for (size_t i = index; i < this->mCount; ++i)
        {
            mEntries[i].Copy(mEntries[i + 1]);
        }
        ReturnErrorOnFailure(this->Save());
        OnEntryRemoved(old);
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Removes the value matching the given entry. The Compare() method
     *        is used to identify the matching entry within the table.
     * @param entry Identifies the value to be removed.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if no entry matches the given input.
     */
    CHIP_ERROR Remove(const EntryType & value)
    {
        ReturnErrorOnFailure(this->Load(true));
        for (size_t i = 0; i < this->mCount; ++i)
        {
            if (mEntries[i].Compare(value))
            {
                return Remove(i);
            }
        }
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Removes all the entries in the table.
     * @return CHIP_NO_ERROR on success.
     */
    CHIP_ERROR RemoveAll()
    {
        ReturnErrorOnFailure(this->Load());
        for (uint16_t i = 0; i < mLimit; ++i)
        {
            OnEntryRemoved(mEntries[i]);
        }
        mCount = 0;
        ReturnErrorOnFailure(this->Save());
        return CHIP_NO_ERROR;
    }

    /**
     * @brief Check if the table is empty
     * @return True when there is no entry in the table. False if there is at least one
     */
    bool IsEmpty()
    {
        VerifyOrReturnError(CHIP_NO_ERROR == this->Load(true), true);
        return 0 == this->mCount;
    }

protected:
    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container, list;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        // Count
        ReturnErrorOnFailure(writer.Put(TLV::ContextTag(PersistentTags::kCount), this->mCount));

        // Entries
        ReturnErrorOnFailure(writer.StartContainer(TLV::ContextTag(PersistentTags::kList), TLV::kTLVType_Array, list));
        for (size_t i = 0; (i < this->mCount) && (i < this->mLimit); ++i)
        {
            ReturnErrorOnFailure(mEntries[i].Serialize(writer));
        }
        ReturnErrorOnFailure(writer.EndContainer(list));

        CHIP_ERROR err = writer.EndContainer(container);
        return err;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container, list;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // Count
        ReturnErrorOnFailure(reader.Next(TLV::ContextTag(PersistentTags::kCount)));
        ReturnErrorOnFailure(reader.Get(this->mCount));

        // Entries
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::ContextTag(PersistentTags::kList)));
        ReturnErrorOnFailure(reader.EnterContainer(list));
        for (size_t i = 0; (i < this->mCount) && (i < this->mLimit); ++i)
        {
            ReturnErrorOnFailure(mEntries[i].Deserialize(reader));
        }
        ReturnErrorOnFailure(reader.ExitContainer(list));

        return reader.ExitContainer(container);
    }

    virtual void OnEntryAdded(const EntryType & entry) {}
    virtual void OnEntryRemoved(const EntryType & entry) {}
    virtual void OnEntryModified(const EntryType & old_value, const EntryType & new_value) {}

private:
    // EntryType & At(size_t index) { return (index < this->mLimit) ? mEntries[index] : mEntries[0]; }
    EntryType mEntries[kMaxListSize];
    const size_t mLimit = 0;
    uint16_t mCount     = 0;
};

} // namespace chip
