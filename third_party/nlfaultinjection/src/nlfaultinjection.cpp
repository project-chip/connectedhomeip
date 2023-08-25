/*
 *
 *    Copyright 2016-2018 The nlfaultinjection Authors.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 */

/**
 *    @file
 *      Implementation of the fault-injection utilities.
 */

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <nlassert.h>

#include <nlfaultinjection.hpp>

namespace nl {

namespace FaultInjection {

static void Die() __attribute__((noreturn));

static GlobalContext * sGlobalContext = nullptr;

/**
 * The callback function that implements the deterministic
 * injection feature (see FailAtFault).
 */
static bool DeterministicCbFn(Identifier aId, Record * aRecord, void * aContext)
{
    bool retval = false;

    (void) aId;
    (void) aContext;

    if (aRecord->mNumCallsToSkip)
    {
        aRecord->mNumCallsToSkip--;
    }
    else if (aRecord->mNumCallsToFail)
    {
        aRecord->mNumCallsToFail--;
        retval = true;
    }

    return retval;
}

/**
 * Callback list node for DeterministicCbFn.
 * This node terminates all callback lists.
 */
static Callback sDeterministicCb = { DeterministicCbFn, nullptr, nullptr };

/**
 * The callback function that implements the random
 * injection feature (see FailRandomlyAtFault).
 */
static bool RandomCbFn(Identifier aId, Record * aRecord, void * aContext)
{
    bool retval = false;

    (void) aId;
    (void) aContext;

    if (aRecord->mPercentage > 0)
    {
        int randValue = (rand() % 100) + 1;
        if (randValue <= aRecord->mPercentage)
        {
            retval = true;
        }
    }

    return retval;
}

/**
 * Callback list node for RandomCbFn.
 * Note that this is initialized to point to sDeterministicCb.
 * All Record instances are initialized to point to
 * this callback node.
 */
static Callback sRandomCb = { RandomCbFn, nullptr, &sDeterministicCb };

/**
 * Alias for the address of the first default callback.
 */
static const Callback * sEndOfCustomCallbacks = &sRandomCb;

/**
 * Initialize the Manager instance.
 *
 * @param[in]   inNumFaults     The size of inFaultArray, equal to the number of fault IDs.
 * @param[in]   inFaultArray    A pointer to an array of Record, in which this object
 *                              will store the configuration of each fault.
 * @param[in]   inManagerName   A pointer to a C string containing the name of the Manager.
 * @param[in]   inFaultNames    A pointer to an array of inNumFaults C strings that describe
 *                              each fault ID.
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::Init(size_t inNumFaults, Record * inFaultArray, Name inManagerName, const Name * inFaultNames)
{
    int32_t err = 0;
    Identifier i;

    nlEXPECT_ACTION((inNumFaults > 0 && inFaultArray && inManagerName && inFaultNames), exit, err = -EINVAL);

    mName         = inManagerName;
    mNumFaults    = inNumFaults;
    mFaultRecords = inFaultArray;
    mFaultNames   = inFaultNames;
    mLock         = nullptr;
    mUnlock       = nullptr;
    mLockContext  = nullptr;

    // Link all callback lists to the two default callbacks.
    for (i = 0; i < mNumFaults; i++)
    {
        mFaultRecords[i].mCallbackList = &sRandomCb;
    }

exit:
    return err;
}

/**
 * Configure a fault to be triggered randomly, with a given probability defined as a percentage
 * This is meant to be used on live systems to generate a build that will encounter random failures.
 *
 * @param[in]   inId            The fault ID
 * @param[in]   inPercentage    An integer between 0 and 100. 100 means "always". 0 means "never".
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::FailRandomlyAtFault(Identifier inId, uint8_t inPercentage)
{
    int32_t err = 0;

    nlEXPECT_ACTION((inId < mNumFaults && inPercentage <= 100), exit, err = -EINVAL);

    Lock();

    mFaultRecords[inId].mNumCallsToSkip = 0;
    mFaultRecords[inId].mNumCallsToFail = 0;
    mFaultRecords[inId].mPercentage     = inPercentage;

    Unlock();

exit:
    return err;
}

/**
 * Configure a fault to be triggered deterministically.
 *
 * @param[in]   inId                The fault ID
 * @param[in]   inNumCallsToSkip    The number of times this fault is to be skipped before it
 *                                  starts to fail.
 * @param[in]   inNumCallsToFail    The number of times the fault should be triggered.
 * @param[in]   inTakeMutex         By default this method takes the Manager's mutex.
 *                                  If inTakeMutex is set to kMutexDoNotTake, the mutex is not taken.
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::FailAtFault(Identifier inId, uint32_t inNumCallsToSkip, uint32_t inNumCallsToFail, bool inTakeMutex)
{
    int32_t err = 0;

    nlEXPECT_ACTION(inId < mNumFaults && inNumCallsToSkip <= UINT16_MAX && inNumCallsToFail <= UINT16_MAX, exit, err = -EINVAL);

    if (inTakeMutex)
    {
        Lock();
    }

    mFaultRecords[inId].mNumCallsToSkip = static_cast<uint16_t>(inNumCallsToSkip);
    mFaultRecords[inId].mNumCallsToFail = static_cast<uint16_t>(inNumCallsToFail);
    mFaultRecords[inId].mPercentage     = 0;

    if (inTakeMutex)
    {
        Unlock();
    }

exit:
    return err;
}

/**
 * @overload int32_t FailAtFault(Identifier inId, uint32_t inNumCallsToSkip, uint32_t inNumCallsToFail, bool inTakeMutex)
 */
int32_t Manager::FailAtFault(Identifier inId, uint32_t inNumCallsToSkip, uint32_t inNumCallsToFail)
{
    return FailAtFault(inId, inNumCallsToSkip, inNumCallsToFail, kMutexTake);
}

/**
 * Configure a fault to reboot the system when triggered.
 * If the application has installed a RebootCallbackFn, it will
 * be invoked when fault inId is triggered.
 * If the application has not installed the callback, the system
 * will crash.
 *
 * @param[in]   inId                The fault ID
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::RebootAtFault(Identifier inId)
{
    int32_t err = 0;

    nlEXPECT_ACTION(inId < mNumFaults, exit, err = -EINVAL);

    Lock();

    mFaultRecords[inId].mReboot = true;

    Unlock();

exit:
    return err;
}

/**
 * Store a set of arguments for a given fault ID.
 * The array of arguments is made available to the code injected with
 * the nlFAULT_INJECT macro.
 * For this to work for a given fault ID, the Manager must allocate memory to
 * store the arguments and configure the Record's mLengthOfArguments and
 * mArguments members accordingly.
 *
 * @param[in]   inId                The fault ID
 * @param[in]   inNumArgs           The number of arguments in the array pointed to by inArgs.
 * @param[in]   inArgs              The pointer to the array of integers to be stored in the fault
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::StoreArgsAtFault(Identifier inId, uint16_t inNumArgs, int32_t * inArgs)
{
    int32_t err = 0;
    size_t i;

    nlEXPECT_ACTION(inId < mNumFaults && mFaultRecords[inId].mArguments != nullptr &&
                        mFaultRecords[inId].mLengthOfArguments >= inNumArgs && inNumArgs <= UINT8_MAX,
                    exit, err = -EINVAL);

    Lock();

    for (i = 0; i < inNumArgs; i++)
    {
        mFaultRecords[inId].mArguments[i] = inArgs[i];
    }

    mFaultRecords[inId].mNumArguments = static_cast<uint8_t>(inNumArgs);

    Unlock();

exit:
    return err;
}

/**
 * Attach a callback to a fault ID.
 * Calling this twice does not attach the callback twice.
 *
 * @param[in]   inId        The fault ID
 * @param[in]   inCallback  The callback node to be attached to the fault
 *
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::InsertCallbackAtFault(Identifier inId, Callback * inCallBack)
{
    int32_t err = 0;

    // Make sure it's not already there
    err = RemoveCallbackAtFault(inId, inCallBack);

    nlEXPECT_SUCCESS(err, exit);

    Lock();

    // Insert the callback at the beginning of the list.
    // Remember that all lists end into the two default (deterministic
    // and random) callbacks!
    inCallBack->mNext                 = mFaultRecords[inId].mCallbackList;
    mFaultRecords[inId].mCallbackList = inCallBack;

    Unlock();

exit:
    return err;
}

/**
 * Detaches a callback from a fault.
 *
 * @param[in]   inId        The fault
 * @param[in]   inCallback  The callback node to be removed.
 * @param[in]   inTakeMutex         By default this method takes the Manager's mutex.
 *                                  If inTakeMutex is set to kMutexDoNotTake, the mutex is not taken.
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::RemoveCallbackAtFault(Identifier inId, Callback * inCallBack, bool inTakeMutex)
{
    int32_t err    = 0;
    Callback ** cb = nullptr;

    nlEXPECT_ACTION((inId < mNumFaults) && (inCallBack != nullptr), exit, err = -EINVAL);

    if (inTakeMutex)
    {
        Lock();
    }

    cb = &mFaultRecords[inId].mCallbackList;

    while (*cb != nullptr)
    {
        if (*cb == inCallBack)
        {
            *cb = (*cb)->mNext;
            break;
        }
        cb = &((*cb)->mNext);
    }

    if (inTakeMutex)
    {
        Unlock();
    }

exit:
    return err;
}

/**
 * @overload int32_t Manager::RemoveCallbackAtFault(Identifier inId, Callback *inCallBack, bool inTakeMutex)
 */
int32_t Manager::RemoveCallbackAtFault(Identifier inId, Callback * inCallBack)
{
    return RemoveCallbackAtFault(inId, inCallBack, kMutexTake);
}

/**
 * When the program traverses the location at which a fault should be injected, this method is invoked
 * on the manager to query the configuration of the fault ID.
 *
 * A fault can be triggered randomly, deterministically or on a call-by-call basis by a callback.
 * All three types of trigger can be installed at the same time, and they all get a chance of
 * injecting the fault.
 *
 * @param[in] inId                The fault ID
 * @param[in] inTakeMutex         By default this method takes the Manager's mutex.
 *                                If inTakeMutex is set to kMutexDoNotTake, the mutex is not taken.
 *
 * @return    true if the fault should be injected; false otherwise.
 */
bool Manager::CheckFault(Identifier inId, bool inTakeMutex)
{
    bool retval     = false;
    Callback * cb   = nullptr;
    Callback * next = nullptr;
    bool reboot     = false;

    nlEXPECT(inId < mNumFaults, exit);

    if (inTakeMutex)
    {
        Lock();
    }

    cb = mFaultRecords[inId].mCallbackList;

    while (cb != nullptr)
    {
        // Save mNext now, in case the callback removes itself
        // calling RemoveCallbackAtFault
        next = cb->mNext;
        if (cb->mCallBackFn(inId, &mFaultRecords[inId], cb->mContext))
        {
            retval = true;
        }
        cb = next;
    }

    reboot = mFaultRecords[inId].mReboot;

    if (retval && sGlobalContext && sGlobalContext->mCbTable.mPostInjectionCb)
    {
        sGlobalContext->mCbTable.mPostInjectionCb(this, inId, &mFaultRecords[inId]);
    }

    if (retval && reboot)
    {
        // If the application has not setup a context and/or reboot callback, the system will crash
        if (sGlobalContext && sGlobalContext->mCbTable.mRebootCb)
        {
            sGlobalContext->mCbTable.mRebootCb();
        }
        else
        {
            Die();
        }
    }

    mFaultRecords[inId].mNumTimesChecked++;

    if (inTakeMutex)
    {
        Unlock();
    }

exit:
    return retval;
}

/**
 * @overload bool CheckFault(Identifier inId, bool inTakeMutex)
 */
bool Manager::CheckFault(Identifier inId)
{
    return CheckFault(inId, kMutexTake);
}

/**
 * When the program traverses the location at which a fault should be injected, this method is invoked
 * on the manager to query the configuration of the fault ID.
 *
 * This version of the method retrieves the arguments stored in the Record.
 *
 * A fault can be triggered randomly, deterministically or on a call-by-call basis by a callback.
 * All three types of trigger can be installed at the same time, and they all get a chance of
 * injecting the fault.
 *
 * @param[in] inId            The fault ID
 * @param[in] outNumArgs      The length of the array pointed to by outArgs
 * @param[in] outArgs         The array of arguments configured for the faultId
 * @param[in] inTakeMutex     By default this method takes the Manager's mutex.
 *                            If inTakeMutex is set to kMutexDoNotTake, the mutex is not taken.
 *
 * @return    true if the fault should be injected; false otherwise.
 */
bool Manager::CheckFault(Identifier inId, uint16_t & outNumArgs, int32_t *& outArgs, bool inTakeMutex)
{
    bool retval = false;

    if (inTakeMutex)
    {
        Lock();
    }

    retval = CheckFault(inId, kMutexDoNotTake);
    if (retval)
    {
        outNumArgs = mFaultRecords[inId].mNumArguments;
        outArgs    = mFaultRecords[inId].mArguments;
    }

    if (inTakeMutex)
    {
        Unlock();
    }

    return retval;
}

/**
 * @overload bool CheckFault(Identifier inId, uint16_t &outNumArgs, int32_t *&outArgs, bool inTakeMutex)
 */
bool Manager::CheckFault(Identifier inId, uint16_t & outNumArgs, int32_t *& outArgs)
{
    return CheckFault(inId, outNumArgs, outArgs, kMutexTake);
}

/**
 * Reset the counters in the fault Records
 * Note that calling this method does not impact the current configuration
 * in any way (including the number of times a fault is to be skipped
 * before it should fail).
 */
void Manager::ResetFaultCounters()
{
    Identifier id = 0;

    Lock();

    for (id = 0; id < mNumFaults; id++)
    {
        mFaultRecords[id].mNumTimesChecked = 0;
    }

    Unlock();
}

/**
 * Reset the configuration of a fault Record
 *
 * @param[in] inId        The fault ID
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::ResetFaultConfigurations(Identifier inId)
{
    Callback * cb;
    int32_t err = 0;

    nlEXPECT_ACTION((inId < mNumFaults), exit, err = -EINVAL);

    Lock();

    mFaultRecords[inId].mNumCallsToSkip = 0;
    mFaultRecords[inId].mNumCallsToFail = 0;
    mFaultRecords[inId].mPercentage     = 0;
    mFaultRecords[inId].mReboot         = 0;
    mFaultRecords[inId].mNumArguments   = 0;

    cb = mFaultRecords[inId].mCallbackList;
    // All callback handling code in this module is based on the assumption
    // that custom callbacks are inserted at the beginning of the list
    while (cb != sEndOfCustomCallbacks && cb != nullptr)
    {
        (void) RemoveCallbackAtFault(inId, cb, kMutexDoNotTake);
        cb = mFaultRecords[inId].mCallbackList;
    }

    Unlock();

exit:
    return err;
}

/**
 * Reset the configuration of all fault Records
 *
 * @return      -EINVAL if the inputs are not valid.
 *              0 otherwise.
 */
int32_t Manager::ResetFaultConfigurations()
{
    int32_t err   = 0;
    Identifier id = 0;

    for (id = 0; id < mNumFaults; id++)
    {
        err = ResetFaultConfigurations(id);
        nlEXPECT(err == 0, exit);
    }

exit:
    return err;
}

/**
 * Take the Manager's mutex.
 */
void Manager::Lock()
{
    if (mLock)
    {
        mLock(mLockContext);
    }
}

/**
 * Release the Manager's mutex.
 */
void Manager::Unlock()
{
    if (mUnlock)
    {
        mUnlock(mLockContext);
    }
}

/**
 * Configure the instance of GlobalContext to use.
 * On systems in which faults are configured and injected from different threads,
 * this function should be called before threads are started.
 *
 * @param[in] inGlobalContext   Pointer to the GlobalContext provided by the application
 */
void SetGlobalContext(GlobalContext * inGlobalContext)
{
    sGlobalContext = inGlobalContext;
}

/**
 * Parse an integer
 *
 * This implementation does not check for ERANGE, as it assumes a very simple
 * underlying implementation of strtol.
 *
 * @param[in]  str      Pointer to a string representing an integer
 *
 * @param[out] num      Pointer to the integer result
 *
 * @return              true in case of success; false if the string does not
 *                      contain an integer.
 */
static bool ParseInt(const char * str, int32_t * num)
{
    char * endptr = nullptr;
    long tmp;
    bool retval = true;

    tmp = strtol(str, &endptr, 10);
    if (!endptr || *endptr != '\0')
    {
        retval = false;
    }
    else
    {
        *num = static_cast<int32_t>(tmp);
    }

    return retval;
}

/**
 * Parse an unsigned integer
 *
 * @param[in]  str      Pointer to a string representing an insigned int
 *
 * @param[out] num      Pointer to the unsigned integer result
 *
 * @return              true in case of success; false if the string does not
 *                      contain an unsigned integer.
 */
static bool ParseUInt(const char * str, uint32_t * num)
{
    bool retval    = true;
    int32_t tmpint = 0;

    retval = ParseInt(str, &tmpint);
    if (retval)
    {
        if (tmpint < 0)
        {
            retval = false;
        }
        else
        {
            *num = static_cast<uint32_t>(tmpint);
        }
    }

    return retval;
}

/**
 * Parse a fault-injection configuration string and apply the configuration.
 *
 * @param[in]   aFaultInjectionStr  The configuration string. An example of a valid string that
 *                                  enables two faults is "system_buffer_f5_s1:inet_send_p33"
 *                                  An example of a configuration string that
 *                                  also passes three integer arguments to the fault point is
 *                                  "system_buffer_f5_s1_a10_a7_a-4"
 *                                  The format is
 *                                  "<module>_<fault>_{f<numTimesToFail>[_s<numTimesToSkip>],p<randomFailurePercentage>}[_a<integer>]..."
 *
 * @param[in]   inArray             An array of GetManagerFn callbacks
 *                                  to be used to parse the string.
 *
 * @param[in]   inArraySize         Num of elements in inArray
 *
 * @return      true  if the string can be parsed completely; false otherwise
 */
bool ParseFaultInjectionStr(char * aFaultInjectionStr, const GetManagerFn * inArray, size_t inArraySize)
{
    ManagerTable table = { inArray, inArraySize };
    size_t numTables   = 1;

    return ParseFaultInjectionStr(aFaultInjectionStr, &table, numTables);
}

/**
 * Parse a fault-injection configuration string and apply the configuration.
 *
 * @param[in]   aFaultInjectionStr  The configuration string. An example of a valid string that
 *                                  enables two faults is "system_buffer_f5_s1:inet_send_p33"
 *                                  An example of a configuration string that
 *                                  also passes three integer arguments to the fault point is
 *                                  "system_buffer_f5_s1_a10_a7_a-4"
 *                                  The format is
 *                                  "<module>_<fault>_{f<numTimesToFail>[_s<numTimesToSkip>],p<randomFailurePercentage>}[_a<integer>]..."
 *
 * @param[in]   inTables            An array of ManagerTable structures
 *                                  to be used to parse the string.
 *
 * @param[in]   inNumTables         Size of inTables
 *
 * @return      true  if the string can be parsed completely; false otherwise
 */
bool ParseFaultInjectionStr(char * aFaultInjectionStr, const ManagerTable * inTables, size_t inNumTables)
{
    char * tok1                      = nullptr;
    char * savePtr1                  = nullptr;
    char * tok2                      = nullptr;
    char * savePtr2                  = nullptr;
    char * outerString               = aFaultInjectionStr;
    size_t i                         = 0;
    nl::FaultInjection::Identifier j = 0;
    int err                          = 0;
    bool retval                      = false;
    int32_t args[kMaxFaultArgs];
    uint16_t numArgs = 0;

    nl::FaultInjection::Manager * mgr      = nullptr;
    nl::FaultInjection::Identifier faultId = 0;

    memset(args, 0, sizeof(args));

    while ((tok1 = strtok_r(outerString, ":", &savePtr1)))
    {
        uint32_t numTimesToFail = 0;
        uint32_t numTimesToSkip = 0;
        uint32_t percentage     = 0;
        bool gotPercentage      = false;
        bool gotReboot          = false;
        bool gotArguments       = false;
        const Name * faultNames = nullptr;

        outerString = nullptr;

        tok2 = strtok_r(tok1, "_", &savePtr2);
        nlEXPECT(tok2 != nullptr, exit);

        // this is the module
        for (i = 0; i < inNumTables; i++)
        {
            for (j = 0; j < inTables[i].mNumItems; j++)
            {
                nl::FaultInjection::Manager & tmpMgr = inTables[i].mArray[j]();
                if (!strcmp(tok2, tmpMgr.GetName()))
                {
                    mgr = &tmpMgr;
                    break;
                }
            }
        }
        nlEXPECT(mgr != nullptr, exit);

        tok2 = strtok_r(nullptr, "_", &savePtr2);
        nlEXPECT(tok2 != nullptr, exit);

        // this is the fault name
        faultNames = mgr->GetFaultNames();
        for (j = 0; j < mgr->GetNumFaults(); j++)
        {
            if (!strcmp(tok2, faultNames[j]))
            {
                faultId = j;
                break;
            }
        }

        nlEXPECT(j != mgr->GetNumFaults(), exit);

        while ((tok2 = strtok_r(nullptr, "_", &savePtr2)))
        {
            switch (tok2[0])
            {
            case 'a': {
                int32_t tmp = 0;
                nlEXPECT(numArgs < kMaxFaultArgs, exit);

                gotArguments = true;

                nlEXPECT(ParseInt(&(tok2[1]), &tmp), exit);
                args[numArgs++] = tmp;
            }
            break;
            case 'f':
                nlEXPECT(ParseUInt(&(tok2[1]), &numTimesToFail), exit);
                break;
            case 's':
                nlEXPECT(ParseUInt(&(tok2[1]), &numTimesToSkip), exit);
                break;
            case 'p':
                gotPercentage = true;
                nlEXPECT(ParseUInt(&(tok2[1]), &percentage), exit);
                nlEXPECT(percentage <= 100, exit);
                break;
            case 'r':
                gotReboot = true;
                break;
            default:
                goto exit;
                break;
            }
        }

        if (gotArguments)
        {
            err = mgr->StoreArgsAtFault(faultId, numArgs, args);
            nlEXPECT_SUCCESS(err, exit);
        }

        if (gotPercentage)
        {
            err = mgr->FailRandomlyAtFault(faultId, static_cast<uint8_t>(percentage));
            nlEXPECT_SUCCESS(err, exit);
        }
        else
        {
            err = mgr->FailAtFault(faultId, numTimesToSkip, numTimesToFail);
            nlEXPECT_SUCCESS(err, exit);
        }
        if (gotReboot)
        {
            err = mgr->RebootAtFault(faultId);
            nlEXPECT_SUCCESS(err, exit);
        }
    }

    retval = true;

exit:
    return retval;
}

/**
 * Internal function to kill the process if a
 * fault is supposed to reboot the process but the application
 * has not installed a callback
 */
static void Die()
{
#if defined(__GNUC__) && (__GNUC__ >= 12)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
    while (true)
        *((volatile long *) 1) = 0;
#if defined(__GNUC__) && (__GNUC__ >= 12)
#pragma GCC diagnostic pop
#endif
}

} // namespace FaultInjection

} // namespace nl
