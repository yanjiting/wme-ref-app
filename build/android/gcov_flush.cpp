#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

extern "C" void __gcov_flush(void);

class CGcovProcess{
    bool m_quit;
    pthread_t m_pid;

protected:
    static void *cc_start_routine(void *params) {
        CGcovProcess *thiz = (CGcovProcess *)params;
        if(thiz) thiz->Run();
        return NULL;
    }

    void Run() {
        while(!m_quit) {
            __gcov_flush();     
            //std::cout <<"123\n";
            usleep(200*1000);
        }
    }

public:
    CGcovProcess() {
        setenv("GCOV_PREFIX", "/sdcard/wme_gcov/", 1);
        setenv("GCOV_PREFIX_STRIP", "6", 1);
        m_quit = false;
        int ret = pthread_create(&m_pid, NULL, cc_start_routine, this);
    }
    virtual ~CGcovProcess() {
        m_quit = true;
        pthread_join(m_pid, NULL);
        sleep(1);
    }
};
static CGcovProcess s_cc;

//#define __TEST
#ifdef __TEST
int main()
{
    getchar();
    return 0;
}
#endif

