# Thread Races in Matter

## Current Situation:

-   Data races cause lots of issues in the SDK:

    -   [_\#7297 - Crashes in chip-tool client after PR 7060_](https://github.com/project-chip/connectedhomeip/issues/7297)

    -   [_\#7493 - Handling of mTestIndex in chip-tool tests is racy_](https://github.com/project-chip/connectedhomeip/issues/7493)

    -   [_\#7498 - Crash due to exchange timing out a packet before it ever gets a chance to be sent_](https://github.com/project-chip/connectedhomeip/issues/7498)

    -   [_\#7623 - Linux test suite errors out due to failure to rename tmp file_](https://github.com/project-chip/connectedhomeip/issues/7623)

    -   [_\#7574 - chip-tool segfault on Darwin_](https://github.com/project-chip/connectedhomeip/issues/7574)

-   Recently added support for tsan that provides an empirical, measurable way
    to detect and validate the ‘race-free’-ness of our stack.

-   Need to really come together as a development team and figure out how to
    address these systemically, rather than applying incremental patches.

-   [_\#7478_](https://github.com/project-chip/connectedhomeip/pull/7478) using
    tsan as a validator to see how **one** particular systemic approach to
    fixing these data races would look like.

-   In chip-tool,

    -   Lots of concurrent access to global state from multiple threads of
        execution across all phases of the lifetime of an application
        (initialization, steady state operation and shutdown).

-   \#7478 took **one** particular approach to solving that specifically with
    chip-tool, which is to:

    -   Provide a number of options within
        [_PlatformManager’s API_](https://github.com/project-chip/connectedhomeip/blob/master/src/include/platform/PlatformManager.h#L79)
        set that empowers applications to compose in the right patterns that
        fits their needs:

        -   A: Run the stack on a separate threading context from the
            ‘application’, and use the stack lock to serialize access to global
            state in the SDK

            -   chip-tool, im-initiator, minimal mdns client, im-responder
                mostly take this approach.

        -   B: Run the stack on the same threading context as the application,
            and avoid the need to use the stack lock to serialize access to
            global state in the SDK

            -   All the server-side example applications mostly take this
                approach

        -   C: Run the stack on a separate threading context from the
            ‘application’, but make it the application’s responsibility to
            handle messaging between the different threading queues in a
            platform-specific manner.

            -   Any darwin example apps perhaps?

    -   Ensure consistent impleme km ntations of the PlatformManager API on all
        platforms (targeted POSIX to start with to establish a baseline)

    -   Selected B) as the basis for the fixes to chip-tool given it was best
        aligned with how it was structured today.

    -   Doesn’t mean that that’s how we **have** to do it.

## Q1: Should the SDK be opinionated and take a particular stance on how threading and synchronization should be achieved by applications? Or should it be left to applications to decide?

-   When it comes to the SDK, achieving true concurrency is less important than
    ensuring serialized, safe access to the global state in the SDK.
-   What are the guidelines for how applications should interact with the SDK
    from a synchronization perspective?

## Q2: Should PlatformManager APIs be consistently implemented on all

platforms? Which subsets of it should be?

-   E.g POSIX’s implementation of PlatformManager API varies quite a bit from
    the Darwin implementation, which varies quite a bit from the FreeRTOS
    version.

    -   See
        [_this_](https://github.com/project-chip/connectedhomeip/issues/7557#issuecomment-860271069)
        issue for analysis on Darwin’s version.

## Q3: How do we support common tools like chip-tool (client), all-clusters-app (server) that work correctly/reliably across different platforms?\*\*

-   Option A and B have the most portability, leverage APIs in PlatformManager
    that can be implemented on most platforms.

-   Option C has the least portability but maximum concurrency, requires highly
    custom APIs for each platform:

    -   > Dispatching blocks on dispatch queues on Darwin

    -   > Dispatching generic closures on an std::queue on POSIX

    -   > Dispatching highly constrained closures onto a FreeRTOS queue on
        > FreeRTOS

-   Should avoid needing to sprinkle \#ifdefs in application code based on
    platform-impl specific API

## Meeting Minutes (June 15th)

Attendees: Jerry, Michael Spang, Michael Sandstedt, Tennessee, Andrei, Vivien,
Yufeng

-   Consensus that applications should be responsible for managing the
    threading, dispatch and queueing of SDK events and API calls, deciding the
    models that are most appropriate to their needs.

-   chip-tool shouldn’t really have been designed from the onset to have a
    separate ‘main’ thread, which doesn’t do much beyond just blocking waiting
    for a result. Instead, it could have been folded into the same threading
    context as that running the SDK core. This could have minimized the issues
    we have seen so far.

-   General consensus that the PlatformManager part of the Device Layer
    shouldn’t really be part of the SDK, and is ‘external’ to the SDK

-   Need to design out a new, more minimal ‘threading/dispatch’ interface that
    the SDK expects to be provided:

    -   A call to post an event

    -   A call to add an event handler for a particular type of event

-   This interface would replace the existing PlatformManager APIs and Impls

-   Q: What about the various \#ifdefs in Inet endpoint logic that selects
    between LWIP and Socket file handles?

    -   This effort does not affect that logic - that still needs a separate
        conversation.

-   With this new operating model, it means there is now not a trivial way to
    write portable, client logic that is reusable across different platforms.
    Notably, this includes tools like chip-tool, clusters-app, etc… Needs more
    discussion.

## Meeting Minutes (June 16th)

Attendees: Jerry, Michael Spang, Michael Sandstedt, Andrei, Vivien, Yufeng,
Boris, Kevin

-   Spent some time clarifying the original rationale for the device layer and
    its design.

-   Acknowledgement however that the device layer today imbues too much policy

-   If application is responsible for managing the threading model, then the
    question of building portable tools like chip-tool surfaces (and a means to
    avoid \#ifdef hell)

-   Suggestion to actually use chip-tool as a means to exemplify different
    threading pattern variants, using it as an example app of sorts AND a
    proving ground for this model

-   Still can keep the various implementations of threading/synchronization
    around in the device layer as possible options, but it is ‘external’ to the
    core SDK. THese implementations would provide an implementation for the
    narrower, ‘Threading and Dispatch’ interface that is mentioned above.

-   Application can use some of these reference examples in their applications
    if it provides them value

-   Side discussion on whether CRTP is still the right pattern to go forward
    with.

    -   Acknowledgement that pure virtual interfaces are used fairly broadly in
        the code, and make it more testable

    -   However, not sure if the testability wins there justify the performance
        losses

-   Kevin to spearhead these changes, Yufeng to assist.

-   See
    [_https://github.com/project-chip/connectedhomeip/issues/7725_](https://github.com/project-chip/connectedhomeip/issues/7725)
    for more details.
