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

static constexpr size_t kPersistentArraySerializedMinSize =
    7 + 200; // ContainerTag(1) + CountTag(1) + Count(2) + ListTag(1) + EndListTag(1) + EndContainerTag(1) + 1
static constexpr size_t kPersistentEntrySerializedMinSize = 2; // EntryStartTag(1) + EntryEndTag(1)

template <size_t kMaxListSize, size_t kMaxSerializedEntrySize, typename EntryType>
struct PersistentArray : public PersistentData<kPersistentArraySerializedMinSize +
                                               kMaxListSize *(kPersistentEntrySerializedMinSize + kMaxSerializedEntrySize)>
{
    PersistentArray(PersistentStorageDelegate * storage, uint16_t count = 0) :
        PersistentData<kPersistentArraySerializedMinSize +
                       kMaxListSize *(kPersistentEntrySerializedMinSize + kMaxSerializedEntrySize)>(storage),
        mLimit(kMaxListSize), mCount(count)
    {}

    EntryType & At(size_t index) { return (index < this->mLimit) ? mEntries[index] : mEntries[0]; }
    size_t Limit() { return this->mLimit; }
    uint16_t Count() { return this->mCount; }

    void Clear() override
    {
        // Clear entries
        for (size_t i = 0; i < this->mLimit; ++i)
        {
            this->ClearEntry(mEntries[i]);
        }
        this->mCount = 0;
    }

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
        CHIP_ERROR err = this->Load();
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (size_t i = 0; i < this->mCount; ++i)
        {
            EntryType & e = mEntries[i];
            if (Compare(e, value))
            {
                // Existing value, index must match
                VerifyOrReturnError(i == index, CHIP_ERROR_DUPLICATE_KEY_ID);
                EntryType old;
                ReturnErrorOnFailure(Copy(old, e));
                ReturnErrorOnFailure(Copy(e, value));
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
            Copy(old, mEntries[index]);
            ReturnErrorOnFailure(Copy(mEntries[index], value));
            ReturnErrorOnFailure(this->Save());
            OnEntryRemoved(old);
        }
        else
        {
            // Insert last
            VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_ARGUMENT);
            VerifyOrReturnError(index == this->mCount, CHIP_ERROR_INVALID_ARGUMENT);
            ReturnErrorOnFailure(Copy(mEntries[this->mCount++], value));
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
    CHIP_ERROR Add(const EntryType & value, bool do_override = true)
    {
        CHIP_ERROR err = this->Load();
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (size_t i = 0; i < this->mCount; ++i)
        {
            EntryType & e = mEntries[i];
            if (Compare(e, value))
            {
                // Already registered
                VerifyOrReturnError(do_override, CHIP_ERROR_DUPLICATE_KEY_ID);
                EntryType old;
                ReturnErrorOnFailure(Copy(old, e));
                ReturnErrorOnFailure(Copy(e, value));
                ReturnErrorOnFailure(this->Save());
                OnEntryModified(old, value);
                return CHIP_NO_ERROR;
            }
        }
        // Insert last
        VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_LIST_LENGTH);
        ReturnErrorOnFailure(Copy(mEntries[this->mCount++], value));
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
        ReturnErrorOnFailure(this->Load());
        VerifyOrReturnError(index < this->mLimit, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(index < this->mCount, CHIP_ERROR_NOT_FOUND);
        ReturnErrorOnFailure(Copy(value, mEntries[index]));
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
     * @param index [out] Index where the entry was found. Undefined on error.
     * @param do_create When true, if no matching entry is found, the entire value will be stored
     *                  as a new entry in the table.
     * @return CHIP_NO_ERROR on success,
     *         CHIP_ERROR_NOT_FOUND if no entry matches the given input.
     */
    CHIP_ERROR Find(EntryType & value, size_t & index, bool do_create = false)
    {
        index          = 0;
        CHIP_ERROR err = this->Load();
        VerifyOrReturnError(CHIP_ERROR_NOT_FOUND == err || CHIP_NO_ERROR == err, err);

        // Check existing
        for (index = 0; index < this->mCount; ++index)
        {
            EntryType & e = mEntries[index];
            if (Compare(e, value))
            {
                ReturnErrorOnFailure(Copy(value, e));
                return CHIP_NO_ERROR;
            }
        }
        // Insert last ?
        VerifyOrReturnError(do_create, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(this->mCount < this->mLimit, CHIP_ERROR_INVALID_LIST_LENGTH);
        ReturnErrorOnFailure(Copy(mEntries[this->mCount++], value));
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
        ReturnErrorOnFailure(this->Load());
        VerifyOrReturnError(index < this->mLimit, CHIP_ERROR_NOT_FOUND);
        VerifyOrReturnError(index < this->mCount, CHIP_ERROR_NOT_FOUND);
        EntryType old;
        Copy(old, mEntries[index]);
        this->mCount--;
        for (size_t i = index; i < this->mCount; ++i)
        {
            Copy(mEntries[i], mEntries[i + 1]);
        }
        ClearEntry(mEntries[this->mCount]);
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
        ReturnErrorOnFailure(this->Load());
        for (size_t i = 0; i < this->mCount; ++i)
        {
            if (Compare(mEntries[i], value))
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
        VerifyOrReturnError(CHIP_NO_ERROR == this->Load(), true);
        return 0 == this->mCount;
    }

protected:
    static constexpr TLV::Tag CountTag() { return TLV::ContextTag(1); }
    static constexpr TLV::Tag ListTag() { return TLV::ContextTag(2); }

    CHIP_ERROR Serialize(TLV::TLVWriter & writer) const override
    {
        TLV::TLVType container, list, entry;
        ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, container));

        // Count
        ReturnErrorOnFailure(writer.Put(CountTag(), this->mCount));

        // Entries
        ReturnErrorOnFailure(writer.StartContainer(ListTag(), TLV::kTLVType_Array, list));
        for (size_t i = 0; (i < this->mCount) && (i < this->mLimit); ++i)
        {
            ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Structure, entry));
            ReturnErrorOnFailure(Serialize(writer, mEntries[i]));
            ReturnErrorOnFailure(writer.EndContainer(entry));
        }
        ReturnErrorOnFailure(writer.EndContainer(list));

        CHIP_ERROR err = writer.EndContainer(container);
        return err;
    }

    CHIP_ERROR Deserialize(TLV::TLVReader & reader) override
    {
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Structure, TLV::AnonymousTag()));

        TLV::TLVType container, list, entry;
        ReturnErrorOnFailure(reader.EnterContainer(container));

        // Count
        ReturnErrorOnFailure(reader.Next(CountTag()));
        ReturnErrorOnFailure(reader.Get(this->mCount));

        // Entries
        ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, ListTag()));
        ReturnErrorOnFailure(reader.EnterContainer(list));
        for (size_t i = 0; (i < this->mCount) && (i < this->mLimit); ++i)
        {
            ReturnErrorOnFailure(reader.Next(TLV::AnonymousTag()));
            VerifyOrReturnError(TLV::kTLVType_Structure == reader.GetType(), CHIP_ERROR_INTERNAL);
            ReturnErrorOnFailure(reader.EnterContainer(entry));
            ReturnErrorOnFailure(Deserialize(reader, mEntries[i]));
            ReturnErrorOnFailure(reader.ExitContainer(entry));
        }
        ReturnErrorOnFailure(reader.ExitContainer(list));

        return reader.ExitContainer(container);
    }

    virtual void ClearEntry(EntryType & entry) = 0;
    virtual bool Compare(const EntryType & a, const EntryType & b) const { return a == b; }
    virtual CHIP_ERROR Copy(EntryType & dest, const EntryType & src) const
    {
        dest = src;
        return CHIP_NO_ERROR;
    }

    virtual CHIP_ERROR UpdateKey(StorageKeyName & key) const override = 0;
    virtual CHIP_ERROR Serialize(TLV::TLVWriter & writer, const EntryType & entry) const = 0;
    virtual CHIP_ERROR Deserialize(TLV::TLVReader & reader, EntryType & entry)           = 0;
    virtual void OnEntryAdded(const EntryType & entry) {}
    virtual void OnEntryRemoved(const EntryType & entry) {}
    virtual void OnEntryModified(const EntryType & old_value, const EntryType & new_value) {}

private:
    EntryType mEntries[kMaxListSize];
    const size_t mLimit = 0;
    uint16_t mCount     = 0;
};

} // namespace chip
