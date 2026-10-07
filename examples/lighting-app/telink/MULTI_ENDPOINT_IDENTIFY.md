# Matter Lighting App Multi-Endpoint Configuration Guide

## 1. Two-Endpoint Demo Guide

### 1.1 Endpoint Definition

| Endpoint ID | Device Type                         | Supported Clusters                                |
| ----------- | ----------------------------------- | ------------------------------------------------- |
| Endpoint 1  | Dimmable Light (Main Light)         | Identify / On/Off / Level Control                 |
| Endpoint 2  | Color Temperature Light (CCT Light) | Identify / On/Off / Level Control / Color Control |

---

### 1.2 ZAP Configuration

Edit `examples/lighting-app/lighting-common/lighting-app.zap` and add
Endpoint 2.

Use a separate endpoint type for Endpoint 2:

```text
Endpoint 1 -> endpointTypeIndex 1
Endpoint 2 -> endpointTypeIndex 2
```

Enable the Identify server cluster on Endpoint 2 and regenerate the Matter data
model:

```bash
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap
```

For two Identify server endpoints, the generated configuration should contain:

```cpp
#define MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT (2)
```

---

### 1.3 chip-tool Validation

```bash
# Read IdentifyTime on endpoint 1
./chip-tool identify read identify-time 1 1

# Read IdentifyTime on endpoint 2
./chip-tool identify read identify-time 1 2

# Start Identify on endpoint 1
./chip-tool identify identify 5 1 1

# Start Identify on endpoint 2
./chip-tool identify identify 5 1 2
```

---

## 2. Extending to 6 Endpoints (6 Lights)

### 2.1 Step 1: Add Endpoints in ZAP Configuration

Edit `examples/lighting-app/lighting-common/lighting-app.zap`:

1. Add Endpoint 3 through Endpoint 6.
2. Create a separate endpoint type for each endpoint.
3. Add the required clusters:
    - Identify
    - On/Off
    - Level Control
    - Color Control (if required)
4. Regenerate the Matter data model.

For six Identify server endpoints, the generated configuration should contain:

```cpp
#define MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT (6)
```

---

### 2.2 Step 2: Configure Identify Endpoints

**File to modify**: `examples/lighting-app/telink/include/AppTask.h`

For the two-endpoint configuration:

```cpp
#include <zap-generated/gen_config.h>

#if MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT > 1
inline constexpr chip::EndpointId kExampleSecondaryEndpointId = 2;

#define TELINK_APP_IDENTIFY_ENDPOINTS(X) \
    X(kExampleEndpointId)                \
    X(kExampleSecondaryEndpointId)
#endif
```

For additional endpoints, define their IDs and extend
`TELINK_APP_IDENTIFY_ENDPOINTS`:

```cpp
inline constexpr chip::EndpointId kExampleSecondaryEndpointId = 2;
inline constexpr chip::EndpointId kExampleEndpoint3Id          = 3;
inline constexpr chip::EndpointId kExampleEndpoint4Id          = 4;
inline constexpr chip::EndpointId kExampleEndpoint5Id          = 5;
inline constexpr chip::EndpointId kExampleEndpoint6Id          = 6;

#define TELINK_APP_IDENTIFY_ENDPOINTS(X) \
    X(kExampleEndpointId)                \
    X(kExampleSecondaryEndpointId)       \
    X(kExampleEndpoint3Id)               \
    X(kExampleEndpoint4Id)               \
    X(kExampleEndpoint5Id)               \
    X(kExampleEndpoint6Id)
```

The common Telink code creates one `Identify` instance for every endpoint in
this list.

---

### 2.3 Step 3: Identify Callbacks

`IdentifyStartHandler` and `IdentifyStopHandler` use the endpoint ID from the
Identify instance and keep the existing indication PWM behavior.

No additional changes are required for the two-endpoint demo.

---

### 2.4 Step 4: On/Off/Level/Color Control

No additional instances are required for On/Off, Level Control, or Color Control
clusters.

These clusters work on all endpoints configured in ZAP.

---

## 3. Summary

To add more lighting endpoints:

1. Add the endpoints and endpoint types in ZAP.
2. Enable the required clusters.
3. Regenerate the Matter data model.
4. Add the endpoint IDs to `TELINK_APP_IDENTIFY_ENDPOINTS`.
