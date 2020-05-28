/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Storage interface implementation
 *
 */

#include <cstddef>
#include <string>
#include <gdbm.h>

#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <platform/Linux/ChipStorage.h>
#include <platform/Linux/ChipStorageImpl.h>

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace ::chip::Platform::PersistedStorage;

class GdbmStorage : public ChipMutableStorage {
private:
    GdbmStorage(const char *name) : mName(name) { }
    friend class PosixStorage;

public:
    ~GdbmStorage() override {
        if (state == DB_READY) gdbm_close(mDatabase);
    }

    const char *GetName() override;

    CHIP_ERROR Open();
    bool IsExists(Key key) override;
    CHIP_ERROR ReadValue(Key key, bool & val) override;
    CHIP_ERROR ReadValue(Key key, uint32_t & val) override;
    CHIP_ERROR ReadValue(Key key, uint64_t & val) override;
    CHIP_ERROR ReadValueStr(Key key, char * buf, size_t bufSize, size_t & outLen) override;
    CHIP_ERROR ReadValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen) override;
    CHIP_ERROR WriteValue(Key key, bool val) override;
    CHIP_ERROR WriteValue(Key key, uint32_t val) override;
    CHIP_ERROR WriteValue(Key key, uint64_t val) override;
    CHIP_ERROR WriteValueStr(Key key, const char * str) override;
    CHIP_ERROR WriteValueBin(Key key, const uint8_t * bin, size_t dataLen) override;
    CHIP_ERROR ClearValue(Key key) override;
    CHIP_ERROR ClearAll() override;
    CHIP_ERROR Commit() override;

private:
    static void GdbmFail(const char *err);
    enum {
        DB_UNINITIALIZED,
        DB_ERROR,
        DB_READY
    } state = DB_UNINITIALIZED;

    enum Type {
        TYPE_BOOL_TRUE,
        TYPE_BOOL_FALSE,
        TYPE_UINT32,
        TYPE_UINT64,
        TYPE_STRING,
        TYPE_BLOB,
    };

    class DataInt32 {
    public:
        Type type;
        uint32_t int32;
    };
    class DataInt64 {
    public:
        Type type;
        uint64_t int64;
    };
    class DataString {
    public:
        Type type;
        char s[0];
    };

    const char *mName;
    GDBM_FILE mDatabase;
};

const char *GdbmStorage::GetName()
{
    return mName;
}

void GdbmStorage::GdbmFail(const char *err)
{
    ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage error %s", err);
}

