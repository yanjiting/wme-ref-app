

#ifndef  __MOCK_CMICE_IMPL_H__
#define  __MOCK_CMICE_IMPL_H__

#include <gmock/gmock.h>
#include "CmICEInterface.h"



class MockICmICESink : public ICmICESink
{
public:
    MOCK_METHOD1(OnServerCreation, void(ICmICETransport* aTransport));

};



#endif


