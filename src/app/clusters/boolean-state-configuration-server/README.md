# Boolean State Configuration Server Cluster

This cluster is used to configure a boolean sensor, including optional state
change alarm features and configuration of the sensitivity level associated with
the sensor.

The functionality of the cluster is documented in section 1.8 of the Matter
Application Clusters specification.

## Application integration

The application interacts with the cluster via a
`BooleanStateConfiguration::Delegate` instance:

-   allows additional handling when commands are received
-   provides type-safe per-attribute change callbacks:
    -   `OnCurrentSensitivityLevelChanged(uint8_t)` - called when
        CurrentSensitivityLevel changes
    -   `OnAlarmsActiveChanged(BitMask<AlarmModeBitmap>)` - called when
        AlarmsActive changes
    -   `OnAlarmsSuppressedChanged(BitMask<AlarmModeBitmap>)` - called when
        AlarmsSuppressed changes
    -   `OnAlarmsEnabledChanged(BitMask<AlarmModeBitmap>)` - called when
        AlarmsEnabled changes
    -   `OnSensorFaultChanged(BitMask<SensorFaultBitmap>)` - called when
        SensorFault changes

## CodegenIntegration / updates from ember `PostAttributeChangeCallback`

The cluster is implemented as a code driven cluster. For compatibility with code
generation and the ember framework, a `CodegenIntegration.h/cpp` file exists,
which pre-allocates and initializes the cluster according to code-generation
settings.

Usage Notes:

-   existing API was preserved in `CodegenIntegration.h` with the added
    requirement that the API calls _MUST_ be called only after server
    initialization (i.e. after the clusters have been created.)
-   The `Delegate` provides type-safe per-attribute change callbacks (e.g.
    `OnCurrentSensitivityLevelChanged(uint8_t)`) that are called whenever the
    corresponding attributes change, whether via attribute writes, command
    handlers, or server APIs such as `SetAlarmsActive()` or
    `GenerateSensorFault()`. Use these instead of implementing functionality in
    `PostAttributeChangeCallback`.
