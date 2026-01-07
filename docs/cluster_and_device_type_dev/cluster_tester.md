# ClusterTester Helper Class

The `ClusterTester` class is a C++ helper designed to simplify unit testing for
Matter clusters implementing the `ServerClusterInterface`. It abstracts away the
complexity of TLV encoding/decoding, path construction, and memory management
for data views.

**Header Location:**
[`src/app/server-cluster/testing/ClusterTester.h`](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/testing/ClusterTester.h)

## Why use ClusterTester?

1. **Automatic TLV Handling**: Encodes C++ structures to TLV for writes/commands
   and decodes TLV to C++ structures for reads/responses automatically.
2. **Memory Safety**: For attributes that return views (like `CharSpan` or
   `List`), `ClusterTester` maintains ownership of the underlying TLV data. This
   allows you to inspect the data safely within the test scope without worrying
   about dangling pointers.
3. **Type Safety**: Leverages generated data model types (e.g.,
   `Attributes::MyAttr::TypeInfo`) to ensure arguments match the spec.

---

## Basic Usage Example

For simple clusters that do not require fabric context (like **Boolean State**),
the setup is straightforward. You simply wrap your cluster instance with the
tester.

### Setup

```cpp
#include <app/server-cluster/testing/ClusterTester.h>
#include <app/clusters/boolean-state-server/BooleanStateCluster.h>

class TestBooleanStateCluster : public ::testing::Test
{
    // ... (Setup memory and context) ...

    BooleanStateCluster booleanState{kRootEndpointId};

    // Initialize the tester with the cluster instance
    chip::Testing::ClusterTester tester{booleanState};

    void SetUp() override {
        booleanState.Startup(testContext.Get());
    }
};

```

### Reading Attributes

The tester allows reading attributes directly into their `DecodableType`s.

```cpp
TEST_F(TestBooleanStateCluster, ReadAttributeTest)
{
    // 1. Read a simple integer attribute
    uint16_t revision{};
    ASSERT_EQ(tester.ReadAttribute(Globals::Attributes::ClusterRevision::Id, revision), CHIP_NO_ERROR);

    // 2. Read a bitmap
    uint32_t features{};
    ASSERT_EQ(tester.ReadAttribute(FeatureMap::Id, features), CHIP_NO_ERROR);

    // 3. Read a boolean
    bool stateValue{};
    ASSERT_EQ(tester.ReadAttribute(StateValue::Id, stateValue), CHIP_NO_ERROR);
}

```

---

## Advanced Usage: Fabric-Scoped Clusters

For complex clusters involving Access Control or Fabric scoping (like **Group
Key Management**), the `ClusterTester` integrates with `FabricTestFixture`.

### Setup with Fabric Context

When testing fabric-scoped clusters, you need to manage the `FabricTestFixture`
within your test class. You do not strictly need to pass the fixture to the
`ClusterTester` constructor, but you **must** set the active fabric index on the
tester before performing operations.

```cpp
class TestGroupKeyManagementCluster : public ::testing::Test
{
    TestServerClusterContext mTestContext;
    FabricTestFixture fabricHelper{ &mTestContext.StorageDelegate() };
    GroupKeyManagementCluster mCluster{ fabricHelper.GetFabricTable() };

    // Initialize tester with just the cluster
    ClusterTester tester{ mCluster};

    void SetUp() override
    {
        // ... (Startup cluster) ...

        // Initialize the test fabric
        fabricHelper.SetUpTestFabric(kTestFabricIndex);

        // IMPORTANT: Set the fabric index for the tester.
        // All subsequent writes/commands will appear to come from this fabric.
        tester.SetFabricIndex(kTestFabricIndex);
    }
};

```

### Writing Attributes (Lists & Structs)

The tester handles complex types like Lists and Structs automatically.

