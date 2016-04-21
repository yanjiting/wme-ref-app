#include "mttimer.h"

#define DEFAULT_DELAY 100  //ms


void CTimerAndroid::Run()
{
    usleep(DEFAULT_DELAY*1000);

    timespec ts;
    long state = m_state;

    vector<TPInfo_t>::iterator msg = NULL;

    while(state != S_QUIT)
    {
        pthread_mutex_lock(&m_mtx);

        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += (m_interval>>10);
        ts.tv_nsec += (m_interval % 1000) << 20;
        pthread_cond_timedwait(&m_cond, &m_mtx, &ts);

        state = m_state;
        if (state == S_QUIT || state == S_WAIT) {
            pthread_mutex_unlock(&m_mtx);
            continue;
        }

        msg = NULL;
        if (!m_msg.empty()) {
            msg = m_msg.begin();
        }
        pthread_mutex_unlock(&m_mtx);

        ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
        if(pMain != NULL)
        {
            pMain->OnThreadRun();
        }
        
        if (msg) 
        {
            if (msg->client) {
                if (msg->host) {
                    msg->client->InitHost(msg->enableCtrl, msg->hostDataPort, msg->connDataType, msg->hostCtrlPort, msg->connCtrlType);
                }else{
                    msg->client->ConnectRemote(msg->hostIP, msg->hostDataPort, msg->connDataType, msg->hostCtrlPort, msg->connCtrlType);
                }
            }
            m_msg.erase(msg);
        }
    }
}

bool CTimerAndroid::SetTimer (unsigned int interval)
{
    pthread_mutex_lock(&m_mtx);
    if (m_state == S_WAIT || m_state == S_RUN) {
        pthread_mutex_unlock(&m_mtx);
        return true;
    }
    m_state = S_WAIT;
    pthread_mutex_unlock(&m_mtx);

    m_tid = -1;
    if (pthread_create(&m_tid, NULL, timer_callback, (void *) this) != 0) {
        m_state = S_INIT;
        return false;
    }

    return true;
}

void CTimerAndroid::KillTimer()
{
    pthread_mutex_lock(&m_mtx);
    m_tid = -1;
    m_state = S_QUIT;
    pthread_mutex_unlock(&m_mtx);
    usleep(1000);
}
