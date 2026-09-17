/* @@@LICENSE
 *
 * Copyright (c) 2017-2025 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */
#include "LsRequester.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>

#include <lib/support/logging/CHIPLogging.h>

// LS_REQ_SERVICE_NAME can be overridden at build time.
#ifndef LS_REQ_SERVICE_NAME
#define LS_REQ_SERVICE_NAME nullptr
#endif

std::atomic<LsRequester *> LsRequester::_singleton;
std::mutex LsRequester::_mutex;

struct SyncCallbackContext
{
    std::mutex mutex;
    std::condition_variable cond;
    std::string result;
    bool timeOut  = false;
    bool error    = false;
    bool received = false;
    std::atomic<bool> ownerReleased{ false };
};

void * LsRequester::lsTask(void * arg)
{
    auto * self = static_cast<LsRequester *>(arg);
    {
        // Signal readiness right before entering the loop so initLocked()'s wait below - and thus
        // any later g_main_loop_is_running() check in getInstance() - never observes the transient
        // "thread created but not yet scheduled" state as "not running".
        std::lock_guard<std::mutex> lock(self->m_startMutex);
        self->m_running = true;
    }
    self->m_startCv.notify_all();
    g_main_loop_run(self->m_mainLoop);
    {
        std::lock_guard<std::mutex> lock(self->m_startMutex);
        self->m_running = false;
    }
    return NULL;
}

LsRequester * LsRequester::getInstance()
{
    LsRequester * inst = _singleton.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if (inst == 0)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        inst = _singleton.load(std::memory_order_relaxed);
        if (inst == 0)
        {
            inst = new LsRequester();
            std::atomic_thread_fence(std::memory_order_release);
            _singleton.store(inst, std::memory_order_relaxed);
        }
        return inst;
    }
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (inst->m_state != State::RUNNING)
        {
            ChipLogError(DeviceLayer, "LsRequester not running, restarting...");
            inst->restartLocked();
        }
    }
    return inst;
}

// Assumes _mutex is held by the caller.
void LsRequester::initLocked()
{
    m_state              = State::STARTING;
    GMainContext * pCxt = g_main_context_new();
    m_mainLoop          = g_main_loop_new(pCxt, false);
    // g_main_loop_new takes its own reference on the context; drop ours so the
    // context is released together with the loop in stopLocked() (avoids a leak per restart()).
    g_main_context_unref(pCxt);
    try
    {
        m_handle = LS::registerService(LS_REQ_SERVICE_NAME);
        m_handle.attachToLoop(m_mainLoop);
        m_running = false;
        m_thread  = g_thread_new("lsTask", lsTask, this);
        {
            // Block until lsTask() has actually entered g_main_loop_run(). Without this, a caller
            // racing getInstance() right after construction/restart could see the loop as "not
            // running" and trigger a spurious restart (see m_running's declaration for why).
            std::unique_lock<std::mutex> startLock(m_startMutex);
            if (!m_startCv.wait_for(startLock, std::chrono::seconds(2), [this] { return m_running; }))
            {
                ChipLogError(DeviceLayer, "LsRequester main loop did not start within timeout");
            }
        }
        // Only claim RUNNING if the worker actually confirmed it; otherwise leave it as STARTING
        // so a subsequent getInstance() health check still attempts a restart instead of trusting
        // a loop that may never come up.
        m_state = m_running ? State::RUNNING : State::STARTING;
        ChipLogDetail(DeviceLayer, "LsRequester initialized, m_mainLoop: %p, m_thread: %p", m_mainLoop, m_thread);
    } catch (const LS::Error & e)
    {
        m_state = State::STOPPED;
        ChipLogError(DeviceLayer, "LsRequester init failed: %s", e.what());
    }
}

LsRequester::LsRequester()
{
    // Constructed only from getInstance() while holding _mutex, so initLocked() is safe here.
    initLocked();
}

LsRequester::~LsRequester()
{
    stop();
}

void LsRequester::restart()
{
    std::lock_guard<std::mutex> lock(_mutex);
    restartLocked();
}

// Assumes _mutex is held by the caller.
void LsRequester::restartLocked()
{
    stopLocked();
    initLocked();
}

void LsRequester::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    stopLocked();
}

void LsRequester::stopLocked()
{
    try
    {
        // m_state (not g_main_loop_is_running()) is the source of truth for whether the worker
        // actually reached g_main_loop_run() and therefore needs to be told to quit.
        if (m_mainLoop && m_state == State::RUNNING)
            g_main_loop_quit(m_mainLoop);
        m_handle.detach();

        if (m_thread)
        {
            // Wait for lsTask() to actually return (g_main_loop_run() to unwind) before the
            // GMainLoop is unreffed below - g_thread_join() also frees the GThread, so no separate
            // g_thread_unref() is needed or safe to call afterwards.
            g_thread_join(m_thread);
            m_thread = nullptr;
        }
        if (m_mainLoop)
        {
            g_main_loop_unref(m_mainLoop);
            m_mainLoop = nullptr;
        }
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "Exception: %s", e.what());
    }
    m_state = State::STOPPED;
}

