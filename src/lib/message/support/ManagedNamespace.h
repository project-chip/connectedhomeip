/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file defines mnemonics and constants for [Managed
 *      Namespaces](@ref mns) in the CHIP SDK.
 *
 */

#ifndef _CHIP_MANAGEDNAMESPACE_HPP
#define _CHIP_MANAGEDNAMESPACE_HPP

/**
 *  @page mns Managed Namespaces
 *
 *  @section mns-intro Introduction
 *
 *    Managed namespaces are used in the CHIP SDK to provide both
 *    CHIP SDK developers and integrators alike with advertised guidance
 *    and subtext on the designation of particular API sets within the SDK
 *    such that they can plan and predict their migration path across
 *    CHIP SDK releases and, potentially, manage multiple, concurrent
 *    CHIP APIs for a given module.
 *
 *  @section mns-designation Designation
 *
 *    Managed namespaces may be managed as one of four designations:
 *
 *      * [Development](@ref mns-designation-development)
 *      * [Next](@ref mns-designation-next)
 *      * [Current](@ref mns-designation-current)
 *      * [Legacy](@ref mns-designation-legacy)
 *
 *    @subsection mns-designation-development Development
 *
 *    Any namespace managed with the Development designation is an
 *    indication to developers and integrators that the APIs contained
 *    within are under active development, may be subject to change,
 *    and are not officially supported. Integrators are generally
 *    discouraged from using these APIs unless they are specifically
 *    directed to do so.
 *
 *    @subsection mns-designation-next Next
 *
 *    Any namespace managed with the Next designation is an indication to
 *    developers and integrators that the APIs contained within, while
 *    they have largely completed active development, may still be
 *    subject to change and are supported for early evaluation
 *    purposes. APIs so designated represent the next evoluationary front
 *    in a CHIP SDK API and will become the current, default APIs at
 *    a major releaes cycle in the immediate to near future.
 *
 *    Backward compatibility, both from an API and over-the-wire
 *    protocol perspective, may exist but is not guaranteed in APIs so
 *    designated.
 *
 *    The Next managed namespace designation effectively provides
 *    developers and integrators with a view to where the CHIP SDK is
 *    headed by hinting what will become the current, default API in a
 *    future release.
 *
 *    The Next managed namespace designation is optional such that a managed
 *    namespace may transition through a lifecycle without using it
 *    (see [Managed Namespace Lifecycle](@ref mns-lifecycle)).
 *
 *    @subsection mns-designation-current Current
 *
 *    Any namespace managed with the Current designation or any
 *    unmanaged namespace (i.e. lacking a managed namespace
 *    designation) represents the current, default, official supported
 *    API for that portion or module of the CHIP SDK. While there
 *    still may be ongoing enhancements to such APIs, changes will
 *    largely be incremental and backward compatibility, both API and
 *    over-the-wire, should be maintained.
 *
 *    The Current managed namespace designation is optional such that
 *    a managed namespace may transition through a lifecycle without
 *    using it (see [Managed Namespace Lifecycle](@ref mns-lifecycle)).
 *    In fact, any unmanaged namespace is implicitly Current.
 *
 *    @subsection mns-designation-legacy Legacy
 *
 *    Any namespace managed with the Legacy designation is an
 *    indication to developers and integrators that the APIs contained
 *    within have been deprecated and are supplanted with a new,
 *    current API. These APIs represent what was formerly the current
 *    API.
 *
 *    APIs so designated will disappear altogether at the next major
 *    CHIP SDK release; consequently, developers and integrators should
 *    establish plans for migration away from these APIs if they
 *    intend to stay with the leading edge of CHIP SDK releases.
 *
 *  @section mns-lifecycle Managed Namespace Lifecycle
 *
 *    The following figure illustrates the lifecycle of a managed
 *    namespace as it transitions from Development and, potentially,
 *    to Legacy:
 *
 *   @code
 *      .-------------.      .- - - .      .- - - - -.      .--------.
 *      | Development | -.->   Next   -.->   Current   ---> | Legacy |
 *      '-------------'  |   '- - - '  |   ' - - - - '      '--------'
 *                       |             |
 *                       '-------------'
 *   @endcode
 *
 *    If it is employed, the managed namespace lifecycle begins with the
 *    Development designation.
 *
 *    When development is complete and the code is ready for
 *    evaluation and integration, the designation migrates to either
 *    Next or Current. Alternatively, the designation may be dropped
 *    altogether and managed namespace no longer employed, effectively
 *    making the designation implicitly Current.
 *
 *    If the code is to live alongside and not yet supplant current
 *    code, then the designation should migrate to Next. If the code
 *    is to supplant current code, then the designation should migrate
 *    to Current.
 *
 *    Using the Next designation, after the code has undergone the desired
 *    number of release and evaluation cycles, the designation migrates to
 *    Current or, again, the Designation may be dropped altogether.
 *
 *    Using the Current designation, if the code is to be supplanted by new
 *    code but still needs to be maintained for a number of release
 *    cycles, the designation migrates to Legacy.
 *
 *    From the Legacy designation, the code is eventually removed from
 *    the CHIP SDK altogether.
 *
 *  @section mns-using Using Managed Namespaces
 *
 *  CHIP SDK users may interact with managed namespaces either as
 *  [developer](@ref mns-using-develop), extending and maintaining
 *  existing code, or as an [integrator](@ref mns-using-integrate),
 *  integrating CHIP into their own application, platform, and system
 *  code. The follow two sections detail recommendations for dealing
 *  with CHIP managed namespaces from those two perspectives.
 *
 *  @subsection mns-using-develop Using Managed Namespaces as a Developer
 *
 *  A key focus of the CHIP SDK developer is enhancing and developing
 *  new CHIP SDK APIs and functionality while, in many cases, at the
 *  same time supporting existing API and functionality deployments.
 *
 *  Where it is not possible to satisfy both of these focus areas in
 *  backward-compatible way within the same API, managed namespaces
 *  provide a mechanism to manage these APIs in parallel, in a way
 *  that does not disrupt existing API and functionality deployments.
 *
 *  As a working example, assume a CHIP profile, Mercury, that
 *  currently exists under the following, unmanaged, namespace
 *  hierarchy:
 *
 *  @code
 *    namespace nl {
 *    namespace chip {
 *    namespace Profiles {
 *    namespace Mercury {
 *
 *    // ...
 *
 *    }; // namespace Mercury
 *    }; // namespace Profiles
 *    }; // namespace chip
 *    }; // namespace nl
 *  @endcode
 *
 *  and the following public headers:
 *
 *    * CHIP/Profiles/Mercury/Mercury.hpp
 *    * CHIP/Profiles/Mercury/Bar.hpp
 *    * CHIP/Profiles/Mercury/Foo.hpp
 *    * CHIP/Profiles/Mercury/Foobar.hpp
 *
 *  where Mercury.hpp is the module "umbrella" header. Most
 *  integrators simply include the module "umbrella" header as shown:
 *
 *  @code
 *    #include <Profiles/Mercury/Mercury.hpp>
 *  @endcode
 *
 *  However, developent of Mercury has now reached a point in which
 *  there is a need to develop a next-generation of the APIs and,
 *  potentially, the over-the-wire protocol that are not
 *  backward-compatible to existing deployments. Using managed
 *  namespaces can help accomplish this without breaking these
 *  existing deployments.
 *
 *  @subsubsection mns-using-develop-move-existing-to-current Move Existing Namespace to Current
 *
 *  With a goal of continuing to support the current release of the
 *  API and functionality for existing deployed integrations, the
 *  first task is to move the current code:
 *
 *  @code
 *    % cd src/lib/profiles/mercury
 *    % mkdir Current
 *    % mv Mercury.hpp Bar.hpp Foo.hpp Foobar.hpp *.cpp Current/
 *  @endcode
 *
 *  Note, in addition to moving the files, the header include guards
 *  for the moved files should also be renamed, potentially decorating
 *  them with '_CURRENT', since new, like-named files will be created
 *  their place below.
 *
 *  With the code moved, the next step is to manage the namespace with
 *  the appropriate designation, here 'Current'. First, create a
 *  header that defines the managed namespace, as
 *  'Current/MercuryManagedNamespace.hpp'. Creating such a header is
 *  preferrable to repeating and duplicating this content in each
 *  header file when there are multiple header files.
 *
 *  @code
 *    % cat << EOF > Current/MercuryManagedNamespace.hpp
 *    #ifndef _CHIP_MERCURY_MANAGEDNAMESPACE_CURRENT_HPP
 *    #define _CHIP_MERCURY_MANAGEDNAMESPACE_CURRENT_HPP
 *
 *    #include <support/ManagedNamespace.hpp>

 *    #if defined(CHIP_CONFIG_MERCURY_NAMESPACE) && CHIP_CONFIG_MERCURY_NAMESPACE != kChipManagedNamespace_Current
 *    #error Compiling CHIP Mercury current-designation managed namespace file with CHIP_CONFIG_MERCURY_NAMESPACE defined !=
 kChipManagedNamespace_Current
 *    #endif
 *
 *    #ifndef CHIP_CONFIG_MERCURY_NAMESPACE
 *    #define CHIP_CONFIG_MERCURY_NAMESPACE kChipManagedNamespace_Current
 *    #endif
 *
 *    namespace nl {
 *    namespace chip {
 *    namespace Profiles {
 *
 *    namespace ChipMakeManagedNamespaceIdentifier(Mercury, kChipManagedNamespaceDesignation_Current) { };
 *
 *    namespace Mercury = ChipMakeManagedNamespaceIdentifier(Mercury, kChipManagedNamespaceDesignation_Current);
 *
 *    }; // namespace Profiles
 *    }; // namespace chip
 *    }; // namespace nl
 *
 *    #endif // _CHIP_MERCURY_MANAGEDNAMESPACE_CURRENT_HPP
 *    EOF
 *  @endcode
 *
 *  Next, include this header prior to other module-specific include
 *  directives in the existing headers. For example:
 *
 *   @code
 *
 *    #include <Profiles/Mercury/Current/MercuryManagedNamespace.hpp>
 *
 *    #include <Profiles/Mercury/Bar.hpp>
 *
 *
 *   @endcode
 *
 *  @subsubsection mns-using-develop-create-compatibility-headers Create Compatibility Headers
 *
 *  Moving the existing headers to a new location and managing their
 *  namespace alone, however, is not enough to ensure that existing
 *  deployments will work without change since they are all using
 *  include directives that specified the headers just moved above.
 *
 *  To address this, compatibility wrapper headers with names matching
 *  those just moved must be created.
 *
 *  @code
 *    % touch Mercury.hpp Bar.hpp Foo.hpp Foobar.hpp
 *  @endcode
 *
 *  If only a Current-designated managed namespace is being created
 *  without creating a Development- or Next-designated managed
 *  namespace to accompany it, the contents of these files can simply
 *  consist of a header include guard and an include directive
 *  specifying the newly-moved header of the same name:
 *
 *  @code
 *    #ifndef _CHIP_MERCURY_BAR_HPP
 *    #define _CHIP_MERCURY_BAR_HPP
 *
 *    #include <Profiles/Mercury/Current/Bar.hpp>
 *
 *    #endif // _CHIP_MERCURY_BAR_HPP
 *  @endcode
 *
 *  However, if a Development- or Next-designated managed namespace is
 *  being created as well to accomodate new, incompatible development,
 *  something slightly more complex needs to be done.
 *
 *  As before, a header for the managed namespace configuration is
 *  created, here as MercuryManagedNamespace.hpp. Again, this is
 *  preferrable to repeating and duplicating this content in each
 *  header file when there are multiple header files..
 *
 *  @code
 *    % cat << EOF > MercuryManagedNamespace.hpp
 *    #ifndef _CHIP_MERCURY_MANAGEDNAMESPACE_HPP
 *    #define _CHIP_MERCURY_MANAGEDNAMESPACE_HPP
 *
 *    #include <support/ManagedNamespace.hpp>
 *
 *    #if defined(CHIP_CONFIG_MERCURY_NAMESPACE)                             \
 *      && (CHIP_CONFIG_MERCURY_NAMESPACE != kChipManagedNamespace_Current) \
 *      && (CHIP_CONFIG_MERCURY_NAMESPACE != kChipManagedNamespace_Development)
 *    #error "CHIP_CONFIG_MERCURY_NAMESPACE defined, but not as namespace kChipManagedNamespace_Current or
 kChipManagedNamespace_Development"
 *    #endif
 *
 *    #if !defined(CHIP_CONFIG_MERCURY_NAMESPACE)
 *    #define CHIP_CONFIG_MERCURY_NAMESPACE kChipManagedNamespace_Current
 *    #endif
 *
 *    #endif // _CHIP_MERCURY_MANAGEDNAMESPACE_HPP
 *    EOF
 *  @endcode
 *
 *  Note that this defaults, as desired, the managed namespace
 *  designation to 'Current' if no configuration has been defined.
 *
 *  With this header in place, the compatibility wrapper headers can
 *  now be edited to contain:
 *
 *  @code
 *    #include <Profiles/Mercury/MercuryManagedNamespace.hpp>
 *
 *    #if CHIP_CONFIG_MERCURY_NAMESPACE == kChipManagedNamespace_Development
 *    #include <Profiles/Mercury/Development/Bar.hpp>
 *    #else
 *    #include <Profiles/Mercury/Current/Bar.hpp>
 *    #endif // CHIP_CONFIG_MERCURY_NAMESPACE == kChipManagedNamespace_Development
 *  @endcode
 *
 *  or whatever is appropriate for the namespace management use case
 *  at hand.
 *
 *  @subsubsection mns-using-develop-create-development-content Create Development Content
 *
 *  At this point, the infrastructure is now in place to start
 *  building out new functionality and APIs alongside the existing
 *  ones.
 *
 *  @code
 *    % mkdir Development
 *    % touch Development/Mercury.hpp Development/Bar.hpp Development/Foo.hpp Development/Foobar.hpp
 *    % cat << EOF > Development/MercuryManagedNamespace.hpp
 *    #ifndef _CHIP_MERCURY_MANAGEDNAMESPACE_DEVELOPMENT_HPP
 *    #define _CHIP_MERCURY_MANAGEDNAMESPACE_DEVELOPMENT_HPP
 *
 *    #include <support/ManagedNamespace.hpp>

 *    #if defined(CHIP_CONFIG_MERCURY_NAMESPACE) && CHIP_CONFIG_MERCURY_NAMESPACE != kChipManagedNamespace_Development
 *    #error Compiling CHIP Mercury development-designated managed namespace file with CHIP_CONFIG_MERCURY_NAMESPACE defined !=
 kChipManagedNamespace_Development
 *    #endif
 *
 *    #ifndef CHIP_CONFIG_MERCURY_NAMESPACE
 *    #define CHIP_CONFIG_MERCURY_NAMESPACE kChipManagedNamespace_Development
 *    #endif
 *
 *    namespace nl {
 *    namespace chip {
 *    namespace Profiles {
 *
 *    namespace ChipMakeManagedNamespaceIdentifier(Mercury, kChipManagedNamespaceDesignation_Development) { };
 *
 *    namespace Mercury = ChipMakeManagedNamespaceIdentifier(Mercury, kChipManagedNamespaceDesignation_Development);
 *
 *    }; // namespace Profiles
 *    }; // namespace chip
 *    }; // namespace nl
 *
 *    #endif // _CHIP_MERCURY_MANAGEDNAMESPACE_DEVELOPMENT_HPP
 *    EOF
 *  @endcode
 *
 *  Of course, if a module is far simpler than the example presented
 *  here and does not have many classes, source, files, or headers,
 *  this could all be accomplished in the same header file without
 *  moving files around and creating multiple standalone configuration
 *  and compatibility headers. However, with this complex example, it
 *  should inspire managed namespace solutions along a spectrum from
 *  complex to simple.
 *
 *  @subsection mns-using-integrate Using Managed Namespaces as an Integrator
 *
 *  A key focus of the CHIP SDK integrator is including the
 *  appropriate CHIP SDK public API headers and integrating and
 *  developing applications against them.
 *
 *  As a working example, again assume a CHIP profile, Mercury, that
 *  has Next-, Current-, and Legacy-designated managed namespaces,
 *  whose public headers are structured as follows:
 *
 *    * CHIP/Profiles/Mercury/Mercury.hpp
 *    * CHIP/Profiles/Mercury/Bar.hpp
 *    * CHIP/Profiles/Mercury/Foo.hpp
 *    * CHIP/Profiles/Mercury/Foobar.hpp
 *    * CHIP/Profiles/Mercury/Next/Mercury.hpp
 *    * CHIP/Profiles/Mercury/Next/Bar.hpp
 *    * CHIP/Profiles/Mercury/Next/Foo.hpp
 *    * CHIP/Profiles/Mercury/Next/Foobar.hpp
 *    * CHIP/Profiles/Mercury/Current/Mercury.hpp
 *    * CHIP/Profiles/Mercury/Current/Bar.hpp
 *    * CHIP/Profiles/Mercury/Current/Foo.hpp
 *    * CHIP/Profiles/Mercury/Current/Foobar.hpp
 *    * CHIP/Profiles/Mercury/Legacy/Mercury.hpp
 *    * CHIP/Profiles/Mercury/Legacy/Bar.hpp
 *    * CHIP/Profiles/Mercury/Legacy/Foo.hpp
 *    * CHIP/Profiles/Mercury/Legacy/Foobar.hpp
 *
 *  where Mercury.hpp is the module "umbrella" header.
 *
 *  Unless the use case at hand motivates including a namespace
 *  managed module within CHIP explicitly, for example:
 *
 *  @code
 *    #include <Profiles/Mercury/Legacy/Mercury.hpp>
 *  @endcode
 *
 *  it is best to reference CHIP module public headers by their
 *  unmanaged, default paths
 *  (e.g. CHIP/Profiles/Mercury/Mercury.hpp). Doing so allows
 *  following a progression of API development without continually
 *  changing a project's include directives as those APIs flow through
 *  the managed [lifecycle](@ref mns-lifecycle).
 *
 *  Following this strategy, deployments can then retarget their code
 *  at a different managed namespace designation, the Current
 *  designation for example, by specifying the desired configuration
 *  in the C/C++ preprocessor. This may be done on the command line,
 *  in the source code, or in a configuration or prefix header:
 *
 *  @code
 *    #define CHIP_CONFIG_MERCURY_NAMESPACE kChipManagedNamespace_Current
 *  @endcode
 *
 *  and use the unmanaged / unqualified include path:
 *
 *  @code
 *    #include <Profiles/Mercury/Mercury.hpp>
 *  @endcode
 *
 *  When, and if, the managed namespace designation changes for the
 *  targeted APIs, for example from Current to Legacy, simply retarget
 *  by adjusting the preprocessor definition:
 *
 *  @code
 *    #define CHIP_CONFIG_MERCURY_NAMESPACE kChipManagedNamespace_Legacy
 *  @endcode
 *
 */

