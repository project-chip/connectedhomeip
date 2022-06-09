#include "Backend.h"
#include "DynamicDevice.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::unique_ptr<Device>> g_devices;
std::vector<std::unique_ptr<DynamicDeviceImpl>> g_device_impls;
std::unique_ptr<DynamicDeviceImpl> g_pending;

PropagateWriteCB g_write_cb = [](CommonCluster * cluster, chip::EndpointId ep, chip::ClusterId clusterId, chip::AttributeId attr,
                                 const uint8_t * buffer) -> EmberAfStatus {
    printf("Write to ep %d cluster %d attribute %d\n", ep, clusterId, attr);
    cluster->WriteFromBridge(attr, buffer);
    return EMBER_ZCL_STATUS_SUCCESS;
};

void NewDevice(const std::vector<std::string> & tokens)
{
    if (g_pending)
    {
        printf("Already pending device!\n");
        return;
    }
    if (tokens.size() < 2)
    {
        g_pending = std::make_unique<DynamicDeviceImpl>();
    }
    else if (tokens[1] == "switch")
    {
        g_pending = std::make_unique<DynamicSwitchDevice>();
    }
    else
    {
        printf("Unknown device type %s\n", tokens[1].c_str());
        return;
    }
}

void AddCluster(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device!\n");
        return;
    }
    if (tokens.size() < 2)
    {
        printf("Expected id of a device type\n");
        return;
    }

    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (tokens[1] == cluster.name)
        {
            std::unique_ptr<CommonCluster> obj(cluster.ctor(::operator new(cluster.size)));
            DynamicAttributeList dynamic_attrs;
            auto attrs = obj->GetAllAttributes();
            for (auto & attr : attrs)
                dynamic_attrs.Add(attr);
            g_pending->AddCluster(std::move(obj), dynamic_attrs, nullptr, nullptr);
        }
    }
}

void CancelDevice(const std::vector<std::string> & tokens)
{
    g_pending.reset(nullptr);
}

void FinishDevice(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device!\n");
        return;
    }

    for (auto * c : g_pending->clusters())
        c->SetCallback(&g_write_cb);

    auto dev = std::make_unique<Device>(g_pending->CreateDevice());
    int ep   = AddDeviceEndpoint(dev.get());
    if (ep < 0)
    {
        printf("Failed to add device\n");
    }
    else
    {
        printf("Added device at index %d\n", ep);
        size_t index = (size_t) ep;
        if (g_devices.size() <= index)
        {
            g_devices.resize(index + 1);
            g_device_impls.resize(index + 1);
        }
        g_devices[index]      = std::move(dev);
        g_device_impls[index] = std::move(g_pending);
    }
}

void RemoveDevice(const std::vector<std::string> & tokens)
{
    if (tokens.size() < 2)
    {
        printf("Expected index of a device\n");
        return;
    }
    uint32_t index = atoi(tokens[1].c_str());
    if (index >= g_devices.size())
    {
        printf("%d is an invalid index\n", index);
        return;
    }

    RemoveDeviceEndpoint(g_devices[index].get());

    g_devices[index]      = nullptr;
    g_device_impls[index] = nullptr;
}

void AddType(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device!\n");
        return;
    }
    if (tokens.size() < 2)
    {
        printf("Expected id of a device type\n");
        return;
    }
    EmberAfDeviceType type = { (uint16_t) atoi(tokens[1].c_str()), 1u };
    if (tokens.size() >= 3)
    {
        type.deviceVersion = (uint8_t) atoi(tokens[2].c_str());
    }
    printf("Adding device type %d ver %d\n", type.deviceId, type.deviceVersion);

    g_pending->AddDeviceType(type);
}

