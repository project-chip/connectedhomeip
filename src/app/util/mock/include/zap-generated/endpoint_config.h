// Number of fixed endpoints
#define FIXED_ENDPOINT_COUNT (3)

// This is one of the defines that endpoint_config generally has. In particular this
// can (and is) used to size I/O buffers for attribute reads (like ember-io-storage.cpp).
//
// Define this to a large number, matching what an all-clusters-app has.
#define ATTRIBUTE_LARGEST (1003)

// Attribute qualities the attribute masks have no room for: MockAttributeId(1) of MockClusterId(5) is Fixed and
// MockAttributeId(2) of MockClusterId(5) is Changes Omitted.
#define GENERATED_FIXED_ATTRIBUTES { 0xFFF1FC05, 0xFFF10001 },
#define GENERATED_CHANGES_OMITTED_ATTRIBUTES { 0xFFF1FC05, 0xFFF10002 },
