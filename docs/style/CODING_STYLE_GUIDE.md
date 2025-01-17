# Coding Style Guide

_Revision 6_ _2024-10-28_

This guide provides a small set of code guidelines used in the SDK. This guide
reflects the currently accepted style practices for the SDK and is subject to
change.

The SDK was seeded from multiple different projects and contains contributions
from many different companies and the SDK code therefore uses several different
coding styles throughout the code base. Stylistically, code should attempt to
conform to the dominant style of the code being modified, while also adhering to
the guidelines below.

## Language standard

Code in the SDK conforms to the following standards. Changes to the C++ standard
happen relatively infrequently. Changes to the Python version are more frequent.

| Language | Version |
| -------- | ------- |
| C++      | C++17   |
| Python   | 3.10    |

Product-specific software may elect to use later standards in their own work.

## Coding Guidelines

### Common

#### When in Rome

The most important convention and practice in the Matter SDK repo is "_When in
Rome..._", per the quote below.

[quote, St. Ambrose]

---

If you should be in Rome, live in the Roman manner; if you should be elsewhere,
live as they do there.

---

Your extensions or fixes to existing code should match the prevailing style of
the original code.

If you find the conventions so foreign or otherwise confusing, it may be best to
let whoever owns the file make the necessary changes or seek the counsel of
others in the group to find out what the right thing to do is. Never just start
changing code wholesale for personal reasons without consulting others first.

#### Commenting Out or Disabling Code

Unused code shall not be disabled by commenting it out with C- or C++-style
comments or with preprocessor `#if 0 ... #endif` semantics. Unused code should
be removed.

#### Auto-formatters

We use the following auto-formatters on code:

| Language    | Formatter          | Style File                         |
| ----------- | ------------------ | -----------------------------------|
| C++         | clang-format       | [.clang-format][clang_format_link] |
| Objective-C | clang-format       | [.clang-format][clang_format_link] |
| java        | google-java-format | N/A                                |
| Python      | pep8, isort, ruff  | [pyproject.toml][pyproject_link]   |
| YAML        | prettier           | None                               |
| JSON        | prettier           | None                               |
| markdown    | prettier           | None                               |

[clang_format_link]:
    https://github.com/project-chip/connectedhomeip/blob/master/.clang-format
[pyproject_link]:
    https://github.com/project-chip/connectedhomeip/blob/master/pyproject.toml

All pull requests run formatting checks using these tools before merge is
allowed. Generated code is not run through restyle.

### C++

#### Use C++ _cstdint_ for Plain Old Data Types

Standard, scalar data types defined in _cstdint_ should be used for basic signed
and unsigned integer types, especially when size and serialization to
non-volatile storage or across a network is concerned.

Examples of these are: `uint8_t`, `int8_t`, etc.

#### Avoid top-level `using namespace` Statements in Headers

By doing this, you are effectively forcing every other module that includes the
header to also be using the namespace. This causes namespace pollution and
generally defeats the purposes of namespaces. Fully-qualified symbols or
namespace blocks should be used instead.

#### Classes / objects not exposed in a header should be in an anonymous namespace

If a cpp class defines a class or instantiates a static object, it should be
enclosed in an anonymous namespace.

```
namespace {
  // CPP internal defines go here
}  // namespace
```

#### Singleton use

The decision to use a singleton class should be considered with care. Do not
default to using a singleton for ease of writing code.

If the class truly should be a singleton (ex. if it is controlling access to a
hardware resource)

-   The standard function name for accessing an SDK singleton is GetInstance().
-   Singleton classes should delete copy and move constructors

#### Avoid Heap-based Resource Allocation and auto-resizing std containers

Heap-based resource allocation should be avoided in the core SDK for common code
that may run on constrained embedded devices. This includes any container
element in std that automatically re-sizes itself at runtime (ex. vector, string
etc.) as these re-size operations are often large and can lead to memory
exhaustion and fragmentation on embedded systems.

Heap-based allocation is allowed for controller code and is at the discretion of
platform vendors for platform-specific code.

##### Alternatives

In either case, recommended resource allocation alternatives are:

-   In-place allocation and initialization
-   Pool-based allocators
-   Platform-defined and -assigned allocators

[CHIPMem.h](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/CHIPMem.h)
provides support for platform defined allocators.

[Pool.h](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/Pool.h)
is the Matter SDK pool allocator implementation.

#### Prefer CopySpanToMutableSpan over memcpy when using spans

See
[Span.h](https://github.com/project-chip/connectedhomeip/blob/master/src/lib/support/Span.h)

#### Prefer std::optional to CHIP implementation in newer code

The Matter SDK Optional.h was implemented when the Matter SDK was C++14, but
newer code can use std::optional, which offers some benefits over the Matter SDK
implementation (ex. std::optional is trivially destructible if the underlying
type is also trivially destructible)

### Python

#### Type hints

Use type hints on function definitions for public APIs.

#### Docstrings

Docstrings should be included for all public APIs.

#### mypy

The current python code does not yet pass mypy checks, but we are working
towards this goal. The more compliant new code is to mypy, the better.