bool FindIndexAttr(const std::vector<std::string> & tokens, ClusterImpl ** cluster, chip::AttributeId * attrId)
{
    if (tokens.size() < 4)
        return false;
    int index = atoi(tokens[1].c_str());
    DynamicDeviceImpl * dev;
    if (index == -1)
    {
        dev = g_pending.get();
    }
    else if ((uint32_t) index < g_device_impls.size())
    {
        dev = g_device_impls[index].get();
    }
    else
    {
        return false;
    }
    *cluster                  = nullptr;
    chip::ClusterId clusterId = (chip::ClusterId) atoi(tokens[2].c_str());
    for (auto * c : dev->clusters())
    {
        if (c->GetClusterId() == clusterId)
        {
            *cluster = c;
            break;
        }
    }
    if (!*cluster)
        return false;
    *attrId = (chip::AttributeId) atoi(tokens[3].c_str());
    return true;
}

void ParseValue(std::vector<uint8_t> * data, uint16_t size, const std::string & str, EmberAfAttributeType type)
{
    switch (type)
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        if (str.size() >= 255)
            return;
        data->resize(size);
        memcpy(data->data() + 1, str.data(), str.size());
        data->data()[0] = (uint8_t) str.size();
        break;
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE: {
        if (str.size() >= 65534)
            return;
        data->resize(size);
        memcpy(data->data() + 2, str.data(), str.size());
        uint16_t length = (uint16_t) str.size();
        memcpy(data->data(), &length, 2);
        data->data()[0] = (uint8_t) str.size();
    }
    break;
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // Writing structs not supported yet
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE: {
        float v = (float) atof(str.c_str());
        data->resize(size);
        memcpy(data->data(), &v, size);
    }
    break;
    case ZCL_DOUBLE_ATTRIBUTE_TYPE: {
        double v = atof(str.c_str());
        data->resize(size);
        memcpy(data->data(), &v, size);
    }
    break;
    default: {
        // Assume integer
        uint64_t v = strtoull(str.c_str(), nullptr, 10);
        data->resize(size);
        memcpy(data->data(), &v, size);
    }
    break;
    }
}

void SetValue(const std::vector<std::string> & tokens)
{
    ClusterImpl * cluster;
    chip::AttributeId attrId;
    if (!FindIndexAttr(tokens, &cluster, &attrId))
    {
        printf("Invalid device or cluster\n");
        return;
    }
    if (tokens.size() < 5)
        return;
    std::vector<uint8_t> data;

    for (auto attr : cluster->GetAllAttributes())
    {
        if (attr.attributeId == attrId)
        {
            ParseValue(&data, attr.size, tokens[4], attr.attributeType);
            break;
        }
    }

    if (data.empty())
    {
        printf("no data to write! Invalid attribute or unsupported type?\n");
        return;
    }

    cluster->WriteFromBridge(attrId, data.data());
}

void Help(const std::vector<std::string> & tokens);

struct Command
{
    const char * name;
    const char * desc;
    void (*fn)(const std::vector<std::string> & tokens);
} const commands[] = {
    { "help", "", &Help },
    { "new-device", "[type]", &NewDevice },
    { "del-device", "index", &RemoveDevice },
    { "add-cluster", "name", &AddCluster },
    { "finish-device", "", &FinishDevice },
    { "cancel-device", "", &CancelDevice },
    { "add-type", "id [version] - Add a device type", &AddType },
    { "set", "index cluster attr value - Index -1 for adding device", &SetValue },
};

void Help(const std::vector<std::string> & tokens)
{
    for (auto & cmd : commands)
    {
        printf("%s %s\n", cmd.name, cmd.desc);
    }
}

void ProcessLine(const std::vector<std::string> & tokens)
{
    for (auto & cmd : commands)
    {
        if (tokens[0] == cmd.name)
        {
            cmd.fn(tokens);
            return;
        }
    }
    printf("Unknown command '%s'\n", tokens[0].c_str());
}

void * UserInputThread(void *)
{
    std::string line;
    while (true)
    {
        printf("? ");
        std::cin >> std::ws;
        std::getline(std::cin, line);
        if (std::cin.eof())
            break;

        std::istringstream iss(line);
        std::vector<std::string> tokens{ std::istream_iterator<std::string>{ iss }, std::istream_iterator<std::string>{} };

        ProcessLine(tokens);
    }
    return nullptr;
}

} // namespace

void StartUserInput()
{
    pthread_t thread;
    pthread_create(&thread, nullptr, &UserInputThread, nullptr);
}