```cpp
TEST_F(TestGroupKeyManagementCluster, TestWriteGroupKeyMapAttribute)
{
    // Create a vector of Structs
    std::vector<GroupKeyManagement::Structs::GroupKeyMapStruct::Type> keys;

    GroupKeyManagement::Structs::GroupKeyMapStruct::Type key;
    key.groupId = 0x1234;
    key.groupKeySetID = 1;
    key.fabricIndex = kTestFabricIndex;
    keys.push_back(key);

    // Wrap in DataModel::List
    auto listToWrite = app::DataModel::List<const GroupKeyManagement::Structs::GroupKeyMapStruct::Type>(keys.data(), keys.size());

    // Write the attribute
    // The tester automatically handles EncodeForWrite for fabric-scoped attributes
    CHIP_ERROR err = tester.WriteAttribute(GroupKeyManagement::Attributes::GroupKeyMap::Id, listToWrite).GetUnderlyingError();

    ASSERT_EQ(err, CHIP_NO_ERROR);
}

```

### Invoking Commands

Use the `Invoke` method with the generated Command Request structures.

```cpp
TEST_F(TestGroupKeyManagementCluster, TestKeySetWriteCommand)
{
    // 1. Prepare the Request Struct
    GroupKeyManagement::Commands::KeySetWrite::Type requestData;
    requestData.groupKeySet.groupKeySetID = kTestKeySetId;
    requestData.groupKeySet.groupKeySecurityPolicy = GroupKeyManagement::GroupKeySecurityPolicyEnum::kTrustFirst;
    requestData.groupKeySet.epochStartTime0 = kStartTime;
    // ... populate other fields ...

    // 2. Invoke the command
    // The CommandId is deduced automatically from the request type,
    // or can be passed explicitly: tester.Invoke(CommandId, request)
    auto result = tester.Invoke(requestData);

    // 3. Verify Result
    EXPECT_TRUE(result.IsSuccess());
}

```

---

## Key Behaviors & Testing Scenarios

Refer to the
[ClusterTester.h header](https://github.com/project-chip/connectedhomeip/blob/master/src/app/server-cluster/testing/ClusterTester.h)
for the full class definition. This section highlights specific internal
behaviors that simplify testing.

### Memory Management (Reads)

When using `ReadAttribute`, `ClusterTester` automatically manages the underlying
TLV data for you.

-   **Views & Lists**: If you read a type that contains views (like `CharSpan`,
    `ByteSpan`, or `DecodableList`), the tester buffers the TLV data internally.
-   **Lifetime Safety**: This data remains valid for the lifetime of the
    `ClusterTester` instance, allowing you to safely iterate over lists or
    inspect spans without worrying about the buffer being deallocated
    immediately after the read call.

### Fabric Scoping (Writes)

`WriteAttribute` abstracts away the complexity of fabric-sensitive writes.

-   **Auto-Encoding**: It uses C++ introspection to detect if a struct is
    fabric-scoped. If so, it automatically uses `EncodeForWrite`; otherwise, it
    uses standard `Encode`.
-   **Subject Descriptor**: It automatically applies the Access Control subject
    descriptor corresponding to the fabric index set via `SetFabricIndex()`.

### Command Results (Invoke)

The `Invoke` wrapper simplifies the distinction between Status responses and
Data responses.

-   **Unified Result**: It returns an `InvokeResult` that acts as a single
    container for both success/failure status and the decoded response payload
    (if the command returns data).
-   **Synchronous execution**: The helper is designed for unit tests where
    command execution is expected to be synchronous.

### Testing Side Effects (Events & Reporting)

Often a test needs to verify that a command or write caused a side effect, such
as emitting an event or marking an attribute as "dirty" (ready for reporting).

**Verifying Event Generation:**

```cpp
// 1. Perform action (e.g. write attribute)
tester.WriteAttribute(MyAttribute::Id, newValue);

// 2. Check the event generator from the underlying context
auto event = tester.GetNextGeneratedEvent();
if (event.has_value()) {
   // Decode event data...
}

```

**Verifying Dirty Attributes:**

```cpp
// 1. Perform action
tester.Invoke(myCommand);

// 2. Inspect the dirty list
auto & dirtyList = tester.GetDirtyList();
bool markedDirty = std::any_of(dirtyList.begin(), dirtyList.end(),
    [](const auto & path){ return path.mAttributeId == MyAttribute::Id; });

```
