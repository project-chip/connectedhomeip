#include "Backend.h"
#include "DynamicDevice.h"
#include "main.h"

#include <algorithm>
#include <charconv>
#include <iostream>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::vector<std::unique_ptr<Device>> g_devices;
std::vector<std::unique_ptr<DynamicDeviceImpl>> g_device_impls;
std::unique_ptr<DynamicDeviceImpl> g_pending;

// Pseudo-index representing the device being built.
static constexpr int kPendingDeviceIndex = -1;

PropagateWriteCB g_write_cb = [](CommonCluster * cluster, chip::EndpointId ep, chip::ClusterId clusterId, chip::AttributeId attr,
                                 const uint8_t * buffer) -> EmberAfStatus {
    printf("Write to ep %d cluster %d attribute %d\n", ep, clusterId, attr);
    cluster->WriteFromBridge(attr, buffer);
    return EMBER_ZCL_STATUS_SUCCESS;
};

const char * ParseValue(const std::vector<std::string> & tokens, size_t index, std::string * v)
{
    if (index >= tokens.size())
        return "Missing argument";
    *v = tokens[index];
    return nullptr;
}
const char * ParseValue(const std::vector<std::string> & tokens, size_t index, std::optional<std::string> * v)
{
    if (index < tokens.size())
        *v = tokens[index];
    return nullptr;
}

template <typename T, typename = std::enable_if<std::is_arithmetic<T>::value>>
const char * ParseValue(const std::vector<std::string> & tokens, size_t index, T * v)
{
    if (index >= tokens.size())
        return "Missing argument";
    const char * start = tokens[index].data();
    const char * end   = start + tokens[index].size();
    auto ret           = std::from_chars(start, end, *v);
    if (ret.ptr != end)
        return "Cannot parse as integer";
    return nullptr;
}

template <typename T, typename = std::enable_if<std::is_arithmetic<T>::value>>
const char * ParseValue(const std::vector<std::string> & tokens, size_t index, std::optional<T> * v)
{
    uint32_t temp;
    if (!ParseValue(tokens, index, &temp))
        *v = temp;
    return nullptr;
}

const char * Parse(const std::vector<std::string> & tokens, size_t index)
{
    if (index >= tokens.size())
        return nullptr;
    return "Unexpected extra tokens";
}

template <typename T, typename... Args>
const char * Parse(const std::vector<std::string> & tokens, size_t index, T arg0, Args... args)
{
    auto ret = ParseValue(tokens, index, arg0);
    return ret ? ret : Parse(tokens, index + 1, args...);
}

void NewDevice(const std::vector<std::string> & tokens)
{
    if (g_pending)
    {
        printf("Already pending device!\n");
        return;
    }

    std::optional<std::string> type;
    const char * err = Parse(tokens, 0, &type);
    if (err)
    {
        printf("Error: %s\nExpected [prebuilt-device-type]\n", err);
    }
    else if (!type)
    {
        g_pending = std::make_unique<DynamicDeviceImpl>();
    }
    else if (type.value() == "switch")
    {
        g_pending = std::make_unique<DynamicSwitchDevice>();
    }
    else
    {
        printf("Unknown device type %s\nSupported types: switch\n", type.value().c_str());
    }
}

void AddCluster(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device! Run new-device\n");
        return;
    }
    std::string cluster_name;
    const char * err = Parse(tokens, 0, &cluster_name);
    if (err)
    {
        printf("Error: %s\n", err);
        return;
    }

    for (const auto & cluster : clusters::kKnownClusters)
    {
        if (cluster_name == cluster.name)
        {
            std::unique_ptr<CommonCluster> obj(cluster.ctor(::operator new(cluster.size)));
            DynamicAttributeList dynamic_attrs;
            auto attrs = obj->GetAllAttributes();
            for (auto & attr : attrs)
                dynamic_attrs.Add(attr);
            g_pending->AddCluster(std::move(obj), dynamic_attrs, nullptr, nullptr);
            printf("Added cluster %s/%d\n", cluster.name, cluster.id);
            return;
        }
    }
}

void CancelDevice(const std::vector<std::string> & tokens)
{
    const char * err = Parse(tokens, 0);
    if (err)
    {
        printf("Error: %s\n", err);
        return;
    }

    g_pending.reset(nullptr);
}

