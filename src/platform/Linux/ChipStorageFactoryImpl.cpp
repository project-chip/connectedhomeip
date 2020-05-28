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
 *          Storage implementation for factory provision. Load provision
 *          information from xml file.
 *
 */

#include <cinttypes>
#include <cstddef>
#include <endian.h>
#include <memory>
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/xpath.h>

#include <platform/Linux/ChipStorage.h>
#include <platform/Linux/ChipStorageImpl.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <support/Base64.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace ::chip::Platform::PersistedStorage;

class ProvisionXmlStorage : public ChipStorage
{
private:
    ProvisionXmlStorage(const char * name) : mName(name) {}
    friend class PosixStorage;

public:
    ~ProvisionXmlStorage() override
    {
        if (state == XML_READY)
            xmlFreeDoc(doc);
    }

    const char * GetName() override;

    CHIP_ERROR Open();
    bool IsExists(Key key) override;
    CHIP_ERROR ReadValue(Key key, bool & val) override;
    CHIP_ERROR ReadValue(Key key, uint32_t & val) override;
    CHIP_ERROR ReadValue(Key key, uint64_t & val) override;
    CHIP_ERROR ReadValueStr(Key key, char * buf, size_t bufSize, size_t & outLen) override;
    CHIP_ERROR ReadValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen) override;

private:
    enum
    {
        XML_UNINITIALIZED,
        XML_ERROR,
        XML_READY
    } state = XML_UNINITIALIZED;

    const char * mName;
    xmlDocPtr doc;

    class Attributes
    {
    public:
        const char * key;
        const enum {
            STRING,
            NUMBER,
            BASE64BIN,
            HEXBIN,   // mac address style XX:XX:XX...
            HEXINT64, // 0xDEADBEEF
        } type;
        const xmlChar * xpath;
    };

    std::vector<Attributes> attrs{
        { "serial-num", Attributes::STRING, (const xmlChar *) "string(/config/a[@key=\"deviceserialnumber\"]/@value)" },
        { "device-id", Attributes::HEXINT64, (const xmlChar *) "string(/config/a[@key=\"deviceid\"]/@value)" },
        { "device-cert", Attributes::BASE64BIN, (const xmlChar *) "string(/config/a[@key=\"chipcertificate\"]/@value)" },
        { "device-key", Attributes::BASE64BIN, (const xmlChar *) "string(/config/a[@key=\"privatekey\"]/@value)" },
        { "product-rev", Attributes::NUMBER, (const xmlChar *) "string(/config/a[@key=\"hardwareversion\"]/@value)" },
        { "mfg-date", Attributes::STRING, NULL /* special case */ },
        { "pairing-code", Attributes::STRING, (const xmlChar *) "string(/config/a[@key=\"pairingcode\"]/@value)" },
    };
};

const char * ProvisionXmlStorage::GetName()
{
    return mName;
}

CHIP_ERROR ProvisionXmlStorage::Open()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (state != XML_UNINITIALIZED)
    {
        if (state == XML_READY)
            return CHIP_NO_ERROR;
        else
            return CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    }

    std::string file = PosixStorage::GetConfigPath() + "/" + GetName() + ".xml";

    doc = xmlReadFile(file.c_str(), NULL, XML_PARSE_NONET);
    VerifyOrExit(doc != NULL, {
        ChipLogError(DeviceLayer, "ChipStorage: ProvisionXmlStorage error read file %s", file.c_str());
        err = CHIP_ERROR_PERSISTED_STORAGE_FAIL;
    });

exit:
    if (err == CHIP_NO_ERROR)
        state = XML_READY;
    else
        state = XML_ERROR;

    return err;
}

bool ProvisionXmlStorage::IsExists(Key key)
{
    for (auto const & attr : attrs)
    {
        if (strcmp(attr.key, key) == 0)
            return true;
    }

    return false;
}

