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
#include <lib/support/logging/CHIPLogging.h>

#define LS_REQ_SERVICE_NAME "com.webos.service.unifiedmatter-req"

std::atomic<LsRequester *> LsRequester::_singleton;
std::mutex LsRequester::_mutex;

struct SyncCallbackContext
{
    std::mutex mutex;
    std::unique_lock<std::mutex> lock;
    std::condition_variable cond;
    std::string result;
    bool alive;
    bool timeOut;
    bool error;
    bool received;

    explicit SyncCallbackContext() : lock(mutex), alive(false), timeOut(false), error(false), received(false) {}

    bool wait(int timeout)
    {
        if (received)
        {
            return true;
        }

        std::chrono::seconds sec(timeout);
        return cond.wait_for(lock, sec, [&]() { return !result.empty(); });
    }
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
    }
    // GMainLoop 상태 점검
    if (!g_main_loop_is_running(inst->m_mainLoop))
    {
        ChipLogError(DeviceLayer, "Main loop not running, restarting...");
        inst->stop();
        inst->restart();
    }
    return inst;
}

LsRequester::LsRequester()
{
    GMainContext * pCxt = g_main_context_new();
    m_mainLoop          = g_main_loop_new(pCxt, false);
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

LsRequester::~LsRequester()
{
    stop();
}

void LsRequester::restart()
{
    stop();
    GMainContext * pCxt = g_main_context_new();
    m_mainLoop          = g_main_loop_new(pCxt, false);
    try
    {
        m_handle = LS::registerService(LS_REQ_SERVICE_NAME);
        m_handle.attachToLoop(m_mainLoop);
        m_thread = g_thread_new("lsTask", lsTask, (GMainLoop *) m_mainLoop);
        ChipLogDetail(DeviceLayer, "LsRequester restarted");
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "LsRequester restart failed: %s", e.what());
    }
}

void LsRequester::stop()
{
    std::lock_guard<std::mutex> lock(_mutex);
    try
    {
        if (g_main_loop_is_running(m_mainLoop))
            g_main_loop_quit(m_mainLoop);
        m_handle.detach();
        g_thread_unref(m_thread);

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
    LS::Message response(reply);
    auto * cc = static_cast<SyncCallbackContext *>(ctx);
    if (cc->timeOut || cc->error)
    {
        delete cc;
        ChipLogError(DeviceLayer, "return by timeout or error");
        return false;
    }
    cc->alive = true;
    cc->result.assign(response.getPayload());
    cc->received = true;
    cc->cond.notify_all();
    cc->alive = false;
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
    SyncCallbackContext * cc = new SyncCallbackContext();
    bool retVal              = true;
    try
    {
        auto call = m_handle.callOneReply(pAPI, pParams, _callbackSync, (void *) cc);
        if (!cc->wait(timeout))
        {
            cc->timeOut = true;
            ChipLogError(DeviceLayer, "lsCallSync timed out after %d seconds for API: %s", timeout, pAPI);
            call.cancel();
            delete cc;
            return false;
        }
        else
        {
            // ChipLogDetail(DeviceLayer, "lsCallSync received response for API: %s, result: %s", pAPI, cc->result.c_str());
        }
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "LS::Error in lsCallSync for API: %s, error: %s", pAPI, e.what());
        retVal = false;
    } catch (const std::system_error & e)
    {
        cc->error = true;
        ChipLogError(DeviceLayer, "System error in lsCallSync for API: %s, error: %s", pAPI, e.what());
        retVal = false;
    }

    response = pbnjson::JDomParser::fromString(cc->result);
    // ChipLogDetail(DeviceLayer, "lsCallSync completed for API: %s, response: %s", pAPI, response.stringify().c_str());
    if (cc)
        delete cc;
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

    try
    {
        LSCall(m_handle.get(), pAPI, pParams, func, ctx, pulToken, NULL);
    } catch (const LS::Error & e)
    {
        ChipLogError(DeviceLayer, "Exception: %s", e.what());
        return false;
    }
    ChipLogDetail(DeviceLayer, "lsSubscribe Success API : %s", pAPI);
    return true;
}
