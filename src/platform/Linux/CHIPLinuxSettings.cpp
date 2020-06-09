/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019-2020 Google LLC.
 *    Copyright (c) 2018 Nest Labs, Inc.
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

/**
 *    @file
 *         This file implements a class for managing client application
 *         user-editable settings.
 *
 */

#include <errno.h>
#include <libgen.h>
#include <nspr/plstr.h>
#include <nspr/prprf.h>
#include <string>
#include <unistd.h>

#include <platform/Linux/CHIPLinuxSettings.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

ChipLinuxSettings::ChipLinuxSettings()
{
    mLock     = NULL;
    mDirty    = false;
    mWritable = true;
}

ChipLinuxSettings::~ChipLinuxSettings()
{
    if (mLock != NULL)
    {
        PR_DestroyLock(mLock);
        mLock = NULL;
    }
}

CHIP_ERROR ChipLinuxSettings::Init(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mLock = PR_NewLock();

    if (mLock == NULL)
    {
        ChipLogError(DeviceLayer, "failed to NewLock");
        retval = CHIP_ERROR_NO_MEMORY;
    }

    if (retval == CHIP_NO_ERROR)
    {
        mConfigPath.assign(configFile);
        retval = ChipLinuxConfig::Init();
    }

    if (retval == CHIP_NO_ERROR && PR_Access(configFile, PR_ACCESS_EXISTS) != PR_SUCCESS)
    {
        mDirty = true;
        retval = Commit();
        mDirty = false;
    }

    if (retval == CHIP_NO_ERROR)
    {
        retval = ChipLinuxConfig::AddConfig(configFile);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, bool & val) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    uint32_t result;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::GetUIntValue(key, result);
    val    = (result == 0 ? false : true);

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, uint32_t & val) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::GetUIntValue(key, val);

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValue(const char * key, uint64_t & val) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::GetUInt64Value(key, val);

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValueStr(const char * key, char * buf, size_t bufSize, size_t & outLen) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::GetBinaryBlobValue(key, (uint8_t *) buf, bufSize, outLen);

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ReadValueBin(const char * key, uint8_t * buf, size_t bufSize, size_t & outLen) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::GetBinaryBlobValue(key, buf, bufSize, outLen);

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, bool val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    if (val)
    {
        retval = WriteValue(key, (uint32_t) 1);
    }
    else
    {
        retval = WriteValue(key, (uint32_t) 0);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, uint32_t val)
{
    char buf[32];

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    PR_snprintf(buf, sizeof(buf), "%d", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxSettings::WriteValue(const char * key, uint64_t val)
{
    char buf[64];

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    PR_snprintf(buf, sizeof(buf), "%llu", val);

    return WriteValueStr(key, buf);
}

CHIP_ERROR ChipLinuxSettings::WriteValueStr(const char * key, const char * val)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    PR_Lock(mLock);

    retval = ChipLinuxConfig::AddEntry(key, val);

    mDirty = true;

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::WriteValueBin(const char * key, const uint8_t * data, size_t dataLen)
{
    static const size_t kMaxBlobSize = 5 * 1024;

    CHIP_ERROR retval         = CHIP_NO_ERROR;
    char * encodedData        = NULL;
    size_t encodedDataLen     = 0;
    size_t expectedEncodedLen = ((dataLen + 3) * 4) / 3;

    if (!mWritable)
    {
        return CHIP_ERROR_BAD_REQUEST;
    }

    // We only support encoding blobs up to 5kb
    if (dataLen > kMaxBlobSize)
    {
        retval = CHIP_ERROR_INVALID_ARGUMENT;
    }

    // Compute our expectedEncodedLen
    // Allocate just enough space for the encoded data, and the NULL terminator
    if (retval == CHIP_NO_ERROR)
    {
        encodedData = (char *) malloc(expectedEncodedLen + 1);
        if (encodedData == NULL)
        {
            retval = CHIP_ERROR_NO_MEMORY;
        }
    }

    // Encode it
    if (retval == CHIP_NO_ERROR)
    {
        encodedDataLen              = Base64Encode(data, dataLen, encodedData);
        encodedData[encodedDataLen] = 0;
    }

    // Store it
    if (retval == CHIP_NO_ERROR)
    {
        WriteValueStr(key, (const char *) encodedData);
    }

    // Free memory
    if (encodedData)
    {
        free(encodedData);
    }

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ClearValue(const char * key)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::RemoveEntry(key);

    if (retval == CHIP_NO_ERROR)
    {
        mDirty = true;
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    PR_Unlock(mLock);

    return retval;
}

CHIP_ERROR ChipLinuxSettings::ClearAll(void)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::RemoveAll();

    if (retval == CHIP_NO_ERROR)
    {
        retval = Commit();
    }
    else
    {
        retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    PR_Unlock(mLock);

    return retval;
}

bool ChipLinuxSettings::HasValue(const char * key) const
{
    bool retval;

    PR_Lock(mLock);

    retval = ChipLinuxConfig::HasValue(key);

    PR_Unlock(mLock);

    return retval;
}

void ChipLinuxSettings::SetWritability(bool writable)
{
    mWritable = writable;
}

void escape_value(const char * val, char * buf, size_t bufSize)
{
    size_t idx, len, outidx;

    len = strlen(val);

    outidx = 0;

    for (idx = 0; (idx < len) && (bufSize > 1); idx++)
    {
        if ((val[idx] == '"') || (val[idx] == '&') || (val[idx] < ' ') || (val[idx] > '~'))
        {
            size_t numchars;

            numchars = PR_snprintf(&buf[outidx], bufSize, "&#%d;", (uint8_t) val[idx]);

            outidx += numchars;
            bufSize -= numchars;
        }
        else
            buf[outidx++] = val[idx];
    }

    buf[outidx] = 0;
}

PRIntn settings_writer(PLHashEntry * he, PRIntn index, void * arg)
{
    PRFileDesc * file = (PRFileDesc *) arg;
    char buf[2048], esc[2048];

    if ((he->key != NULL) && (he->value != NULL))
    {
        size_t len, numwritten;

        escape_value((const char *) he->value, esc, sizeof(esc));

        len = PR_snprintf(buf, sizeof(buf), "  <s key=\"%s\" value=\"%s\"/>\n", he->key, esc);

        numwritten = PR_Write(file, buf, len);

        if (len != numwritten)
            ChipLogError(DeviceLayer, "failed to write key=%s value=%s", (const char *) he->key, esc);
    }

    return HT_ENUMERATE_NEXT;
}

// Updating a file atomically and durably on Linux requires:
// 1. Writing to a temporary file
// 2. Sync'ing the temp file to commit updated data
// 3. Using rename() to overwrite the existing file
// 4. Sync'ing the directory/directories to commit the rename
CHIP_ERROR ChipLinuxSettings::Commit(void)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    const char * path = mConfigPath.c_str();

    if (mDirty && mWritable && path != NULL && strcmp(path, ""))
    {
        PR_Lock(mLock);

        PRFileDesc * file;
        char buf[128];
        uint32_t ret, len, numwritten;
        std::string nltmppath;
        const char * tmppath;

        nltmppath = mConfigPath;
        nltmppath.append(".tmp");
        tmppath = nltmppath.c_str();

        file = PR_Open(tmppath, PR_TRUNCATE | PR_CREATE_FILE | PR_WRONLY, 0666);

        if (file != NULL)
        {
            ChipLogProgress(DeviceLayer, "writing settings to file (%s)", path);

            len = PR_snprintf(buf, sizeof(buf), "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<settings>\n");

            numwritten = PR_Write(file, buf, len);

            if (len == numwritten)
            {
                if (mConfigStore != NULL)
                    HASHTABLE_ENUMERATE(mConfigStore, settings_writer, file);

                len = PR_snprintf(buf, sizeof(buf), "</settings>\n");

                numwritten = PR_Write(file, buf, len);
            }

            if (len != numwritten)
            {
                ChipLogError(DeviceLayer, "failed to write to %s", tmppath);
                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
            }

            ret = PR_Sync(file);
            if (ret != PR_SUCCESS)
            {
                ChipLogError(DeviceLayer, "failed to sync %s", tmppath);
                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
            }

            // Regardless of what's happened previously, always close
            ret = PR_Close(file);

            // If things have been successful, attempt the rename
            if (ret == PR_SUCCESS && retval == CHIP_NO_ERROR)
            {
                // Use rename() to ensure an atomic overwrite -
                // PR_Rename() will not overwrite an existing file,
                // and deleting the file first leaves a period of time
                // where no valid settings file exists at mConfigPath.
                ret = rename(tmppath, path);
                if (ret == 0)
                {
                    // dirname() may modify its argument, so make a copy of tmppath
                    char * tmppath_dup = PL_strdup(tmppath);
                    if (tmppath_dup != NULL)
                    {
                        char * dir        = dirname(tmppath_dup);
                        PRFileDesc * pdir = PR_Open(dir, PR_RDONLY, 0);
                        if (pdir != NULL)
                        {
                            ret = PR_Sync(pdir);
                            if (ret != PR_SUCCESS)
                            {
                                ChipLogError(DeviceLayer, "failed to sync %s", dir);
                                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
                            }

                            // Again, regardless of what's happened previously, always close
                            ret = PR_Close(pdir);
                            if (ret == PR_SUCCESS)
                            {
                                mDirty = false;
                                ChipLogError(DeviceLayer, "written settings to file (%s)", path);
                            }
                            else
                            {
                                ChipLogError(DeviceLayer, "failed to sync & close (%s)", dir);
                                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
                            }
                        }
                        else
                        {
                            ChipLogError(DeviceLayer, "failed to open (%s)", dir);
                            retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
                        }
                        PL_strfree(tmppath_dup);
                    }
                    else
                    {
                        ChipLogError(DeviceLayer, "failed to duplicate string");
                        retval = CHIP_ERROR_NO_MEMORY;
                    }
                }
                else
                {
                    ChipLogError(DeviceLayer, "failed to rename (%s), %s (%d)", tmppath, strerror(errno), errno);
                    retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
                }
            }
            else if (retval == CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "failed to sync & close (%s)", tmppath);
                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "failed to open file (%s) for writing", tmppath);
            retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
        }
        PR_Unlock(mLock);
    }

    return retval;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