void FinishDevice(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device! Run new-device\n");
        return;
    }

    const char * err = Parse(tokens, 0);
    if (err)
    {
        printf("Error: %s\n", err);
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
    uint32_t index;
    const char * err = Parse(tokens, 0, &index);
    if (err)
    {
        printf("Error: %s.\nExpected index of a device\n", err);
        return;
    }
    if (index >= g_devices.size())
    {
        printf("%d is an invalid index\n", index);
        return;
    }

    RemoveDeviceEndpoint(g_devices[index].get());

    for (auto & room : gRooms)
        room.RemoveEndpoint(g_devices[index]->GetEndpointId());

    g_devices[index]      = nullptr;
    g_device_impls[index] = nullptr;
}

void AddType(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device! Run new-device\n");
        return;
    }
    uint32_t type;
    std::optional<uint32_t> version;
    const char * err = Parse(tokens, 0, &type, &version);
    if (err)
    {
        printf("Error: %s.\nExpected: type [version]\n", err);
        return;
    }
    EmberAfDeviceType devType = { (uint16_t) type, (uint8_t) version.value_or(1u) };
    printf("Adding device type %d ver %d\n", devType.deviceId, devType.deviceVersion);

    g_pending->AddDeviceType(devType);
}

DynamicDeviceImpl * FindDevice(int32_t index)
{
    if (index == kPendingDeviceIndex)
    {
        return g_pending.get();
    }
    else if ((uint32_t) index < g_device_impls.size())
    {
        return g_device_impls[index].get();
    }
    else
    {
        return nullptr;
    }
}

Device * FindEndpoint(int32_t index)
{
    if (index >= 0 && (uint32_t) index < g_device_impls.size())
    {
        return g_devices[index].get();
    }
    else
    {
        return nullptr;
    }
}

ClusterImpl * FindCluster(DynamicDeviceImpl * dev, const std::string & clusterId)
{
    uint32_t id;
    const char * start = clusterId.data();
    const char * end   = start + clusterId.size();
    if (std::from_chars(start, end, id).ptr != end)
    {
        id = 0;
        for (const auto & c : clusters::kKnownClusters)
        {
            if (clusterId == c.name)
            {
                id = c.id;
                break;
            }
        }
        if (!id)
            return nullptr;
    }

    for (auto * c : dev->clusters())
    {
        if (c->GetClusterId() == id)
        {
            return c;
        }
    }
    return nullptr;
}

const EmberAfAttributeMetadata * FindAttrib(ClusterImpl * cluster, uint32_t attrId)
{
    for (auto & attr : cluster->GetAllAttributes())
    {
        if (attr.attributeId == attrId)
        {
            return &attr;
        }
    }
    return nullptr;
}

