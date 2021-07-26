# Threading & Synchronization

Meeting notes 2021-06-14

-   Two overall schools of thought around how SDK should approach
    synchronization:

    1.  **Make SDK Public/External APIs thread-safe.**

        Two variations in approaches here:

        1.  Async: Make the APIs Post to the CHIP thread and complete the work
            on that thread. This is the approach outlined in
            [_Yufeng’s PR_](https://github.com/project-chip/connectedhomeip/pull/7117).

            -   The approach currently resident in the OpenThread stack.
            -   Requires SDK to be designed in a very async-friendly manner, and
                requires applications to maintain state machines to handle
                bottom half-handlers.

        2.  Sync: Make the APIs grab narrowly-scoped locks that protect very
            specific critical sections.

            -   e.g. One lock to protect ExchangeMgr’s singleton state, another
                to protect the InteractionModelEngine singleton’s state, etc.

    2.  **Not Thread-safe APIs, rely on external synchronization**

        Unless marked otherwise, APIs will be deemed ‘not thread-safe’, and
        require applications embedding the SDK to guarantee thread-safety when
        calling into the SDK

        Couple of options available to applications:

        1.  Run their application logic that calls into the SDK APIs on the same
            threading context as the SDK itself. This requires no further
            synchronization

        2.  Run their application logic on a separate thread, and if so, grab
            the CHIP lock before calling any of the SDK APIs

        3.  Run their application logic on a separate thread, and provide custom
            ‘Posted’ versions of the SDK APIs as they see fit to their consumers
            to safely call into the SDK API on the CHIP thread.

            1.  They’ll also be responsible for handling responses when handling
                the events and routing them back to the originating calling
                thread.
            2.  The SDK will **not** provide the ‘Posted’ versions of these APIs

        When dispatching callbacks from the SDK, applications can call back into
        the SDK safely without grabbing the CHIP lock. Applications however may
        not make any assumptions about the specific threading context on that
        callback (i.e whether it’s running on the CHIP thread or not).

-   In a pure greenfield development model, most agree that 1a) would have been
    ideal. It places the onus of ensuring thread safety on SDK developers to
    solve, leaving application developers unencumbered and unlikely to make
    mistakes across arguably, a wide swatch of developer capabilities.

-   However:

    -   Switching to an async, event-post model pervasively requires a lot of
        changes to the implementation to provide appropriate callbacks + add
        state machines. The stack is just not designed in this way today

    -   Can potentially entail a lot of flash cost (\~150 bytes / call-site in
        the IM
        [_PR_](https://github.com/project-chip/connectedhomeip/pull/7117#issuecomment-849999350)
        that Yufeng put out), which can eventually end up costing a lot (needs
        to be ratified with some analysis though).

    -   Switching to a narrowly-scoped lock model requires a lot of up-front
        analysis (which we should do anyways) as to the critical sections
        involved, and whether there can be potential to have races/deadlocks in
        certain call-paths

-   **Recommendation:**

    -   Go with 2 (use external synchronization rather than thread-safe APIs)
        for now, with well-written documentation that stipulates the
        expectations of application developers, as well as example applications
        that are updated to showcase the different approaches available to them.

        -   On Linux, example apps could use the post-event using lambdas +
            closures, or grab the lock.
        -   On embedded, example apps could just run on the CHIP thread

    -   Easiest to deploy today.

    -   Continue to investigate whether we can incrementally move to 1
        (thread-safe APIs) in the future, including doing the critical section
        analysis (which we should do anyways).
