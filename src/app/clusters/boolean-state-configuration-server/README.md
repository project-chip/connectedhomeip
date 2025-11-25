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
-   provides a post-attribute update callback, which can be used for example to
    react to updates to the `CurrentSensitivityLevel` attribute.

## CodegenIntegration / updates from ember `PostAttributeChangeCallback`

The cluster is implemented as a code driven cluster. For compatibility with code
generation and the ember framework, a `CodegenIntegration.h/cpp` file exists,
which pre-allocates and initializes the cluster according to code-generation
settings.

Usage Notes:

-   existing API was preserved in `CodegenIntegration.h` with the added
    requirement that the API calls _MUST_ be called only after server
    initialization (i.e. after the clusters have been created.)
-   The `Delegate` provides a `OnAttributeChanged` hook that is to be used to
    react/detect when attributes change (e.g. when `CurrentSensitivityLevel::Id`
    was updated). Use this instead of implementing functionality in
    `PostAttributeChangeCallback`.
