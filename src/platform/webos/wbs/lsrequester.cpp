/* @@@LICENSE
 *
 * Copyright (c) 2017 LG Electronics, Inc.
 *
 * Confidential computer software. Valid license from LG required for
 * possession, use or copying. Consistent with FAR 12.211 and 12.212,
 * Commercial Computer Software, Computer Software Documentation, and
 * Technical Data for Commercial Items are licensed to the U.S. Government
 * under vendor's standard commercial license.
 *
 * LICENSE@@@
 */
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <new>
#include <string>

#include <lib/support/logging/CHIPLogging.h>

#include "lsrequester.h"

// Can be overridden at build time (e.g. -DLS_REQ_SERVICE_NAME=...) to match the product LS2 role.
#ifndef LS_REQ_SERVICE_NAME
#define LS_REQ_SERVICE_NAME "com.webos.service.unifiedmatter-req"
#endif

// How long lsCallSync() waits for the cancel to be processed on the lsTask thread after a timeout.
#define LS_SYNC_CANCEL_WAIT_SEC 2

std::atomic<LsRequester*> LsRequester::_singleton;
std::mutex LsRequester::_mutex;

namespace {

// Shared between lsCallSync() (caller thread) and _callbackSync() / CancelOnLsTask() (lsTask thread).
// All fields are guarded by `mutex`.
//  - Normal path: _callbackSync() stores the reply and the caller frees the context.
//  - Timeout path: the call is cancelled on the lsTask thread, which is the only thread that dispatches
//    _callbackSync(), so no reply can be in flight once CancelOnLsTask() has run and the caller frees the
//    context. If even that does not complete in time, the context is orphaned and CancelOnLsTask() frees it.
struct SyncCallbackContext {
    std::mutex mutex;
    std::condition_variable cond;
    std::string result;
    LSHandle * handle = nullptr;
    LSMessageToken token = LSMESSAGE_TOKEN_INVALID;
    bool received = false;
    bool cancelled = false;
    bool orphaned = false;
};

gboolean CancelOnLsTask(gpointer data)
{
    SyncCallbackContext * cc = static_cast<SyncCallbackContext *>(data);
    LSCallCancel(cc->handle, cc->token, NULL);

    std::unique_lock<std::mutex> lock(cc->mutex);
    if (cc->orphaned)
    {
        lock.unlock();
        delete cc;
        return G_SOURCE_REMOVE;
    }
    cc->cancelled = true;
    cc->cond.notify_all();
    return G_SOURCE_REMOVE;
}

gboolean QuitLoop(gpointer data)
{
    g_main_loop_quit(static_cast<GMainLoop *>(data));
    return G_SOURCE_REMOVE;
}

} // namespace

void *LsRequester::lsTask(void *arg)
{
    g_main_loop_run((GMainLoop*)arg);
    return NULL;
}

LsRequester* LsRequester::getInstance()
{
    LsRequester* inst = _singleton.load(std::memory_order_relaxed);
    std::atomic_thread_fence(std::memory_order_acquire);
    if(inst == 0) {
        std::lock_guard<std::mutex> lock(_mutex);
        inst = _singleton.load(std::memory_order_relaxed);
        if(inst == 0) {
            inst = new LsRequester();
            std::atomic_thread_fence(std::memory_order_release);
            _singleton.store(inst, std::memory_order_relaxed);
        }
    }
    return inst;
}

LsRequester::LsRequester()
{
    GMainContext *pCxt = g_main_context_new();
    m_mainLoop = g_main_loop_new( pCxt, false);
    // g_main_loop_new() holds its own reference to the context.
    g_main_context_unref(pCxt);
    try
    {
        m_handle = LS::registerService(LS_REQ_SERVICE_NAME);
        m_handle.attachToLoop(m_mainLoop);
        m_thread = g_thread_new("lsTask", lsTask, (GMainLoop*)m_mainLoop);
    }
    catch(const LS::Error& e)
    {
        ChipLogError(DeviceLayer, "Exception %s", e.what());
    }
}

LsRequester::~LsRequester()
{
    stop();
}

void LsRequester::stop()
{
    GMainLoop * loop = nullptr;
    GThread * thread = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        loop = m_mainLoop;
        thread = m_thread;
        m_mainLoop = nullptr;
        m_thread = nullptr;
    }
    if (loop == nullptr)
        return;

    // Quit from inside the loop: g_main_loop_quit() called before lsTask() entered g_main_loop_run()
    // would be lost and the join below would never return.
    if (thread != nullptr)
    {
        g_main_context_invoke(g_main_loop_get_context(loop), QuitLoop, loop);
        // Join (not just unref) so that the loop is no longer running when it is released below.
        // Done without holding _mutex because callbacks on lsTask may need it.
        g_thread_join(thread);
    }

    {
        std::lock_guard<std::mutex> lock(_mutex);
        try
        {
            m_handle.detach();
        }
        catch(const LS::Error& e)
        {
            ChipLogError(DeviceLayer, "Exception: %s", e.what());
        }
    }
    g_main_loop_unref(loop);
}