bool LsRequester::_callbackSync(LSHandle * sh, LSMessage * reply, void * ctx)
{
    auto * ctxOwner                         = static_cast<std::shared_ptr<SyncCallbackContext> *>(ctx);
    std::shared_ptr<SyncCallbackContext> cc = *ctxOwner;

    LS::Message response(reply);
    bool result = true;
    {
        std::lock_guard<std::mutex> lock(cc->mutex);
        if (cc->timeOut || cc->error)
        {
            ChipLogError(DeviceLayer, "return by timeout or error");
            result = false;
        }
        else
        {
            cc->result.assign(response.getPayload());
            cc->received = true;
            cc->cond.notify_all();
            // ChipLogDetail(DeviceLayer, "Response: %s", response.getPayload());
        }
    }

    bool expected = false;
    if (cc->ownerReleased.compare_exchange_strong(expected, true))
    {
        delete ctxOwner;
    }
    return result;
}

bool LsRequester::lsCallSync(const char * pAPI, const char * pParams, pbnjson::JValue & response, int timeout)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (pAPI == NULL || pParams == NULL)
    {
        ChipLogError(DeviceLayer, "Invalid API or params: %s, %s", pAPI, pParams);
        return false;
    }
    // ChipLogDetail(DeviceLayer, "lsCallSync calling API: %s, params: %s, timeout: %d", pAPI, pParams, timeout);
    auto cc         = std::make_shared<SyncCallbackContext>();
    auto * ctxOwner = new std::shared_ptr<SyncCallbackContext>(cc);
    bool retVal     = true;
    bool gotReply   = false;
    try
    {
        auto call = m_handle.callOneReply(pAPI, pParams, _callbackSync, (void *) ctxOwner);

        std::unique_lock<std::mutex> waitLock(cc->mutex);
        gotReply = cc->cond.wait_for(waitLock, std::chrono::seconds(timeout), [&]() { return cc->received; });
        if (!gotReply)
        {
            cc->timeOut = true;
            waitLock.unlock();
            ChipLogError(DeviceLayer, "lsCallSync timed out after %d seconds for API: %s", timeout, pAPI);
            call.cancel();
            // If cancel() fully suppressed the pending callback, _callbackSync() will never run to
            // free ctxOwner - claim and free it here instead. If a callback is still in flight, it
            // will win the race on ownerReleased and this becomes a no-op.
            bool expected = false;
            if (cc->ownerReleased.compare_exchange_strong(expected, true))
            {
                delete ctxOwner;
            }
            return false;
        }
        // ChipLogDetail(DeviceLayer, "lsCallSync received response for API: %s, result: %s", pAPI, cc->result.c_str());
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "LS::Error in lsCallSync for API: %s, error: %s", pAPI, e.what());
        // The callback was never registered with LS2 for this attempt, so it will never run
        // to release ctxOwner - release it here instead.
        delete ctxOwner;
        retVal = false;
    } catch (const std::system_error & e)
    {
        std::lock_guard<std::mutex> errLock(cc->mutex);
        cc->error = true;
        ChipLogError(DeviceLayer, "System error in lsCallSync for API: %s, error: %s", pAPI, e.what());
        retVal = false;
    }

    if (gotReply)
    {
        std::lock_guard<std::mutex> resultLock(cc->mutex);
        response = pbnjson::JDomParser::fromString(cc->result);
    }
    // ChipLogDetail(DeviceLayer, "lsCallSync completed for API: %s, response: %s", pAPI, response.stringify().c_str());
    return retVal;
}

bool LsRequester::lsCallCancel(LSMessageToken ulToken)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (!m_mainLoop || m_state != State::RUNNING)
    {
        ChipLogError(DeviceLayer, "LsRequester is not running");
        return false;
    }

    if (ulToken == LSMESSAGE_TOKEN_INVALID)
    {
        ChipLogError(DeviceLayer, "Invalid token");
        return false;
    }

    LSError lserror;
    LSErrorInit(&lserror);
    if (!LSCallCancel(m_handle.get(), ulToken, &lserror))
    {
        ChipLogError(DeviceLayer, "Failed to CallCancel: %s", lserror.message);
        LSErrorFree(&lserror);
        return false;
    }
    ChipLogDetail(DeviceLayer, "Success to CallCancel Token %lu ", ulToken);
    return true;
}

bool LsRequester::lsSubscribe(const char * pAPI, const char * pParams, void * ctx, LSFilterFunc func, LS::Call & call)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (call.isActive())
    {
        call.cancel();
    }

    ChipLogDetail(DeviceLayer, "API : %s, params: %s", pAPI, pParams);

    try
    {
        call = m_handle.callMultiReply(pAPI, pParams);
        call.continueWith(func, ctx);
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool LsRequester::lsSubscribe(const char * pAPI, const char * pParams, void * ctx, LSFilterFunc func, LSMessageToken * pulToken)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (*pulToken != LSMESSAGE_TOKEN_INVALID)
    {
        LSError lserror;
        LSErrorInit(&lserror);
        if (!LSCallCancel(m_handle.get(), *pulToken, &lserror))
        {
            ChipLogError(DeviceLayer, "Failed to CallCancel: %s", lserror.message);
            LSErrorFree(&lserror);
        }
    }

    ChipLogDetail(DeviceLayer, "API : %s, params: %s", pAPI, pParams);

    LSError lserror;
    LSErrorInit(&lserror);
    bool ok = false;
    try
    {
        ok = LSCall(m_handle.get(), pAPI, pParams, func, ctx, pulToken, &lserror);
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "Exception: %s", e.what());
        LSErrorFree(&lserror);
        return false;
    }
    if (!ok)
    {
        ChipLogError(DeviceLayer, "LSCall failed for API: %s, error: %s", pAPI, lserror.message);
        LSErrorFree(&lserror);
        return false;
    }
    LSErrorFree(&lserror);
    ChipLogDetail(DeviceLayer, "lsSubscribe Success API : %s", pAPI);
    return true;
}
