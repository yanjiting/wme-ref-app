#ifndef _MISCMUTEX_H_
#define _MISCMUTEX_H_

#if defined(WIN32)
#include <winsock2.h>
#include <windows.h>
typedef CRITICAL_SECTION THREAD_MUTEX_T;
typedef DWORD THREAD_ID;
typedef HANDLE THREAD_HANDLE;
#else
#include <pthread.h>
typedef pthread_mutex_t THREAD_MUTEX_T;
typedef pthread_t THREAD_ID;
typedef THREAD_ID THREAD_HANDLE;
#endif

class MiscMutex
{
public:
    MiscMutex();
    virtual ~MiscMutex();
    long Lock();
    long UnLock();
    long TryLock();

    THREAD_MUTEX_T& GetMutexType() { return m_Lock;}

protected:
    THREAD_MUTEX_T m_Lock;
};

class AutoLock
{
public:
    AutoLock(MiscMutex &mtx) : m_Mutex(mtx) {
        m_Mutex.Lock();
    }
    virtual ~AutoLock() {
        m_Mutex.UnLock();
    }

private:
    MiscMutex & m_Mutex;
};

#endif // _MISCMUTEX_H_
