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
 *          Storage interface implementation
 *
 */

#include <nspr/prio.h>
#include <nspr/prmem.h>
#include <string>
#include <unistd.h>

#include <platform/Linux/CHIPLinuxConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

static void * config_alloc_table(void * pool, PRSize size)
{
    return PR_MALLOC(size);
}

static void config_free_table(void * pool, void * item)
{
    PR_Free(item);
}

static PLHashEntry * config_alloc_entry(void * pool, const void * key)
{
    return PR_NEW(PLHashEntry);
}

static void config_free_entry(void * pool, PLHashEntry * he, PRUintn flag)
{
    if (flag == HT_FREE_ENTRY)
    {
        if (he->key != NULL)
        {
            free((void *) he->key);
            he->key = NULL;
        }

        if (he->value != NULL)
        {
            free(he->value);
            he->value = NULL;
        }

        PR_Free(he);
    }
}

static PLHashAllocOps config_alloc_ops = { config_alloc_table, config_free_table, config_alloc_entry, config_free_entry };

ChipLinuxConfig::ChipLinuxConfig()
{
    mConfigStore = NULL;
}

ChipLinuxConfig::~ChipLinuxConfig()
{
    if (mConfigStore != NULL)
    {
        HASHTABLE_DESTROY(mConfigStore);
        mConfigStore = NULL;
    }
}

CHIP_ERROR ChipLinuxConfig::Init(void)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    mConfigStore = HASHTABLE_NEW_ALLOCOPS(&config_alloc_ops);

    if (mConfigStore == NULL)
    {
        ChipLogError(DeviceLayer, "failed to allocate config store");
        retval = CHIP_ERROR_NO_MEMORY;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::AddConfig(const char * configFile)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    char * xml        = NULL;
    PRFileInfo info;
    PRFileDesc * file = PR_Open(configFile, PR_RDONLY, 0);

    if (file != NULL)
    {
        PRStatus status;

        status = PR_GetOpenFileInfo(file, &info);

        if (status == PR_SUCCESS)
        {
            xml = (char *) malloc(info.size);

            if (xml != NULL)
            {
                PRInt32 numread;

                ChipLogProgress(DeviceLayer, "reading file (%s)", configFile);

                numread = PR_Read(file, xml, info.size);

                if (numread != info.size)
                {
                    ChipLogError(DeviceLayer, "read error in xml file: %s", configFile);
                    retval = CHIP_ERROR_NO_MEMORY;
                }
            }
            else
            {
                ChipLogError(DeviceLayer, "failed to allocate %d bytes for xml for file: %s", info.size, configFile);
                retval = CHIP_ERROR_NO_MEMORY;
            }
        }
        else
        {
            ChipLogError(DeviceLayer, "failed to get info for file: %s", configFile);
            retval = CHIP_ERROR_NO_MEMORY;
        }

        PR_Close(file);
    }
    else
    {
        ChipLogError(DeviceLayer, "failed to open config file: %s", configFile);
        retval = CHIP_ERROR_NOT_IMPLEMENTED;
    }

    if (retval == CHIP_NO_ERROR)
    {
        xmlDoc * doc   = NULL;
        xmlNode * root = NULL;

        // parse the xml

        doc = xmlParseMemory(xml, info.size);

        if (doc != NULL)
        {
            root = xmlDocGetRootElement(doc);

            if (root != NULL)
                retval = GetConfigData(root);
            else
            {
                ChipLogError(DeviceLayer, "failed to get doc root for config file: %s", configFile);
                retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
            }

            xmlFreeDoc(doc);
        }
        else
        {
            ChipLogError(DeviceLayer, "failed to parse config file: %s", configFile);
            retval = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
        }
    }

    if (xml != NULL)
        free(xml);

    return retval;
}