/**
 *  @def kChipManagedNamespaceDesignation_Legacy
 *
 *  @brief
 *    This managed namespace designation should be used for
 *    designating namespaces containing formerly-current, default,
 *    production APIs as supported but marked for short-term
 *    deprecation.
 *
 *  @sa #ChipMakeManagedNamespaceIdentifier
 *
 */
#define kChipManagedNamespaceDesignation_Legacy Legacy

/**
 *  @def kChipManagedNamespaceDesignation_Current
 *
 *  @brief
 *    This managed namespace designation should be used for
 *    designating namespaces containing current, default,
 *    production APIs.
 *
 *  @sa #ChipMakeManagedNamespaceIdentifier
 *
 */
#define kChipManagedNamespaceDesignation_Current Current

/**
 *  @def kChipManagedNamespaceDesignation_Next
 *
 *  @brief
 *    This managed namespace designation should be used for
 *    designating namespaces containing future production APIs.
 *
 *  @sa #ChipMakeManagedNamespaceIdentifier
 *
 */
#define kChipManagedNamespaceDesignation_Next Next

/**
 *  @def kChipManagedNamespaceDesignation_Development
 *
 *  @brief
 *    This managed namespace designation should be used for
 *    designating namespaces containing unstable APIs under active
 *    development.
 *
 *  @sa #ChipMakeManagedNamespaceIdentifier
 *
 */
