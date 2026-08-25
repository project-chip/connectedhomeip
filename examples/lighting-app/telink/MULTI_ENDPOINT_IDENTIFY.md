# Matter Lighting App Multi-Endpoint Configuration Guide

## 1. Two-Endpoint Demo Guide

### 1.1 Endpoint Definition

| Endpoint ID | Device Type                         | Supported Clusters                                |
| ----------- | ----------------------------------- | ------------------------------------------------- |
| Endpoint 1  | Dimmable Light (Main Light)         | Identify / On/Off / Level Control                 |
| Endpoint 2  | Color Temperature Light (CCT Light) | Identify / On/Off / Level Control / Color Control |

---

### 1.2 Add Endpoint 2 in ZAP Configuration

Edit `examples/lighting-app/lighting-common/lighting-app.zap`.

The original lighting app contains Endpoint 0 and Endpoint 1.  
For the two-endpoint demo, add Endpoint 2 using the same lighting endpoint type
as Endpoint 1:

```json
{
    "endpointTypeName": "MA-dimmablelight",
    "endpointTypeIndex": 1,
    "profileId": 259,
    "endpointId": 2,
    "networkId": 0,
    "parentEndpointIdentifier": null
}
```

Then regenerate the Matter data model:

```bash
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap
```

Verify that `examples/lighting-app/lighting-common/lighting-app.matter` contains
`endpoint 2` with the Identify server cluster.

> **Important:** `MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT` can still be
> `1` when Endpoint 1 and Endpoint 2 reuse the same generated endpoint type. Do
> not use this macro as the number of fixed Identify endpoint instances.
>
> The fixed endpoint topology is generated in `zap-generated/endpoint_config.h`.
> For Endpoint 0 + Endpoint 1 + Endpoint 2, `FIXED_ENDPOINT_COUNT` should be
> `3`.

---

### 1.3 chip-tool Validation

Identify cluster read/write works correctly:

```bash
# Read IdentifyTime on endpoint 1
./chip-tool identify read identify-time 1 1

# Read IdentifyTime on endpoint 2
./chip-tool identify read identify-time 1 2
```

---

## 2. Extending to 6 Endpoints (6 Lights)

If you need to support 6 independent lights (Endpoint 1 through Endpoint 6),
follow these steps:

### 2.1 Step 1: Add Endpoints in ZAP Configuration

Edit `examples/lighting-app/lighting-common/lighting-app.zap`:

1. Add Endpoint 3 through Endpoint 6
2. Add the required clusters for each endpoint:
    - Identify (**mandatory** for all lighting endpoints per Matter spec)
    - On/Off
    - Level Control
    - Color Control (if supporting dimmable/CCT/color lights)
3. Regenerate ZAP code:

```bash
./scripts/tools/zap/generate.py examples/lighting-app/lighting-common/lighting-app.zap
```

After adding Endpoint 3 through Endpoint 6, verify that the generated model
contains Endpoint 0 through Endpoint 6.

For this demo topology, the generated `zap-generated/endpoint_config.h` should
contain:

```cpp
#define FIXED_ENDPOINT_COUNT (7)
```

`MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT` may still remain `1` if all
six lighting endpoints reuse the same endpoint type.

---

### 2.2 Step 2: Update Identify Instance Initialization

**File to modify**: `examples/platform/telink/common/src/AppTaskCommon.cpp`

Locate the Identify instance definitions (around lines 291-312) and extend as
follows.

Make sure the generated fixed-endpoint configuration is available in this file:

```cpp
#include <zap-generated/endpoint_config.h>
```

`FIXED_ENDPOINT_COUNT` is used instead of
`MATTER_DM_IDENTIFY_CLUSTER_SERVER_ENDPOINT_COUNT`, because multiple fixed
endpoints can reuse the same generated endpoint type.

```cpp
#ifndef IDENTIFY_CLUSTER_DISABLED

// ==============================================
// List of all lighting endpoint IDs. Modify this when adding/removing endpoints.
// Endpoint 1 keeps the existing Telink kExampleEndpointId definition.
// ==============================================
constexpr chip::EndpointId kLightEndpointIds[] = { kExampleEndpointId, 2, 3, 4, 5, 6 };

void OnIdentifyTriggerEffect(Identify * identify)
{
    chip::EndpointId endpoint = identify->mCluster.Cluster().GetPaths()[0].mEndpointId;
    ChipLogProgress(Zcl, "OnIdentifyTriggerEffect for endpoint %u, effect: %u", endpoint,
                    static_cast<unsigned>(identify->mCurrentEffectIdentifier));
    AppTaskCommon::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

// ==============================================
// Create Identify instance for each fixed lighting endpoint.
// FIXED_ENDPOINT_COUNT includes Endpoint 0.
// ==============================================
Identify sIdentify1 = {
    kLightEndpointIds[0], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};

#if FIXED_ENDPOINT_COUNT > 2
Identify sIdentify2 = {
    kLightEndpointIds[1], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

// ========== Add Endpoint 3 ~ 6 ==========
#if FIXED_ENDPOINT_COUNT > 3
Identify sIdentify3 = {
    kLightEndpointIds[2], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

#if FIXED_ENDPOINT_COUNT > 4
Identify sIdentify4 = {
    kLightEndpointIds[3], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

#if FIXED_ENDPOINT_COUNT > 5
Identify sIdentify5 = {
    kLightEndpointIds[4], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

#if FIXED_ENDPOINT_COUNT > 6
Identify sIdentify6 = {
    kLightEndpointIds[5], AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler, Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

#endif
```