CHIP_ERROR ChipLinuxConfig::GetUIntValue(const char * key, uint32_t & val) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    char *str, *endPtr;

    str = (char *) HASHTABLE_LOOKUP(mConfigStore, key);

    if (str != NULL)
    {
        errno = 0;
        val   = strtoul(str, &endPtr, 0);
        if (errno != 0 || *endPtr != 0)
        {
            retval = CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::GetUInt64Value(const char * key, uint64_t & val) const
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    char *str, *endPtr;

    str = (char *) HASHTABLE_LOOKUP(mConfigStore, key);

    if (str != NULL)
    {
        errno = 0;
        val   = strtoull(str, &endPtr, 0);
        if (errno != 0 || *endPtr != 0)
        {
            retval = CHIP_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::GetBinaryBlobDataAndLengths(const char * key, char *& encodedData, size_t & encodedDataLen,
                                                        size_t & decodedDataLen) const
{
    CHIP_ERROR retval            = CHIP_NO_ERROR;
    size_t encodedDataPaddingLen = 0;

    encodedData = (char *) HASHTABLE_LOOKUP(mConfigStore, key);

    // Compute the expectedDecodedLen
    if (encodedData)
    {
        encodedDataLen = strlen(encodedData);

        // Check if encoded data was padded. Only "=" or "==" padding combinations are allowed.
        if ((encodedDataLen > 0) && (encodedData[encodedDataLen - 1] == '='))
        {
            encodedDataPaddingLen++;
            if ((encodedDataLen > 1) && (encodedData[encodedDataLen - 2] == '='))
                encodedDataPaddingLen++;
        }

        decodedDataLen = ((encodedDataLen - encodedDataPaddingLen) * 3) / 4;
    }
    else
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::GetBinaryBlobValue(const char * key, uint8_t * decodedData, size_t bufSize,
                                               size_t & decodedDataLen) const
{
    CHIP_ERROR retval  = CHIP_NO_ERROR;
    char * encodedData = NULL;
    size_t encodedDataLen;
    size_t expectedDecodedLen = 0;

    retval = GetBinaryBlobDataAndLengths(key, encodedData, encodedDataLen, expectedDecodedLen);

    // Check the size
    if (retval == CHIP_NO_ERROR)
    {
        if (expectedDecodedLen > bufSize)
        {
            retval = CHIP_ERROR_BUFFER_TOO_SMALL;
        }
    }

    // Decode it
    if (retval == CHIP_NO_ERROR)
    {
        decodedDataLen = Base64Decode(encodedData, encodedDataLen, (uint8_t *) decodedData);
        if (decodedDataLen == PR_UINT16_MAX || decodedDataLen > expectedDecodedLen)
        {
            retval = CHIP_ERROR_NOT_IMPLEMENTED;
        }
    }

    return retval;
}

const char * ChipLinuxConfig::GetValue(const char * key) const
{
    return (char *) HASHTABLE_LOOKUP(mConfigStore, key);
}

bool ChipLinuxConfig::HasValue(const char * key) const
{
    bool retval = false;

    if (key != NULL)
        retval = (bool) (GetValue(key) != NULL);

    return retval;
}

CHIP_ERROR ChipLinuxConfig::AddEntry(const char * key, const char * value)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;
    char *mkey = NULL, *mval = NULL;
    const char * oldval;

    oldval = GetValue(key);

    if (oldval != NULL)
    {
        if (oldval != value)
            free((void *) oldval);

        mkey = (char *) key;
    }
    else
        mkey = (char *) malloc(strlen(key) + 1);

    mval = (char *) malloc(strlen(value) + 1);

    if ((mkey != NULL) && (mval != NULL))
    {
        if (key != mkey)
            strcpy(mkey, key);

        strcpy(mval, value);

        HASHTABLE_ADD(mConfigStore, mkey, mval);
    }
    else
    {
        ChipLogError(DeviceLayer, "failed to allocate key/value pair for: %s, %s", key, value);

        if ((mkey != NULL) && (mkey != key))
            free(mkey);

        if (mval != NULL)
            free(mval);

        retval = CHIP_ERROR_NO_MEMORY;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::RemoveEntry(const char * key)
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (HASHTABLE_REMOVE(mConfigStore, key) == PR_FALSE)
    {
        retval = CHIP_ERROR_KEY_NOT_FOUND;
    }

    return retval;
}

CHIP_ERROR ChipLinuxConfig::RemoveAll()
{
    CHIP_ERROR retval = CHIP_NO_ERROR;

    if (mConfigStore != NULL)
    {
        HASHTABLE_DESTROY(mConfigStore);
        retval = Init();
    }
    else
    {
        retval = CHIP_ERROR_NO_MEMORY;
    }

    return retval;
}

static int xmlEqualCString(const xmlChar * str1, const char * str2)
{
    return strcmp((const char *) str1, str2) == 0;
}

CHIP_ERROR ChipLinuxConfig::GetConfigData(xmlNode * node)
{
    CHIP_ERROR retval   = CHIP_NO_ERROR;
    const xmlNode * cur = node;

    while ((cur != NULL) && (retval == CHIP_NO_ERROR))
    {
        if (cur->type == XML_ELEMENT_NODE)
        {
            xmlAttr * attr   = cur->properties;
            const char * key = NULL;
            const char * val = NULL;

            while (attr != NULL)
            {
                if ((attr->name != NULL) && (attr->children != NULL))
                {
                    if (xmlEqualCString(attr->name, "key"))
                        key = (const char *) attr->children->content;
                    else if (xmlEqualCString(attr->name, "value"))
                        val = (const char *) attr->children->content;
                }

                attr = attr->next;
            }

            if ((key != NULL) && (val != NULL))
            {
                unsigned char latin1[2048];
                int inlen, outlen;

                inlen  = strlen(val);
                outlen = sizeof(latin1) - 1;

                outlen = UTF8Toisolat1(latin1, &outlen, (unsigned char *) val, &inlen);
                if (outlen < 0)
                    outlen = 0;
                latin1[outlen] = 0;

                retval = AddEntry(key, (char *) latin1);

                if (retval == CHIP_ERROR_NO_MEMORY)
                    break;
            }
        }

        retval = GetConfigData(cur->children);

        cur = cur->next;
    }

    return retval;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
