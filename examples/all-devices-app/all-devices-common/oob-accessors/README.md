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
        +Register(accessor: unique_ptr~OOBAccessor~) CHIP_ERROR
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) CHIP_ERROR
    }
    class OOBAccessor {
        <<Interface>>
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) optional~CHIP_ERROR~*
    }
    class BooleanStateOOBAccessor {
        +HandleAction(actionName: CharSpan, tlvBuffer: ByteSpan) optional~CHIP_ERROR~
    }
    class BooleanStateCluster {
    }

    class NamedPipeDispatcher["NamedPipe::Dispatcher"] {
        +DispatchJson(json: Json::Value) CHIP_ERROR
    }

    OOBAccessorRegistry "1" *-- "many" OOBAccessor
    BooleanStateOOBAccessor --|> OOBAccessor
    BooleanStateOOBAccessor "1" --> "1" BooleanStateCluster : references
    NamedPipeDispatcher --> OOBAccessorRegistry : uses
```
