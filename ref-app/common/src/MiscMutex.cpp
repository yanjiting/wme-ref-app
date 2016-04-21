#include "MiscMutex.h"

MiscMutex::MiscMutex()
{
#ifdef WIN32
//    ::InitializeCriticalSection(&m_Lock);
#elif defined(IOS)
    // ...
#elif defined(MACOS)
    pthread_mutexattr_t mutex_attr;
    ::pthread_mutexattr_init(&mutex_attr);
    ::pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    ::pthread_mutex_init(&m_Lock,&mutex_attr);
    ::pthread_mutexattr_destroy(&mutex_attr);
#else
    pthread_mutexattr_t mutex_attr;
    ::pthread_mutexattr_init(&mutex_attr);
#if defined(LINUX)
    ::pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_RECURSIVE_NP);
#else
    ::pthread_mutexattr_settype(&mutex_attr,PTHREAD_MUTEX_RECURSIVE);
#endif
    ::pthread_mutex_init(&m_Lock, &mutex_attr);
    ::pthread_mutexattr_destroy(&mutex_attr);
#endif // WIN32
}

MiscMutex::~MiscMutex()
{
#ifdef WIN32
 //   ::DeleteCriticalSection(&m_Lock);
#elif defined(IOS)
    // ...
#else
    int nRet = ::pthread_mutex_destroy(&m_Lock);
    if (nRet != 0)  {
    }
#endif // WIN32
}


long MiscMutex::Lock()
{
#ifdef WIN32
//    ::EnterCriticalSection(&m_Lock);
    return 0;
#elif defined(IOS)
    return -1;
#else
    int nRet = ::pthread_mutex_lock(&m_Lock);
    if (nRet == 0)
        return 0;
    else {
        return -1;
    }
#endif // WIN32
}

long MiscMutex::UnLock()
{
#ifdef WIN32
 //   ::LeaveCriticalSection(&m_Lock);
    return 0;
#elif defined(IOS)
    return -1;
#else
    int nRet = ::pthread_mutex_unlock(&m_Lock);
    if (nRet == 0)
        return 0;
    else {
        return -1;
    }
#endif // WIN32
}

long MiscMutex::TryLock()
{
#ifdef WIN32
#if(_WIN32_WINNT >= 0x0400)
    BOOL bRet = ::TryEnterCriticalSection(&m_Lock);
    return bRet ? 0 : -1;
#else
    return -1;
#endif //
#elif defined(IOS)
    return -1;
#else
    int nRet = ::pthread_mutex_trylock(&m_Lock);
    return (nRet == 0) ? 0 : -1;
#endif // WIN32
}

