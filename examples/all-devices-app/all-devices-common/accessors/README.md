# Out-of-Band (OOB) Control and Simulation Accessors

This directory contains the Out-of-Band (OOB) control and simulation framework
for the `all-devices-app`.

The framework provides a generic interface for simulating physical events or
writing to read-only attributes on simulated devices, decoupling the core device
logic from specific transport protocols (like Pigweed RPC or Shell commands).

## Architecture Overview

The framework consists of a central `AccessorRegistry` that manages a list of
`OOBAccessor` instances. External interfaces (such as Pigweed RPC services or
CLI shell handlers) route requests through the registry, which forwards them to
the appropriate accessor based on the target Endpoint ID.

```mermaid
classDiagram
    class AccessorRegistry {
        -accessors: List~SingleEndpointDeviceAccessor~
        +Register(accessor) CHIP_ERROR
        +SetAttribute(path, decoder)
        +InvokeAction(endpointId, actionName, arguments)
    }
    class OOBAccessor {
        <<Interface>>
        +HandleAction(actionName, tlvBuffer)*
    }

    namespace BooleanStateSensorExample {
        class BooleanStateSensorAccessor {
            +HandleAction(actionName, tlvBuffer)*
        }
        class BooleanStateSensorDevice {
        }
    }

    class PigweedAttributeAccessor {
        <<Pigweed Interceptor>>
        -registry: AccessorRegistry
        +Write(path, tlvReader)
    }
    class ShellCommandHandler {
        -registry: AccessorRegistry
        +HandleCommand(args)
    }

    AccessorRegistry "1" *-- "many" OOBAccessor
    BooleanStateSensorAccessor --|> OOBAccessor
    BooleanStateSensorAccessor "1" --> "1" BooleanStateSensorDevice : references
    PigweedAttributeAccessor --> AccessorRegistry : uses
    ShellCommandHandler --> AccessorRegistry : uses
```