CHIP_ERROR GdbmStorage::Open()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (state != DB_UNINITIALIZED)
    {
        if (state == DB_READY)
            return CHIP_NO_ERROR;
        else
            return CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    std::string file = PosixStorage::GetDataPath() + "/" + GetName() + ".gdbm";

    mDatabase = gdbm_open(file.c_str(), 512, GDBM_WRCREAT, S_IRUSR|S_IWUSR, GdbmStorage::GdbmFail);
    VerifyOrExit(mDatabase != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage error %s", gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    if (err == CHIP_NO_ERROR)
        state = DB_READY;
    else
        state = DB_ERROR;

    return err;
}

bool GdbmStorage::IsExists(Key key)
{
    datum k;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    bool exist = gdbm_exists(mDatabase, k);
    if (!exist && gdbm_errno != GDBM_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage exists %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
    }

    return exist;
}

CHIP_ERROR GdbmStorage::ReadValue(Key key, bool & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    Type *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    datum v = gdbm_fetch(mDatabase, k);
    VerifyOrExit(v.dptr != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read bool %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    data = (Type*)v.dptr;
    if (*data == Type::TYPE_BOOL_TRUE)
    {
        val = true;
    }
    else if (*data == Type::TYPE_BOOL_FALSE)
    {
        val = false;
    }
    else
    {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read bool %s:%s type mismatch %d", GetName(), key, *data);
    }

exit:
    if (v.dptr != NULL)
        free(v.dptr);

    return err;
}

CHIP_ERROR GdbmStorage::ReadValue(Key key, uint32_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    DataInt32 *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    datum v = gdbm_fetch(mDatabase, k);
    VerifyOrExit(v.dptr != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint32_t %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    data = (DataInt32*)v.dptr;
    VerifyOrExit(data->type == Type::TYPE_UINT32, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint32_t %s:%s type mismatch %d", GetName(), key, data->type);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    VerifyOrExit(v.dsize == sizeof(DataInt32), {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint32_t %s:%s size mismatch %d", GetName(), key, v.dsize);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    val = data->int32;

exit:
    if (v.dptr != NULL)
        free(v.dptr);

    return err;
}

CHIP_ERROR GdbmStorage::ReadValue(Key key, uint64_t & val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    DataInt64 *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    datum v = gdbm_fetch(mDatabase, k);
    VerifyOrExit(v.dptr != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint64_t %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    data = (DataInt64*)v.dptr;
    VerifyOrExit(data->type == Type::TYPE_UINT64, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint64_t %s:%s type mismatch %d", GetName(), key, data->type);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    VerifyOrExit(v.dsize == sizeof(DataInt64), {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read uint64_t %s:%s size mismatch %d", GetName(), key, v.dsize);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    val = data->int64;

exit:
    if (v.dptr != NULL)
        free(v.dptr);

    return err;
}

CHIP_ERROR GdbmStorage::ReadValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    DataString *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    datum v = gdbm_fetch(mDatabase, k);
    VerifyOrExit(v.dptr != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read string %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    data = (DataString*)v.dptr;
    VerifyOrExit(data->type == Type::TYPE_STRING, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read string %s:%s type mismatch %d", GetName(), key, data->type);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    outLen = v.dsize - offsetof(DataString, s) + 1;
    VerifyOrExit(bufSize >= outLen, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read string %s:%s error buffer too small %d < %d", GetName(), key, bufSize, outLen);
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    });

    memcpy(buf, data->s, outLen - 1);
    buf[outLen - 1] = 0;

exit:
    if (v.dptr != NULL)
        free(v.dptr);

    return err;
}

CHIP_ERROR GdbmStorage::ReadValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    DataString *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    datum v = gdbm_fetch(mDatabase, k);
    VerifyOrExit(v.dptr != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read bin %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    data = (DataString*)v.dptr;
    VerifyOrExit(data->type == Type::TYPE_BLOB, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read bin %s:%s type mismatch %d", GetName(), key, data->type);
        err = CHIP_ERROR_KEY_NOT_FOUND;
    });

    outLen = v.dsize - offsetof(DataString, s);

    VerifyOrExit(bufSize >= outLen, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage read bin %s:%s error buffer too small %d < %d", GetName(), key, bufSize, outLen);
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
    });

    memcpy(buf, data->s, outLen);

exit:
    if (v.dptr != NULL)
        free(v.dptr);

    return err;
}

CHIP_ERROR GdbmStorage::WriteValue(Key key, bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    datum v;

    Type data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    if (val) data = Type::TYPE_BOOL_TRUE;
    else data = Type::TYPE_BOOL_FALSE;
    v.dptr = (char*)&data;
    v.dsize = sizeof(data);

    err = gdbm_store(mDatabase, k, v, GDBM_REPLACE);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage write bool %s:%s error %d,%s", GetName(), key, err, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    return err;
}

CHIP_ERROR GdbmStorage::WriteValue(Key key, uint32_t val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    datum v;

    DataInt32 data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    data.type = Type::TYPE_UINT32;
    data.int32 = val;
    v.dptr = (char*)&data;
    v.dsize = sizeof(data);

    err = gdbm_store(mDatabase, k, v, GDBM_REPLACE);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage write uint32 %s:%s error %d,%s", GetName(), key, err, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    return err;
}

CHIP_ERROR GdbmStorage::WriteValue(Key key, uint64_t val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    datum v;

    DataInt64 data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    data.type = Type::TYPE_UINT64;
    data.int64 = val;
    v.dptr = (char*)&data;
    v.dsize = sizeof(data);

    err = gdbm_store(mDatabase, k, v, GDBM_REPLACE);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage write uint64 %s:%s error %d,%s", GetName(), key, err, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    return err;
}

CHIP_ERROR GdbmStorage::WriteValueStr(Key key, const char * str)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    datum v;

    DataString *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    size_t strLen = strlen(str);
    v.dsize = sizeof(DataString) + strLen;
    data = (DataString*)malloc(v.dsize);
    data->type = Type::TYPE_STRING;
    memcpy(data->s, str, strLen);
    v.dptr = (char*)data;

    err = gdbm_store(mDatabase, k, v, GDBM_REPLACE);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage write string %s:%s error %d,%s", GetName(), key, err, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    free(data);
    return err;
}

CHIP_ERROR GdbmStorage::WriteValueBin(Key key, const uint8_t * bin, size_t dataLen)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    datum v;

    DataString *data;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    v.dsize = sizeof(DataString) + dataLen;
    data = (DataString*)malloc(v.dsize);
    data->type = Type::TYPE_BLOB;
    memcpy(data->s, bin, dataLen);
    v.dptr = (char*)data;

    err = gdbm_store(mDatabase, k, v, GDBM_REPLACE);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage write bin %s:%s error %d,%s", GetName(), key, err, gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    free(data);
    return err;
}

CHIP_ERROR GdbmStorage::ClearValue(Key key)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum k;
    k.dptr = (char*)key;
    k.dsize = strlen(key);

    err = gdbm_delete(mDatabase, k);
    VerifyOrExit(err == 0, {
        if (gdbm_errno == GDBM_ITEM_NOT_FOUND) {
            err = CHIP_ERROR_KEY_NOT_FOUND;
        } else {
            ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage clear %s:%s error %s", GetName(), key, gdbm_strerror(gdbm_errno));
            err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
        }
    });

exit:
    return err;
}

CHIP_ERROR GdbmStorage::ClearAll()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    datum current = gdbm_firstkey(mDatabase);
    while (current.dptr) {
        CHIP_ERROR err_inner = gdbm_delete(mDatabase, current);
        if (err_inner != CHIP_NO_ERROR) {
            ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage clear all %s:%s error %s", GetName(), current.dptr, gdbm_strerror(gdbm_errno));
            err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
            // continue clear and return first error
        }

        datum previous = current;
        current = gdbm_nextkey(mDatabase, previous);
        free(previous.dptr);
    }
    return err;
}

CHIP_ERROR GdbmStorage::Commit()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = gdbm_sync(mDatabase);
    VerifyOrExit(err == 0, {
        ChipLogError(DeviceLayer, "ChipStorage: GdbmStorage sync %s error %s", GetName(), gdbm_strerror(gdbm_errno));
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    return err;
}

std::string PosixStorage::sConfigPath;

ChipMutableStorage *PosixStorage::GetConfigStorage()
{
    static GdbmStorage instance("config");
    CHIP_ERROR err = instance.Open();
    if (err == CHIP_NO_ERROR)
        return &instance;
    else
        return NULL;
}

ChipMutableStorage *PosixStorage::GetCountersStorage()
{
    static GdbmStorage instance("counters");
    CHIP_ERROR err = instance.Open();
    if (err == CHIP_NO_ERROR)
        return &instance;
    else
        return NULL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
