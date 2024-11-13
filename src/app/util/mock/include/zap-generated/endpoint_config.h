// Number of fixed endpoints
#define FIXED_ENDPOINT_COUNT (3)

// This is one of the defines that endpoint_config generally has. In particular this
// can (and is) used to size I/O buffers for attribute reads (like ember-io-storage.cpp).
//
// Define this to a large number, matching what an all-clusters-app has.
#define ATTRIBUTE_LARGEST (1003)
