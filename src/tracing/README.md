# Matter Tracing

This library provides a runtime-configurable tracing and logging infrastructure
for matter. The overall tracing subsystem is enabled or disabled via the
`matter_enable_tracing_support` build setting; when disabled all tracing code is
compiled out completely.

## Types of data

### Low-Level Tracing

This is mostly intended for following execution flow and measuring time spent
for various operations. These traces are either

-   **scoped** where separate begin and end events are emitted, or
-   **instant** where a single notable event is emitted, representing a point in
    time of a notable event.

The low-level tracing API is provided via `MATTER_TRACE_*` macros and is
thread-safe.

Tracing labels and instant values MUST be constant strings as some backends rely
on that property for caching (e.g. pw_trace would do tokenization and perfetto
marks them as `perfetto::StaticString`)

The tracing implementation is selected via the `matter_trace_config` build
setting; the selected implementation directly provides the implementations of
the tracing macros, allowing these to be bridged to a platform tracing mechanism
with very low overhead.

A `multiplexed` configuration is available that routes tracing macros through
the runtime backend registry used by the higher-level Data Logging / Metrics API
(see below). The `none` configuration compiles out trace macros completely.

### Data Logging / Metrics

Data logging provides the tracing module the opportunity to report input/output
data for matter data processing, as well as high-level operational metrics.

The data logging is generally limited in count and covers:

-   **Messages**, specifically sent matter requests and received matter
    responses
-   **DNSSD operations** as they are a core component of matter, specifically
    attempts to discover nodes as well as when a node is discovered or fails
    discovery.
-   **Metrics**, for tracking operational events with associated values and
    error codes

The high-level data logging / metrics API consists of various `MATTER_LOG_*`
macros (e.g. `MATTER_LOG_MESSAGE_SEND`, `MATTER_LOG_NODE_DISCOVERED`,
`MATTER_LOG_METRIC`, ...).

Backends for this data logging layer implement the `chip::Tracing::Backend` API
(see `backend.h`), and are registered at runtime via `chip::Tracing::Register()`
(see `registry.h`).

Note that while registration and unregistration of backends must be performed
while the Matter stack lock is being held, data logging itself is thread-safe
(and must be implemented as such by all backends.)
