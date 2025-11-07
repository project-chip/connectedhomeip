/**
 * A main loop implementation describes how an application main loop is to be
 * run:
 *    - how to execute the main loop
 *    - what to do to stop it (inside a signal handler - CTRL+C is captured
 *      by the main loop function)
 */
class AppMainLoopImplementation
{
public:
    virtual ~AppMainLoopImplementation() = default;
    /**
     * Execute main loop. Generally should have at least some
     * `DeviceLayer::PlatformMgr().RunEventLoop();` or equivalent setup
     *
     * This is expected to RUN and BLOCK until SignalSafeStopMainLoop is
     * called or some internal close logic is run (e.g. a UI may
     * stop when the window close button is clicked.)
     */
    virtual void RunMainLoop() = 0;

    /**
     * Stop the above `RunMainLoop` function.
     *
     * Generally should contain at least a
     *
     *    Server::GetInstance().GenerateShutDownEvent()
     *
     * and then call StopEventLoopTask() in whatever way is appropriate for the
     * way the event loop was started.
     */
    virtual void SignalSafeStopMainLoop() = 0;
};