#define kChipManagedNamespaceDesignation_Development Development

/// @cond
#define _ChipMakeManagedNamespaceIdentifier(aIdentifier, aDesignation) aIdentifier##_##aDesignation
/// @endcond

/**
 *  @def ChipMakeManagedNamespaceIdentifier(aIdentifier, aDesignation)
 *
 *  @brief
 *    This creates a managed namespace identifier consisting of the
 *    concatenation of @a aIdentifier with @a aDesignation as
 *    "aIdentifier_aDesignation".
 *
 *  @param[in]  aIdentifier   The based, primary, or default identifier
 *                            for the namespace to decorate with the
 *                            specified designation.
 *  @param[in]  aDesignation  The managed designation with which to qualify
 *                            the specified namespace identifier.
 *
 *  @sa #kChipManagedNamespaceDesignation_Development
 *  @sa #kChipManagedNamespaceDesignation_Next
 *  @sa #kChipManagedNamespaceDesignation_Current
 *  @sa #kChipManagedNamespaceDesignation_Legacy
 *
 */
#define ChipMakeManagedNamespaceIdentifier(aIdentifier, aDesignation) _ChipMakeManagedNamespaceIdentifier(aIdentifier, aDesignation)

/**
 *  @def kChipManagedNamespace_Legacy
 *
 *  @brief
 *    Where the selection of multiple managed namespaces is available
 *    via configuration, this should be specified when the [Legacy
 *    designation](@ref #kChipManagedNamespaceDesignation_Legacy) is
 *    available and desired.
 *
 *  @sa #kChipManagedNamespace_Current
 *  @sa #kChipManagedNamespace_Next
 *  @sa #kChipManagedNamespace_Development
 *
 */
