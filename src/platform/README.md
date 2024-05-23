# CHIP Device Layer Design Notes

This document contains overviews, notes and other informative material related
to the internal design of the CHIP Device Layer (`src/platform`). It is intended
as a place to host documentation on topics that are of value to implementers,
but for reasons of size or scope don't naturally fit within comments in the
code.

This is intended to be a living document, with an informal structure, that
evolves along with the code. Developers are encouraged to add things that they
think would be useful to their fellow engineers.

This document contains the following sections:

-   [Device Layer Adaptation Patterns](#Device-Layer-Adaptation-Patterns)

---

### Device Layer Adaptation Patterns

The Device Layer uses various design patterns to make it easier to adapt the
code to different platforms and operating contexts.

The CHIP Device Layer is intended to work across a variety of platforms and
operating contexts. These environments may differ by system type, OS, network
stack and/or threading model. One of the goals of the Device Layer is to make it
easy to adapt the CHIP application stack to new environments. This is especially
desirable in cases where the new platform is substantially similar to an
existing adaptation.

As part of its design, the CHIP Device Layer enables a pattern of code reuse
that strives to reduce the need for preprocessor conditionals (e.g. #ifdef).
While not eliminating #ifdefs entirely, the design allows major variances in
behavior to be expressed as distinct code bases (typically separate C++ classes)
which are then brought together via composition to enable a particular
adaptation.

To promote application portability, the CHIP Device Layer employs a pattern of
static polymorphism to insulate its application-visible API from the underlying
platform-specific implementation. A similar interface pattern is used within the
Device Layer itself to provide compartmentalization between components.

As much as possible, the above goals are achieved via the use of zero-cost
abstraction patterns (zero-cost in terms of code size and execution overhead).
Effort has been made to make the patterns easy to work with, without a lot of
conceptual burden or fiddly syntax.

This following sections describe some of the patterns used to achieve these
goals.

1. [Interface and Implementation Classes](#Interface-and-Implementation-Classes)<br>
2. [Method Forwarding](#Method-Forwarding)<br>
3. [Target Platform Selection](#Target-Platform-Selection)<br>
4. [Generic Implementation Classes](#Generic-Implementation-Classes)<br>
5. [Overriding Generic Behaviors](#Overriding-Generic-Behaviors)<br>
6. [Multiple Inheritance and Subclassing of Generic Implementations](#Multiple-Inheritance-and-Subclassing-of-Generic-Implementations)<br>
7. [Static Virtualization of Generic Implementation Behavior](#Static-Virtualization-of-Generic-Implementation-Behavior)<br>
8. [.cpp Files and Explicit Template Instantiation](#-ipp-files-and-explicit-template-instantiation)<br>

---

### Interface and Implementation Classes

The CHIP Device Layer uses a dual-class pattern to separate the abstract
features of a component object (generally its externally visible methods) from
the concrete implementation of those features on a particular platform.
Following this pattern, each major component in the Device Layer is embodied in
(at least) two C++ classes: an abstract interface class and an implementation
class.

The outwardly visible _**abstract interface class**_ defines a set of common
methods (and potentially other members) that are universally available to the
component user, but independent of the underlying implementation. Interface
classes contains no functionality themselves, but instead forward all method
calls to an associated implementation class using a zero-cost abstraction
technique. Interface classes serve to formalize the functional interface of a
component, as well as provide a place on which to host implementation-neutral
API documentation.

An _**implementation class**_ provides a concrete, platform-specific
implementation of the logical functionality exposed by an interface class. This
functionality may be provided directly by the class itself (i.e. within its
methods), or via delegation to one or more helper classes.

Pairs of abstract interface classes and implementation classes exists for each
major application-visible component of the Device Layer. Additionally, similar
pairs of classes are defined within the Device Layer to aid in isolation between
components.

Abstract interface classes are named after the functionality they provide–e.g.
ConfigurationManager, ConnectivityManager, etc. Implementation classes take the
name of their interface class with the suffix `Impl` attached. In all cases,
implementation class are required to inherit publicly from their interface
classes.

```cpp
class ConfigurationManagerImpl;

/** Interface class for ConfigurationManager component
 */
class ConfigurationManager
{
    using ImplClass = ConfigurationManagerImpl;

public:
    CHIP_ERROR GetDeviceId(uint64_t & deviceId);
    static CHIP_ERROR Init();
    ...
};

/** Concrete implementation of ConfigurationManager component for a specific platform
 */
class ConfigurationManagerImpl final
    : public ConfigurationManager
{
    ...
};
```

### Method Forwarding

Interface classes forward method calls to their implementation classes by means
of short inline functions called **_forwarding methods_**. These methods forward
calls from the application by downcasting the object’s `this` pointer and
invoking similarly named methods on the implementation class. This pattern is
similar to the C++
[Curiously Recurring Template Pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern)
with the exception that the relationship between the base class and the subclass
is fixed, rather than being expressed as a template parameter. A type alias
named `ImplClass` is used within the interface to make the forwarding method
definitions more concise.

```cpp
inline CHIP_ERROR ConfigurationManager::GetDeviceId(uint64_t & deviceId)
{
    /* forward method call... */
    return static_cast<ImplClass*>(this)->_GetDeviceId(deviceId);
}
```

One convenient feature of this pattern is it allows for the forwarding of static
methods as well as instance methods. E.g.:

```cpp
inline CHIP_ERROR ConfigurationManager::Init()
{
    return ImplClass::_Init();
}
```

The methods on the implementation class that are the target of forwarding
methods are known as **_implementation methods_**. For every forwarding method
there must be a corresponding implementation method.

A leading underscore (\_) is used to distinguish implementation methods from
their forwarding methods. This arrangement helps to emphasize the distinction
between the two, and ensures the generation of a compilation error should an
implementer neglect to provide an implementation method.

Implementation methods are not meant to be called directly. To block this type
of use, implementation classes declare their implementation methods as private,
and then use a friend declaration to give the interface class the (sole) right
to invoke these methods as part of forwarding.

```cpp
class ConfigurationManagerImpl;

/** Interface class for ConfigurationManager component
 */
class ConfigurationManager
{
    using ImplClass = ConfigurationManagerImpl;

public:
    CHIP_ERROR GetDeviceId(uint64_t & deviceId);
    static CHIP_ERROR Init();
    ...
};

/** Concrete implementation of ConfigurationManager component for specific platform
 */
class ConfigurationManagerImpl final : public ConfigurationManager
{
    /* Let the forwarding methods on ConfigurationManager call implementation
       methods on this class. */
    friend ConfigurationManager;

private:
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId);
    static CHIP_ERROR _Init();
    ...
};

inline CHIP_ERROR ConfigurationManager::GetDeviceId(uint64_t & deviceId)
{
    /* Forward calls to corresponding implementation method... */
    return static_cast<ImplClass*>(this)->_GetDeviceId(deviceId);
}

inline CHIP_ERROR ConfigurationManager::Init()
{
    /* Forward calls to corresponding static implementation method... */
    return ImplClass::_Init();
}
```

### Target Platform Selection

An implementation class provides a concrete implementation of a Device Layer
component for use on a particular platform. Multiple implementation classes may
exist within the Device Layer source tree for the same component. Each of these
classes has the same name, but their code is unique to the associated platform.
The choice of which implementation class is included at compile time is done by
means of a computed #include directive with the following form:

```cpp
/* contents of ConfigurationManager.h */

...

#define CONFIGURATIONMANAGERIMPL_HEADER \
        <platform/CHIP_DEVICE_LAYER_TARGET/ConfigurationManagerImpl.h>
#include CONFIGURATIONMANAGERIMPL_HEADER

...
```

This directive appears within the header file that defines the component’s
interface class. The C++ pre-processor automatically expands the #include line
to select the appropriate implementation header based on the chosen platform. In
this way, source files which include the component’s interface header file
naturally get the correct implementation header as well.

Implementation header files for each supported platform are arranged within
subdirectories that are named after their target platform (e.g. `ESP32`). All
such files have the same file name (e.g. `ConfigurationManagerImpl.h`), and each
file contains a definition for like-named class (`ConfigurationManagerImpl`).

Source files specific to a platform are placed in subdirectories immediately
beneath the root Device Layer source directory (e.g.
`src/adaptations/device-layer/ESP32`). Like platform-specific header
directories, these are subdirectories are named after the target platform.

The choice of target platform for the Device Layer is specified at project
configuration time, using the configure script option
`--device-layer=<target-platform>`. Passing the --device-layer option results in
the definition of a pair of pre-processor symbols where the name of the target
platform is incorporated into the definitions. E.g.:

```cpp
#define CHIP_DEVICE_LAYER_TARGET ESP32
#define CHIP_DEVICE_LAYER_TARGET_ESP32 1
```

The --device-layer configuration option also selects the appropriate set of
platform-specific source files to be included in the generated library file.
This is accomplished via logic in the Device Layer’s Makefile.am.

### Generic Implementation Classes

Often it is possible to share implementation code across a range of platforms.
In some cases the code in question is substantially the same for all targets,
with only minor customizations required in each case. In other cases, the
commonality of the implementation extends across a subset of platforms that
share a particular architectural feature, such as a common OS (Linux, FreeRTOS)
or network stack (sockets, LwIP).

To accommodate this, the CHIP Device Layer encourages a pattern of factoring
common functionality into **_generic implementation base classes_**. These base
classes are then used to compose (via inheritance) the concrete implementation
classes that underlie a component.

Generic implementation base classes are realized as C++ class templates which
follow the C++
[Curiously Recurring Template Pattern](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
Implementation classes wishing to incorporate common behavior inherit from an
instantiation of the template, passing the implementation class itself as the
template’s parameter.

```cpp
/** Generic base class for use in implementing ConfigurationManager components
 */
template<class ImplClass>
class GenericConfigurationManagerImpl
{
    ...
};

/** Concrete implementation of ConfigurationManager component for specific platform
 */
class ConfigurationManagerImpl final
    : public ConfigurationManager,
      public GenericConfigurationManagerImpl<ConfigurationManagerImpl> /* <-- Implementation provided by
                                                                              generic base class. */
{
    ...
};
```

In many cases, the generic implementation base class itself will directly
provide some or all of the implementation methods needed to satisfy the
component’s interface. The rules of C++ method resolution are such that calls to
a forwarding method on the interface class are mapped directly to the base class
method. In this situation, the derived implementation class needn't declare a
version of the target method at all, and method calls are forwarded statically,
at compile time, without overhead.

```cpp
/** Interface class for ConfigurationManager component
 */
class ConfigurationManager
{
    using ImplClass = ConfigurationManagerImpl;

public:
    CHIP_ERROR GetDeviceId(uint64_t & deviceId);
    static CHIP_ERROR Init();
    ...
};

/** Generic base class for use in implementing ConfigurationManager components
 */
template<class ImplClass>
class GenericConfigurationManagerImpl
{
protected:
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId); /* <-- Invoked when GetDeviceId() called. */
    ...
};

/** Concrete implementation of ConfigurationManager component for specific platform
 */
class ConfigurationManagerImpl final
    : public ConfigurationManager,
      public GenericConfigurationManagerImpl<ConfigurationManagerImpl>
{
    ...
};
```

### Overriding Generic Behaviors

Where desirable, concrete implementation classes are free to override the
implementation methods provided by a generic base class. This is accomplished by
defining a platform-specific version of the method on the implementation class.
The rules of C++ result in the method on the implementation class being called
in preference to the generic method.

The new method may entirely replace the behavior of the generic method, or it
may augment its behavior by calling the generic method in the midst of its own
implementation.

```cpp
CHIP_ERROR ConfigurationManagerImpl::_GetDeviceId(uint64_t & deviceId)
{
    using GenericImpl = GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

    /* Call the generic implementation to get the device id. */
    uint64_t deviceId = GenericImpl::_GetDeviceId(deviceId);

    /* Special case the situation where the device id is not known. */
    if (deviceId == kNodeIdNotSpecified) {
        deviceId = PLATFORM_DEFAULT_DEVICE_ID;
    }

    return deviceId;
}
```

### Multiple Inheritance and Subclassing of Generic Implementations

Concrete implementation classes are free to inherit from multiple generic base
classes. This pattern is particularly useful when the overall functionality of a
component can be naturally split into independent slices (e.g. methods that
support WiFi and methods that support Thread). Each such slice can then be
implemented by a distinct base class which ends up being composed together with
other base classes in the final implementation.

```cpp
/** Concrete implementation of ConfigurationManager component for specific platform
 */
class ConfigurationManagerImpl final
    : public ConfigurationManager,
      public GenericWiFiConfigurationManagerImpl<ConfigurationManagerImpl>, /* <-- WiFi features */
      public GenericThreadConfigurationManagerImpl<ConfigurationManagerImpl> /* <-- Thread features */
{
    ...
};
```

A generic implementation base class may also inherit from other generic base
classes. This is useful for “specializing” a generic implementation for a
certain sub-range of use cases (e.g. for a particular OS type).

```cpp
/** Generic base class for use in implementing PlatformManager features
 *  on all platforms.
 */
template<class ImplClass>
class GenericPlatformManagerImpl
{
    ...
};

/** Generic base class for use in implementing PlatformManager features
 *  on FreeRTOS platforms.
 */
template<class ImplClass>
class GenericPlatformManagerImpl_FreeRTOS
    : public GenericPlatformManagerImpl<ImplClass>
{
    ...
};
```

### Static Virtualization of Generic Implementation Behavior

When creating generic implementation base classes, developers are encouraged to
use a pattern of static virtualization to delegate operations to the concrete
implementation class in cases where the operation may or must be implemented in
a platform-specific way.

For example, consider a generic implementation of the ConfigurationManager
component where value accessor methods such as `GetDeviceId()` operate by
retrieving values from an underlying key-value store. The particulars of how the
key-value store is implemented will likely vary by platform. To allow for this,
the generic implementation class is structured to delegate the operation of
retrieving the value for a key to a method on the concrete implementation class.

Following the Curiously Recurring Template Pattern, delegation is accomplished
by casting the `this` pointer to the implementation class and calling a method
with the appropriate signature. An inline helper function named `Impl()` helps
to make the code concise.

```cpp
template<class ImplClass>
class GenericConfigurationManagerImpl
{
protected:
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId);
    ...
private:
    ImplClass * Impl() { return static_cast<ImplClass*>(this); }
};

class ConfigurationManagerImpl final
    : public ConfigurationManager,
      public GenericConfigurationManagerImpl<ConfigurationManagerImpl>
{
    friend GenericConfigurationManagerImpl<ConfigurationManagerImpl>;
private:
    CHIP_ERROR ReadConfigValue(const char * key, uint64_t & value);
};

template<class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceId(uint64_t & deviceId)
{
    /* delegate to the implementation class to read the 'device-id' config value */
    return Impl()->ReadConfigValue(“device-id”, deviceId);
}

CHIP_ERROR ConfigurationManagerImpl::ReadConfigValue(const char * key, uint64_t & value)
{
    /* read value from platform-specific key-value store */
    ...
}
```

In the above example, the delegated method is conceptually ‘pure virtual’ in
that the concrete implementation class must supply a version of the method,
otherwise compilation will fail. In other situations, a similar pattern can be
used to allow an implementation to override a default behavior supplied by the
base class on an as-needed basis.

Again, delegation happens by casting the `this` pointer and calling an
appropriate method. In this case, however, the generic base class provides a
default implementation of the target method which will be used unless the
subclass overrides it.

```cpp
template<class ImplClass>
class GenericPlatformManagerImpl
{
protected:
    void _DispatchEvent(const CHIPDeviceEvent * event);
    void DispatchEventToApplication(const CHIPDeviceEvent * event);
    ...
private:
    ImplClass * Impl() { return static_cast<ImplClass*>(this); }
};

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::_DispatchEvent(const CHIPDeviceEvent * event)
{
    ...
    /* Delegate work to method that can be overridden by implementation class */
    Impl()->DispatchEventToApplication(event);
    ...
}

template<class ImplClass>
void GenericPlatformManagerImpl<ImplClass>::DispatchEventToApplication(const CHIPDeviceEvent * event)
{
    /* provide default implementation of DispatchEventToApplication() */
    ...
}
```

### .cpp Files and Explicit Template Instantiation

The rules for C++ templates require that the compiler 'see' the full definition
of a class template at the moment of its instantiation. (Instantiation in this
context means the moment at which the compiler is forced to generate an actual
class from the recipe provided by the template). Typically this requires placing
the entire definition of the class template, including all its methods, into a
header file, which must then be included before the moment of instantiation.

To provide a separation between the definition of a class template and the
definitions of its members, the CHIP Device Layer places all non-inlined
template member definitions into a separate file. This file has the same base
name as the template header file, but with a `.cpp` suffix. This pattern reduces
clutter in the header file and makes it possible to include the non-inlined
member definitions only when they are needed (more on this below).

```cpp
/* contents of GenericConfigurationManagerImpl.h */

template<class ImplClass>
class GenericConfigurationManagerImpl
{
protected:
    CHIP_ERROR _GetDeviceId(uint64_t & deviceId);
    ...
};
```

```cpp
/* contents of GenericConfigurationManagerImpl.cpp */

template<class ImplClass>
CHIP_ERROR GenericConfigurationManagerImpl<ImplClass>::_GetDeviceId(uint64_t & deviceId)
{
    ...
}
```

It is often the case that the C++ compiler is forced to instantiate a class
template multiple times, once for each .cpp file it compiles. This can add
significant overhead to the compilation process. To avoid this, the Device Layer
uses the C++11 technique of
[explicit template instantiation](https://en.cppreference.com/w/cpp/language/class_template#Explicit_instantiation)
to direct the compiler to instantiate the template only once. This is
accomplished in two steps: first, all header files that makes use of a class
template includes an `extern template class` declaration before the point at
which the template class is used. This tells the compiler to _not_ instantiate
the template in that context.

```cpp
/* contents of ConfigurationManagerImpl.h */

#include <CHIP/DeviceLayer/internal/GenericConfigurationManagerImpl.h>

// Instruct the compiler to instantiate the GenericConfigurationManagerImpl<ConfigurationManagerImpl>
// class only when explicitly asked to do so.
extern template class GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

...
```

Then, within a corresponding .cpp file, the template's .cpp file is included and
a `template class` definition is used to force an explicit instantiation of the
template.

```cpp
/* contents of ConfigurationManagerImpl.cpp */

#include <CHIP/DeviceLayer/internal/GenericConfigurationManagerImpl.cpp>

// Fully instantiate the GenericConfigurationManagerImpl<ConfigurationManagerImpl> class.
template class GenericConfigurationManagerImpl<ConfigurationManagerImpl>;

...
```

The result is that the template's non-inlined members are only parsed and
instantiated once, during compilation of the referencing .cpp file, avoiding
redundant processing in other contexts.
