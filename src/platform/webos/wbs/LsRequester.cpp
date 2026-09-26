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
#define LS_REQ_SERVICE_NAME "com.webos.service.unifiedmatter-req"
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
};

void * LsRequester::lsTask(void * arg)
{
    g_main_loop_run((GMainLoop *) arg);
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
        if (!inst->m_mainLoop || !g_main_loop_is_running(inst->m_mainLoop))
        {
            ChipLogError(DeviceLayer, "Main loop not running, restarting...");
            inst->restartLocked();
        }
    }
    return inst;
}

// Assumes _mutex is held by the caller.
void LsRequester::initLocked()
{
    GMainContext * pCxt = g_main_context_new();
    m_mainLoop          = g_main_loop_new(pCxt, false);
    // g_main_loop_new takes its own reference on the context; drop ours so the
    // context is released together with the loop in stopLocked() (avoids a leak per restart()).
    g_main_context_unref(pCxt);
    try
    {
        m_handle = LS::registerService(LS_REQ_SERVICE_NAME);
        m_handle.attachToLoop(m_mainLoop);
        m_thread = g_thread_new("lsTask", lsTask, (GMainLoop *) m_mainLoop);
        ChipLogDetail(DeviceLayer, "LsRequester initialized, m_mainLoop: %p, m_thread: %p", m_mainLoop, m_thread);
    } catch (const LS::Error & e)
    {
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
        if (m_mainLoop && g_main_loop_is_running(m_mainLoop))
            g_main_loop_quit(m_mainLoop);
        m_handle.detach();

        if (m_thread)
        {
            g_thread_unref(m_thread);
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
}

bool LsRequester::_callbackSync(LSHandle * sh, LSMessage * reply, void * ctx)
{
    // ctx owns one shared_ptr reference to the context; releasing this wrapper drops that
    // reference. The context itself stays alive as long as lsCallSync()'s local shared_ptr
    // (or this one) still references it, so it is never freed while either side is using it.
    std::unique_ptr<std::shared_ptr<SyncCallbackContext>> ctxOwner(static_cast<std::shared_ptr<SyncCallbackContext> *>(ctx));
    std::shared_ptr<SyncCallbackContext> cc = *ctxOwner;

    LS::Message response(reply);
    std::lock_guard<std::mutex> lock(cc->mutex);
    if (cc->timeOut || cc->error)
    {
        ChipLogError(DeviceLayer, "return by timeout or error");
        return false;
    }
    cc->result.assign(response.getPayload());
    cc->received = true;
    cc->cond.notify_all();
    // ChipLogDetail(DeviceLayer, "Response: %s", response.getPayload());
    return true;
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
    if (!m_mainLoop || !g_main_loop_is_running(m_mainLoop))
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