void ParseValue(std::vector<uint8_t> * data, uint16_t size, const std::string & str, EmberAfAttributeType type)
{
    switch (type)
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        if (str.size() >= size)
            return;
        data->resize(size);
        memcpy(data->data() + 1, str.data(), str.size());
        data->data()[0] = (uint8_t) str.size();
        break;
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE: {
        if (str.size() >= size)
            return;
        data->resize(size);
        memcpy(data->data() + 2, str.data(), str.size());
        (*data)[0]      = (uint8_t) (size & 0xFF);
        (*data)[1]      = (uint8_t) ((size >> 8) & 0xFF);
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
    uint32_t attrId;
    int32_t index;
    std::string clusterId;
    std::string value;
    const char * err = Parse(tokens, 0, &index, &clusterId, &attrId, &value);
    if (err)
    {
        printf("Error: %s.\nExpected: deviceIndex clusterId attributeId value\nUse deviceIndex -1 for pending device.\n", err);
        return;
    }

    DynamicDeviceImpl * dev = FindDevice(index);
    if (!dev)
    {
        printf("Could not find device at index %d\n", index);
        return;
    }

    ClusterImpl * cluster = FindCluster(dev, clusterId);
    if (!cluster)
    {
        printf("Device does not implement cluster %s\nSupported clusters: ", clusterId.c_str());
        for (auto * c : dev->clusters())
        {
            printf("%d ", c->GetClusterId());
        }
        printf("\n");
        return;
    }

    const EmberAfAttributeMetadata * attr = FindAttrib(cluster, attrId);
    if (!attr)
    {
        printf("Cluster does not implement atttr %d\nSupported attributes: ", attrId);
        for (auto attrMeta : cluster->GetAllAttributes())
        {
            printf("%d ", attrMeta.attributeId);
        }
        printf("\n");
        return;
    }

    std::vector<uint8_t> data;
    ParseValue(&data, attr->size, value, attr->attributeType);
    if (data.size() > attr->size)
    {
        printf("Data is too large. Maximum size %d bytes\n", attr->size);
        return;
    }

    cluster->WriteFromBridge(attrId, data.data());
}

void AddRoom(const std::vector<std::string> & tokens)
{
    if (tokens.size() != 1)
    {
        printf("Expected exactly one string\n");
        return;
    }
    auto room = FindRoom(tokens[0]);
    if (room)
    {
        printf("Room already exists\n");
        return;
    }
}

void DelRoom(const std::vector<std::string> & tokens)
{
    if (tokens.size() != 1)
    {
        printf("Expected exactly one string\n");
        return;
    }
    auto room = FindRoom(tokens[0]);
    if (!room)
    {
        printf("No such room\n");
        return;
    }

    room->SetName(std::string());
    while (room->GetEndpointListSize() > 0)
        room->RemoveEndpoint(*room->GetEndpointListData());
}

void AddToRoom(const std::vector<std::string> & tokens)
{
    uint32_t index;
    std::string room;
    const char * err = Parse(tokens, 0, &room, &index);
    if (err)
    {
        printf("Error: %s.\nExpected: room index.\n", err);
        return;
    }
    auto roomPtr = FindRoom(tokens[0]);
    if (!roomPtr)
    {
        printf("No such room\n");
        return;
    }
    auto dev = FindEndpoint(index);
    if (!dev)
    {
        printf("No such device\n");
        return;
    }
    roomPtr->AddEndpoint(dev->GetEndpointId());
}

void DelFromRoom(const std::vector<std::string> & tokens)
{
    uint32_t index;
    std::string room;
    const char * err = Parse(tokens, 0, &room, &index);
    if (err)
    {
        printf("Error: %s.\nExpected: room index.\n", err);
        return;
    }
    auto roomPtr = FindRoom(tokens[0]);
    if (!roomPtr)
    {
        printf("No such room\n");
        return;
    }
    auto dev = FindEndpoint(index);
    if (!dev)
    {
        printf("No such device\n");
        return;
    }
    roomPtr->RemoveEndpoint(dev->GetEndpointId());
}

void SetParentId(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device! Run new-device\n");
        return;
    }

    int32_t index;
    const char * err = Parse(tokens, 0, &index);
    if (err)
    {
        printf("Error: %s.\nExpected: deviceIndex\n", err);
        return;
    }
    auto dev = FindEndpoint(index);
    if (!dev)
    {
        printf("No such device\n");
        return;
    }
    g_pending->SetParentEndpointId(dev->GetEndpointId());
}

void Help(const std::vector<std::string> & tokens);

struct Command
{
    const char * name;
    const char * desc;
    void (*fn)(const std::vector<std::string> & tokens);
} const commands[] = {
    { "help", "", &Help },
    { "new-device", "[type] - Begin adding a new device. Add device types and clusters, then finish", &NewDevice },
    { "del-device", "index - Remove a device at the specified index", &RemoveDevice },
    { "add-cluster", "name - Add a cluster by name, eg FixedLabel", &AddCluster },
    { "finish-device", "Finish the currently pending device. Clusters and device types are immutable.", &FinishDevice },
    { "cancel-device", "Abort adding a new device", &CancelDevice },
    { "add-type", "id [version] - Add a device type by ID, optionally with a version", &AddType },
    { "set", "deviceIndex clusterId attributeId value - Set a value. deviceIndex -1 for adding device", &SetValue },
    { "set-parent-id", "deviceIndex - Set the parent endpoint ID to the specified device", &SetParentId },
    { "add-room", "name - Add a new named room", &AddRoom },
    { "del-room", "name - Remove an existing named room", &DelRoom },
    { "add-to-room", "name index - Add a device to a named room", &AddToRoom },
    { "del-from-room", "name index - Remove a device from a named room", &DelFromRoom },
};

void Help(const std::vector<std::string> & tokens)
{
    for (auto & cmd : commands)
    {
        printf("%s %s\n", cmd.name, cmd.desc);
    }
}

void ProcessLine(std::vector<std::string> & tokens)
{
    for (auto & cmd : commands)
    {
        if (tokens[0] == cmd.name)
        {
            tokens.erase(tokens.begin());
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
