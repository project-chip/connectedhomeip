# Out-of-Band (OOB) Control and Simulation Accessors

This directory contains the Out-of-Band (OOB) control and simulation framework
for the `all-devices-app`.

The framework provides a generic interface (backdoor) for simulating physical
events or writing to read-only attributes on simulated devices, decoupling the
core device logic from specific transport protocols (like Pigweed RPC or Shell
commands).

## Architecture Overview

The framework consists of a central `OOBAccessorRegistry` that manages a list of
`OOBAccessor` instances. External interfaces (such as Pigweed RPC services or
CLI shell handlers) route requests through the registry, which forwards them to
the appropriate accessor based on the target Endpoint ID.

```mermaid
classDiagram
    class OOBAccessorRegistry {
        -mAccessors: IntrusiveList~OOBAccessor~
        +Register(accessor: OOBAccessor&) void
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) CHIP_ERROR
    }
    class OOBAccessor {
        <<Interface>>
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) std::optional~CHIP_ERROR~*
    }
    class BooleanStateSensorAccessor {
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) std::optional~CHIP_ERROR~
    }
    class BooleanStateSensorDevice {
    }

    class PigweedAttributeAccessor {
        <<Pigweed Interceptor>>
        +Write(path: ConcreteDataAttributePath, reader: TLVReader)
    }
    class ShellCommandHandler {
        +HandleCommand(args)
    }

    OOBAccessorRegistry "1" *-- "many" OOBAccessor
    BooleanStateSensorAccessor --|> OOBAccessor
    BooleanStateSensorAccessor "1" --> "1" BooleanStateSensorDevice : references
    PigweedAttributeAccessor --> OOBAccessorRegistry : uses
    ShellCommandHandler --> OOBAccessorRegistry : uses
```
