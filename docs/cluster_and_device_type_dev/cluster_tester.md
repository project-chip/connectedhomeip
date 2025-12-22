# ClusterTester Helper Class

The `ClusterTester` class is a C++ helper designed to simplify unit testing for
Matter clusters implementing the `ServerClusterInterface`. It abstracts away the
complexity of TLV encoding/decoding, path construction, and memory management
for data views.

**Header Location:** `src/app/server-cluster/testing/ClusterTester.h`

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

When testing fabric-scoped clusters, pass the `FabricTestFixture` to the tester
and set the active Fabric Index.

```cpp
class TestGroupKeyManagementCluster : public ::testing::Test
{
    TestServerClusterContext mTestContext;
    FabricTestFixture fabricHelper{ &mTestContext.StorageDelegate() };
    GroupKeyManagementCluster mCluster{ fabricHelper.GetFabricTable() };

    // Initialize tester with both cluster and fabric helper
    ClusterTester tester{ mCluster, &fabricHelper };

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

## API Reference

### `ReadAttribute`

Reads an attribute value from the cluster.

-   **Signature**:
    `ActionReturnStatus ReadAttribute(AttributeId attr_id, T & out)`
-   **Behavior**:
-   Constructs a read path.
-   Reads and decodes the TLV.
-   **Crucial**: If `T` contains views (e.g., `Span` or `List`), the internal
    TLV buffer remains allocated within the `ClusterTester` instance. You can
    safely iterate over the list immediately.

### `WriteAttribute`

Writes a value to the cluster.

-   **Signature**:
    `ActionReturnStatus WriteAttribute(AttributeId attr, const T & value)`
-   **Behavior**:
-   Detects if `T` is fabric-scoped.
-   Uses `EncodeForWrite` if fabric-scoped, or standard `Encode` otherwise.
-   Uses the subject descriptor from the current `SetFabricIndex`.

### `Invoke`

Invokes a command.

-   **Signature**:
    `InvokeResult<ResponseType> Invoke(const RequestType & request)`
-   **Returns**: An `InvokeResult` struct containing:
-   `status`: The `ActionReturnStatus` (success/failure).
-   `response`: An `std::optional<ResponseType>` containing the decoded response
    struct (if the command returns data).

### `GetDirtyList` & `GetNextGeneratedEvent`

Accessors for inspecting the `TestServerClusterContext`.

-   **Usage**: Useful for verifying that a command or write caused a side effect
    (like marking an attribute dirty for reporting or emitting an event).

```cpp
// Check if an attribute was marked dirty
auto & dirtyList = tester.GetDirtyList();

// Check if an event was generated
auto event = tester.GetNextGeneratedEvent();
if(event.has_value()) {
   // Decode event data...
}

```