CHIP_ERROR ProvisionXmlStorage::ReadValue(Key key, bool & val)
{
    ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR ProvisionXmlStorage::ReadValue(Key key, uint32_t & val)
{
    for (auto const & attr : attrs)
    {
        if (strcmp(attr.key, key) == 0)
        {
            if (attr.type == Attributes::NUMBER)
            {
                std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> context(xmlXPathNewContext(doc),
                                                                                         &xmlXPathFreeContext);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result(
                    xmlXPathEvalExpression(attr.xpath, context.get()), &xmlXPathFreeObject);
                sscanf((const char *) result->stringval, "%u", &val);
                return CHIP_NO_ERROR;
            }
            else
            {
                ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
                return CHIP_ERROR_KEY_NOT_FOUND;
            }
        }
    }
    ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR ProvisionXmlStorage::ReadValue(Key key, uint64_t & val)
{
    ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR ProvisionXmlStorage::ReadValueStr(Key key, char * buf, size_t bufSize, size_t & outLen)
{
    for (auto const & attr : attrs)
    {
        if (strcmp(attr.key, key) == 0)
        {
            if (strcmp(attr.key, "mfg-date") == 0)
            {
                int year, month, day;
                if (bufSize < 11)
                {
                    outLen = 11;
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
                std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> context(xmlXPathNewContext(doc),
                                                                                         &xmlXPathFreeContext);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result1(
                    xmlXPathEvalExpression((const xmlChar *) "string(/config/a[@key=\"year\"]/@value)", context.get()),
                    &xmlXPathFreeObject);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result2(
                    xmlXPathEvalExpression((const xmlChar *) "string(/config/a[@key=\"month\"]/@value)", context.get()),
                    &xmlXPathFreeObject);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result3(
                    xmlXPathEvalExpression((const xmlChar *) "string(/config/a[@key=\"day\"]/@value)", context.get()),
                    &xmlXPathFreeObject);
                sscanf((const char *) result1->stringval, "%d", &year);
                sscanf((const char *) result2->stringval, "%d", &month);
                sscanf((const char *) result3->stringval, "%d", &day);
                outLen = snprintf(buf, bufSize, "%04d/%02d/%02d", year, month, day) + 1;
                return CHIP_NO_ERROR;
            }
            else if (attr.type == Attributes::STRING)
            {
                std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> context(xmlXPathNewContext(doc),
                                                                                         &xmlXPathFreeContext);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result(
                    xmlXPathEvalExpression(attr.xpath, context.get()), &xmlXPathFreeObject);
                outLen = strlen((const char *) result->stringval) + 1;
                if (bufSize < outLen)
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                strncpy(buf, (const char *) result->stringval, bufSize);
                return CHIP_NO_ERROR;
            }
            else
            {
                ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
                return CHIP_ERROR_KEY_NOT_FOUND;
            }
        }
    }

    ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR ProvisionXmlStorage::ReadValueBin(Key key, uint8_t * buf, size_t bufSize, size_t & outLen)
{
    for (auto const & attr : attrs)
    {
        if (strcmp(attr.key, key) == 0)
        {
            if (attr.type == Attributes::HEXINT64)
            {
                std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> context(xmlXPathNewContext(doc),
                                                                                         &xmlXPathFreeContext);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result(
                    xmlXPathEvalExpression(attr.xpath, context.get()), &xmlXPathFreeObject);
                if (bufSize < sizeof(uint64_t))
                {
                    outLen = sizeof(uint64_t);
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
                uint64_t val;
                outLen = sizeof(val);
                sscanf((const char *) result->stringval, "%" PRIx64, &val);
                *(uint64_t *) buf = htobe64(val);
                return CHIP_NO_ERROR;
            }
            else if (attr.type == Attributes::BASE64BIN)
            {
                std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> context(xmlXPathNewContext(doc),
                                                                                         &xmlXPathFreeContext);
                std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> result(
                    xmlXPathEvalExpression(attr.xpath, context.get()), &xmlXPathFreeObject);
                size_t len = strlen((const char *) result->stringval);
                if (len * 3 / 4 > bufSize)
                {
                    outLen = len * 3 / 4;
                    return CHIP_ERROR_BUFFER_TOO_SMALL;
                }
                outLen = Base64Decode((const char *) result->stringval, len, buf);
                return CHIP_NO_ERROR;
            }
            else
            {
                ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
                return CHIP_ERROR_KEY_NOT_FOUND;
            }
        }
    }

    ChipLogError(DeviceLayer, "ProvisionXmlStorage::%s(%d): key(%s) not found", __FUNCTION__, __LINE__, key);
    return CHIP_ERROR_KEY_NOT_FOUND;
}

std::string PosixStorage::sDataPath;

ChipStorage * PosixStorage::GetFacotryStorage()
{
    static ProvisionXmlStorage instance("factory");
    CHIP_ERROR err = instance.Open();

    if (err == CHIP_NO_ERROR)
        return &instance;
    else
        return NULL;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
