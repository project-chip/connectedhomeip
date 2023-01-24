#include "Backend.h"
#include "DynamicDevice.h"
#include "main.h"

#include <platform/PlatformManager.h>

#include <algorithm>
#include <charconv>
#include <future>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::unique_ptr<DynamicDevice> g_pending;

// Pseudo-index representing the device being built.
static constexpr int kPendingDeviceIndex = -1;

PropagateWriteCB g_write_cb = [](CommonCluster * cluster, const chip::app::ConcreteDataAttributePath & path,
                                 chip::app::AttributeValueDecoder & decoder) -> CHIP_ERROR {
    CHIP_ERROR ret = cluster->WriteFromBridge(path, decoder);
    printf("Write to ep %d cluster %d attribute %d ret %" CHIP_ERROR_FORMAT "\n", path.mEndpointId, path.mClusterId,
           path.mAttributeId, ret.Format());
    return ret;
};

const char * ParseValue(const std::vector<std::string> & tokens, size_t index, std::string * v)
{
    if (index >= tokens.size())
        return "Missing argument";
    *v = tokens[index];
    return nullptr;
}
const char * ParseValue(const std::vector<std::string> & tokens, size_t index, chip::Optional<std::string> * v)
{
    if (index < tokens.size())
        v->SetValue(tokens[index]);
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
const char * ParseValue(const std::vector<std::string> & tokens, size_t index, chip::Optional<T> * v)
{
    uint32_t temp;
    if (!ParseValue(tokens, index, &temp))
        v->SetValue(temp);
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

    chip::Optional<std::string> type;
    const char * err = Parse(tokens, 0, &type);
    if (err)
    {
        printf("Error: %s\nExpected [prebuilt-device-type]\n", err);
    }
    else if (!type.HasValue())
    {
        g_pending = std::make_unique<DynamicDevice>();
    }
    else
    {
        printf("Unknown device type %s\nSupported types: switch\n", type.Value().c_str());
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

    auto c = CreateCluster(cluster_name.c_str());
    if (c)
    {
        g_pending->AddCluster(
            std::make_unique<DynamicCluster>(std::move(c), c->GetIncomingCommandList(), c->GetOutgoingCommandList()));
    }
    else
    {
        printf("No such cluster '%s'\n", cluster_name.c_str());
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

    int ep = AddDevice(std::move(g_pending));

    if (ep < 0)
    {
        printf("Failed to add device\n");
    }
    else
    {
        printf("Added device at index %d\n", ep);
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
    if (!RemoveDeviceAt(index))
    {
        printf("%d is an invalid index\n", index);
    }
}

void AddType(const std::vector<std::string> & tokens)
{
    if (!g_pending)
    {
        printf("Expected pending device! Run new-device\n");
        return;
    }
    uint32_t type;
    chip::Optional<uint32_t> version;
    const char * err = Parse(tokens, 0, &type, &version);
    if (err)
    {
        printf("Error: %s.\nExpected: type [version]\n", err);
        return;
    }
    EmberAfDeviceType devType = { (uint16_t) type, (uint8_t) version.ValueOr(1u) };
    printf("Adding device type %d ver %d\n", devType.deviceId, devType.deviceVersion);

    g_pending->AddDeviceType(devType);
}

DynamicDevice * FindDevice(int32_t index)
{
    if (index == kPendingDeviceIndex)
    {
        return g_pending.get();
    }
    else if ((uint32_t) index < g_device_impls.size())
    {
        return g_device_impls[(uint32_t) index].get();
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
        return g_devices[(uint32_t) index].get();
    }
    else
    {
        return nullptr;
    }
}

ClusterInterface * FindCluster(DynamicDevice * dev, const std::string & clusterId)
{
    uint32_t id;
    const char * start = clusterId.data();
    const char * end   = start + clusterId.size();
    if (std::from_chars(start, end, id).ptr != end)
    {
        auto r = LookupClusterByName(clusterId.c_str());
        if (r.HasValue())
        {
            return nullptr;
        }
        id = r.Value();
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

const EmberAfAttributeMetadata * FindAttrib(ClusterInterface * cluster, const std::string & attrId)
{
    AttributeInterface * attr = nullptr;
    uint32_t id;
    const char * start = attrId.data();
    const char * end   = start + attrId.size();
    if (std::from_chars(start, end, id).ptr == end)
    {
        attr = cluster->FindAttribute(id);
    }
    if (!attr)
    {
        attr = cluster->FindAttributeByName(chip::CharSpan(attrId.data(), attrId.size()));
    }

    return attr ? &attr->GetMetadata() : nullptr;
}

void ParseValue(std::vector<uint8_t> * data, uint16_t size, const std::string & str, EmberAfAttributeType type)
{
    chip::TLV::TLVWriter wr;
    wr.Init(data->data(), data->size());
    switch (type)
    {
    case ZCL_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_CHAR_STRING_ATTRIBUTE_TYPE:
        wr.PutString(chip::TLV::AnonymousTag(), str.data(), (uint32_t) str.size());
        break;
    case ZCL_LONG_OCTET_STRING_ATTRIBUTE_TYPE:
    case ZCL_LONG_CHAR_STRING_ATTRIBUTE_TYPE:
        wr.PutBytes(chip::TLV::AnonymousTag(), (const uint8_t *) str.data(), (uint32_t) str.size());
        break;
    case ZCL_STRUCT_ATTRIBUTE_TYPE:
        // Writing structs not supported yet
        break;
    case ZCL_SINGLE_ATTRIBUTE_TYPE:
        wr.Put(chip::TLV::AnonymousTag(), (float) atof(str.c_str()));
        break;
    case ZCL_DOUBLE_ATTRIBUTE_TYPE:
        wr.Put(chip::TLV::AnonymousTag(), atof(str.c_str()));
        break;
    case ZCL_INT8S_ATTRIBUTE_TYPE:
    case ZCL_INT16S_ATTRIBUTE_TYPE:
    case ZCL_INT24S_ATTRIBUTE_TYPE:
    case ZCL_INT32S_ATTRIBUTE_TYPE:
    case ZCL_INT40S_ATTRIBUTE_TYPE:
    case ZCL_INT48S_ATTRIBUTE_TYPE:
    case ZCL_INT56S_ATTRIBUTE_TYPE:
    case ZCL_INT64S_ATTRIBUTE_TYPE:
        wr.Put(chip::TLV::AnonymousTag(), (int64_t) strtoll(str.c_str(), nullptr, 10));
        break;

    case ZCL_INT8U_ATTRIBUTE_TYPE:
    case ZCL_INT16U_ATTRIBUTE_TYPE:
    case ZCL_INT24U_ATTRIBUTE_TYPE:
    case ZCL_INT32U_ATTRIBUTE_TYPE:
    case ZCL_INT40U_ATTRIBUTE_TYPE:
    case ZCL_INT48U_ATTRIBUTE_TYPE:
    case ZCL_INT56U_ATTRIBUTE_TYPE:
    case ZCL_INT64U_ATTRIBUTE_TYPE:
        wr.Put(chip::TLV::AnonymousTag(), (uint64_t) strtoll(str.c_str(), nullptr, 10));
        break;

    default:
        // Assume integer
        wr.Put(chip::TLV::AnonymousTag(), (int64_t) strtoll(str.c_str(), nullptr, 10));
        break;
    }
    wr.Finalize();
    data->resize(wr.GetLengthWritten());
}

void SetValue(const std::vector<std::string> & tokens)
{
    std::string attrId;
    int32_t index;
    std::string clusterId;
    std::string value;
    const char * err = Parse(tokens, 0, &index, &clusterId, &attrId, &value);
    if (err)
    {
        printf("Error: %s.\nExpected: deviceIndex clusterId attributeId value\nUse deviceIndex -1 for pending device.\n", err);
        return;
    }

    DynamicDevice * dev = FindDevice(index);
    if (!dev)
    {
        printf("Could not find device at index %d\n", index);
        return;
    }

    ClusterInterface * cluster = FindCluster(dev, clusterId);
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
        printf("Cluster does not implement attr %s\nSupported attributes: ", attrId.c_str());
        for (auto attrMeta : cluster->GetAllAttributes())
        {
            printf("%d ", attrMeta->GetId());
        }
        printf("\n");
        return;
    }

    std::vector<uint8_t> data(attr->size + 64);
    ParseValue(&data, attr->size, value, attr->attributeType);

    chip::TLV::TLVReader rd;
    rd.Init(data.data(), data.size());
    rd.Next();

    if (!cluster->Write(attr->attributeId, rd))
    {
        printf("Write failed\n");
    }
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
    int32_t index;
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
    int32_t index;
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

struct Op
{
    std::vector<std::string> * tokens;
    const Command * command;
    std::promise<void> lock;
};
void ProcessLineOnMatterThread(intptr_t arg)
{
    Op * op = reinterpret_cast<Op *>(arg);
    op->command->fn(*op->tokens);
    op->lock.set_value();
}

void ProcessLine(std::vector<std::string> & tokens)
{
    for (auto & cmd : commands)
    {
        if (tokens[0] == cmd.name)
        {
            tokens.erase(tokens.begin());

            Op op{ &tokens, &cmd };
            chip::DeviceLayer::PlatformMgr().ScheduleWork(&ProcessLineOnMatterThread, reinterpret_cast<intptr_t>(&op));
            // Wait for command completion
            op.lock.get_future().wait();
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
