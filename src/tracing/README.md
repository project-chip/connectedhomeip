# Matter tracing

This library provides a runtime-configurable tracing and logging infrastructure
for matter.

## Types of data

### Tracing

Tracing is mostly intended for following execution flow and measuring time spent
for various operations. They are:

-   _scoped_ where separate begin and end events are emitted _or_

-   _instant_ where a single notable event is emitted, representing a point in
    time of a notable event

Tracing and instant values MUST be constant strings as some backends rely on
that property for caching (e.g. pw_trace would do tokenization and perfetto
marks them as `perfetto::StaticString`)

### Data Logging

Data logging provides the tracing module the opportunity to report input/output
data for matter data processing.

The data logging is generally limited in count and covers:

-   _Messages_, specifically sent matter requests and received matter responses

-   _DNSSD_ operations as they are a core component of matter, specifically
    attempts to discover nodes as well as when a node is discovered or fails
    discovery.

## Usage

Backends are defined by extending `chip::Tracing::Backend` in `backend.h` and
registering it via functions in `registry.h`

Actual usage is controlled using `macros.h` (and for convenience `scope.h`
provides scoped begin/end invocations).

tracing macros can be completely made a `noop` by setting
``matter_enable_tracing_support=false` when compiling.