> **Important Note**: Why does Identify require per-endpoint instances while
> On/Off does not?
>
> - On/Off/Level/Color Control clusters use **RAM/NVM static storage** - ZAP
>   automatically allocates attribute storage for all endpoints during code
>   generation.
> - Identify cluster uses **EXTERNAL_STORAGE** - it requires an active C++
>   object instance to manage the countdown timer, effect callbacks, and dynamic
>   state. Each endpoint needs its own instance.
>
> `FIXED_ENDPOINT_COUNT` includes Endpoint 0, so:
>
> - EP0 + EP1 -> `FIXED_ENDPOINT_COUNT == 2`
> - EP0 + EP1 + EP2 -> `FIXED_ENDPOINT_COUNT == 3`
> - EP0 + EP1 ... EP6 -> `FIXED_ENDPOINT_COUNT == 7`

---

### 2.3 Step 3: Update Identify Callbacks for Independent LEDs/PWMs

In the same file, locate the `IdentifyStartHandler` and `IdentifyStopHandler`
functions (around lines 676-733). Add PWM/LED configurations for endpoints 3-6
in the switch-case:

```cpp
event.Handler = [](AppEvent * event) {
    chip::EndpointId ep = static_cast<chip::EndpointId>(reinterpret_cast<uintptr_t>(event->TimerEvent.Context));
    ChipLogProgress(Zcl, "OnIdentifyStart for endpoint %u", ep);

    PwmManager::EAppPwm pwm = PwmManager::EAppPwm_Indication;
    switch (ep)
    {
    case kExampleEndpointId:
        pwm = PwmManager::EAppPwm_Indication;  // Endpoint 1 status indicator
        break;
    case 2:
        pwm = PwmManager::EAppPwm_Indication;  // Endpoint 2 status indicator (can change to dedicated PWM)
        break;
    // ========== Add endpoints 3~6 ==========
    case 3:
        pwm = PwmManager::EAppPwm_Red;         // Select appropriate PWM channel based on hardware connections
        break;
    case 4:
        pwm = PwmManager::EAppPwm_Green;
        break;
    case 5:
        pwm = PwmManager::EAppPwm_Blue;
        break;
    case 6:
        pwm = PwmManager::EAppPwm_Indication;
        break;
    default:
        break;
    }
    PwmManager::getInstance().setPwmBlink(pwm, kIdentifyBlinkRateMs, kIdentifyBlinkRateMs);
};
```

Make the same changes to the switch-case in `IdentifyStopHandler`:

- For start handler: `setPwmBlink()` to start blinking
- For stop handler: `setPwm(pwm, false)` to stop blinking

---

### 2.4 Step 4: On/Off/Level/Color Control Callbacks

No additional instances are needed for On/Off, Level Control, or Color Control
clusters. These clusters use static RAM storage and automatically work across
all endpoints once configured in ZAP.

If you need independent LED control for each light, update the switch-case in
existing callbacks like `PostAttributeChangeCallback` or
`OnOffClusterSetValueCallback` to handle each endpoint:

```cpp
// Example: In your On/Off attribute change callback
void AppTaskCommon::OnOffAttributeChangedCallback(chip::EndpointId endpoint, bool onOff)
{
    switch (endpoint)
    {
    case 1:
        // Control main light relay/PWM
        break;
    case 2:
        // Control CCT light relay/PWM
        break;
    case 3:
        // Control light 3
        break;
    // ... add cases 4-6
    }
}
```

---

## 3. Alternative: Array Initialization

For cleaner code with 4 or more endpoints, you can use array initialization
instead of separate variables:

```cpp
#ifndef IDENTIFY_CLUSTER_DISABLED

#if FIXED_ENDPOINT_COUNT > 6

#define EP_CNT 6  // Endpoint 1 through Endpoint 6

void OnIdentifyTriggerEffect(Identify * identify)
{
    chip::EndpointId endpoint = identify->mCluster.Cluster().GetPaths()[0].mEndpointId;
    ChipLogProgress(Zcl, "OnIdentifyTriggerEffect for endpoint %u, effect: %u", endpoint,
                    static_cast<unsigned>(identify->mCurrentEffectIdentifier));
    AppTaskCommon::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

// Single array initialization - all instances in one place.
// Use this form only for a dedicated build where Endpoint 1 through Endpoint 6 exist.
Identify sIdentifyList[EP_CNT] = {
    {kExampleEndpointId, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
    {2, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
    {3, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
    {4, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
    {5, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
    {6, AppTask::IdentifyStartHandler, AppTask::IdentifyStopHandler,
     Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, OnIdentifyTriggerEffect},
};

#endif // FIXED_ENDPOINT_COUNT > 6
#endif // IDENTIFY_CLUSTER_DISABLED
```