bool LsRequester::_callbackSync(LSHandle *sh, LSMessage *reply, void *ctx)
{
    SyncCallbackContext *cc = static_cast<SyncCallbackContext *>(ctx);
    const char * payload = LSMessageGetPayload(reply);
    //ChipLogDetail(DeviceLayer, "Response: %s", payload);

    std::lock_guard<std::mutex> lock(cc->mutex);
    cc->result.assign(payload != nullptr ? payload : "");
    cc->received = true;
    cc->cond.notify_all();

    return true;
}

bool LsRequester::lsCallSync(const char* pAPI, const char* pParams, pbnjson::JValue &response, int timeout)
{
    if(pAPI == NULL || pParams == NULL)
        return false;

    //ChipLogDetail(DeviceLayer, "API : %s, params: %s", pAPI, pParams);

    SyncCallbackContext *cc = new (std::nothrow) SyncCallbackContext();
    if (cc == nullptr)
        return false;

    GMainContext * lsContext = nullptr;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (m_mainLoop == nullptr)
        {
            delete cc;
            return false;
        }

        LSError lserror;
        LSErrorInit(&lserror);
        cc->handle = m_handle.get();
        if (!LSCallOneReply(cc->handle, pAPI, pParams, _callbackSync, cc, &cc->token, &lserror))
        {
            ChipLogError(DeviceLayer, "LSCallOneReply failed (%s): %s", pAPI, lserror.message);
            LSErrorFree(&lserror);
            delete cc;
            return false;
        }
        lsContext = g_main_context_ref(g_main_loop_get_context(m_mainLoop));
    }

    std::unique_lock<std::mutex> lock(cc->mutex);
    if (!cc->cond.wait_for(lock, std::chrono::seconds(timeout), [cc]() { return cc->received; }))
    {
        ChipLogError(DeviceLayer, "timeout: %d sec (%s)", timeout, pAPI);
        lock.unlock();
        g_main_context_invoke(lsContext, CancelOnLsTask, cc);
        g_main_context_unref(lsContext);

        lock.lock();
        if (!cc->cond.wait_for(lock, std::chrono::seconds(LS_SYNC_CANCEL_WAIT_SEC), [cc]() { return cc->cancelled; }))
        {
            // lsTask is not responsive: CancelOnLsTask() releases the context when it eventually runs.
            cc->orphaned = true;
            return false;
        }
        lock.unlock();
        delete cc;
        return false;
    }

    std::string result = std::move(cc->result);
    lock.unlock();
    delete cc;
    g_main_context_unref(lsContext);

    response = pbnjson::JDomParser::fromString(result);
    return true;
}

bool LsRequester::lsCallCancel(LSMessageToken& ulToken)
{
    std::lock_guard<std::mutex> lock(_mutex);
    ChipLogProgress(DeviceLayer, "lsCallCancel %lu",ulToken);

    bool bRet = false;
    if (ulToken != LSMESSAGE_TOKEN_INVALID)
    {
        try {
            LSError lserror;
            LSErrorInit(&lserror);
            bRet = LSCallCancel(m_handle.get(), ulToken, &lserror);
            if (!bRet)
            {
                ChipLogError(DeviceLayer, "Failed to CallCancel: %s", lserror.message);
                LSErrorFree(&lserror);
            }
            ulToken = LSMESSAGE_TOKEN_INVALID;
        } catch (std::exception& error) {
            ChipLogError(DeviceLayer, "Failed to CallCancel (%lu) : %s", ulToken, error.what());
        }
    }
    else {
        ChipLogDetail(DeviceLayer, "lsCallCancel2 ulToken == LSMESSAGE_TOKEN_INVALID (%lu)",ulToken);
    }
    return bRet;
}

bool LsRequester::lsSubscribe(const char* pAPI, const char* pParams, void* ctx, LSFilterFunc func, LS::Call& call)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (call.isActive()) {
        call.cancel();
    }

    //ChipLogDetail(DeviceLayer, "API : %s, params: %s", pAPI, pParams);

    try {
        call = m_handle.callMultiReply(pAPI, pParams);
        call.continueWith(func, ctx);
    }
    catch (const LS::Error &e) {
        ChipLogError(DeviceLayer, "Exception: %s", e.what());
        return false;
    }
    return true;
}

bool LsRequester::lsSubscribe(const char* pAPI, const char* pParams, void* ctx, LSFilterFunc func, LSMessageToken *pulToken)
{
    std::lock_guard<std::mutex> lock(_mutex);
    LSError lserror;
    LSErrorInit(&lserror);
    if (*pulToken != LSMESSAGE_TOKEN_INVALID )
    {
        if (!LSCallCancel(m_handle.get(), *pulToken, &lserror))
        {
            ChipLogError(DeviceLayer, "Failed to CallCancel: %s", lserror.message);
            LSErrorFree(&lserror);
        }
        *pulToken = LSMESSAGE_TOKEN_INVALID;
    }

    //ChipLogDetail(DeviceLayer, "API : %s, params: %s", pAPI, pParams);

    // LSCall() is a C API: failures are reported through the return value / LSError, not exceptions.
    LSErrorInit(&lserror);
    if (!LSCall(m_handle.get(), pAPI, pParams, func, ctx, pulToken, &lserror))
    {
        ChipLogError(DeviceLayer, "LSCall failed (%s): %s", pAPI, lserror.message);
        LSErrorFree(&lserror);
        *pulToken = LSMESSAGE_TOKEN_INVALID;
        return false;
    }
    return true;
}