#define kChipManagedNamespace_Legacy (0xFFFFFFFF)

/**
 *  @def kChipManagedNamespace_Current
 *
 *  @brief
 *    Where the selection of multiple managed namespaces is available
 *    via configuration, this should be specified when the [Current
 *    designation](@ref #kChipManagedNamespaceDesignation_Current) is
 *    available and desired.
 *
 *  @sa #kChipManagedNamespace_Legacy
 *  @sa #kChipManagedNamespace_Next
 *  @sa #kChipManagedNamespace_Development
 */
#define kChipManagedNamespace_Current (0x00000000)

/**
 *  @def kChipManagedNamespace_Next
 *
 *  @brief
 *    Where the selection of multiple managed namespaces is available
 *    via configuration, this should be specified when the [Next
 *    designation](@ref #kChipManagedNamespaceDesignation_Next) is
 *    available and desired.
 *
 *  @sa #kChipManagedNamespace_Legacy
 *  @sa #kChipManagedNamespace_Current
 *  @sa #kChipManagedNamespace_Development
 */
#define kChipManagedNamespace_Next (0x00000001)

/**
 *  @def kChipManagedNamespace_Development
 *
 *  @brief
 *    Where the selection of multiple managed namespaces is available
 *    via configuration, this should be specified when the [Development
 *    designation](@ref #kChipManagedNamespaceDesignation_Development) is
 *    available and desired.
 *
 *  @sa #kChipManagedNamespace_Legacy
 *  @sa #kChipManagedNamespace_Current
 *  @sa #kChipManagedNamespace_Next
 */
#define kChipManagedNamespace_Development (0x7FFFFFFF)

#endif // _CHIP_MANAGEDNAMESPACE_HPP
