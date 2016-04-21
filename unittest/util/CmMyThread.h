#include "CmThread.h"

enum {
	STATUS_INIT,
	STATUS_RUN,
	STATUS_STOP
};

class CCmMyThread : public ACmThread
{
public:

	CCmMyThread();
	virtual ~CCmMyThread() {};

	virtual CmResult Stop(CCmTimeValue* aTimeout = NULL);
        virtual void OnThreadRun();

public:
	int get_status(void)
	{
		return m_status;
	}

private:
	BOOL m_isStop;
	int m_status;

};
