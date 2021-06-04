#include <pthread.h>

/** A scoped lock/unlock around a mutex. */
struct StackLockGuard
{
public:
    StackLockGuard(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_lock(mMutex); }
    ~StackLockGuard() { pthread_mutex_unlock(mMutex); }

private:
    pthread_mutex_t * mMutex;
};

/**
 * Use StackUnlockGuard to temporarily unlock the CHIP BLE stack, e.g. when calling application
 * or Android BLE code as a result of a BLE event.
 */
struct StackUnlockGuard
{
public:
    StackUnlockGuard(pthread_mutex_t * mutex) : mMutex(mutex) { pthread_mutex_unlock(mMutex); }
    ~StackUnlockGuard() { pthread_mutex_lock(mMutex); }

private:
    pthread_mutex_t * mMutex;
};
