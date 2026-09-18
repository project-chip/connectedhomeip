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
#pragma once
#ifndef LSREQUESTER_H_
#define LSREQUESTER_H_

#include <condition_variable>
#include <luna-service2/lunaservice.hpp>
#include <mutex>
#include <pbnjson.hpp>

#define STR_RETURN_VALUE "returnValue"

class LsRequester
{
public:
    static LsRequester * getInstance();
    void stop();
    void restart();
    bool lsCallSync(const char * pAPI, const char * pParams, pbnjson::JValue & response, int timeout = 10);
    bool lsSubscribe(const char * pAPI, const char * pParams, void * ctx, LSFilterFunc func, LS::Call & call);
    bool lsSubscribe(const char * pAPI, const char * pParams, void * ctx, LSFilterFunc func, LSMessageToken * pulToken);
    bool lsCallCancel(LSMessageToken ulToken);

    static bool _callbackSync(LSHandle * sh, LSMessage * reply, void * ctx);

private:
    LsRequester();
    virtual ~LsRequester();

    // *Locked() helpers assume the caller already holds _mutex.
    void stopLocked();
    void initLocked();
    void restartLocked();
    static void * lsTask(void * arg);
    GMainLoop * m_mainLoop = nullptr;
    LS::Handle m_handle;

    static std::atomic<LsRequester *> _singleton;
    static std::mutex _mutex;
    GThread * m_thread = nullptr;

    std::mutex m_startMutex;
    std::condition_variable m_startCv;
    bool m_running = false;

    enum class State
    {
        STOPPED,
        STARTING,
        RUNNING,
    };
    State m_state = State::STOPPED;
};

#endif /* LSREQUESTER_H_ */
