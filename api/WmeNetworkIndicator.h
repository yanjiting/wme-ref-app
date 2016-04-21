#ifndef WME_INTERFACE_NETWORK_INDICATOR
#define WME_INTERFACE_NETWORK_INDICATOR

#include "WmeCommonTypes.h"
#include "WmeUnknown.h"

namespace wme
{

///Network quality index
typedef enum _tagNetworkIndex
{
    NETWORK_UNKNOWN,
    NETWORK_USELESS,
    NETWORK_SEVERE,
    NETWORK_NOT_ACCEPTABLE,
    NETWORK_POOR,
    NETWORK_GOOD,
    NETWORK_VERY_GOOD,
    NETWORK_EXCELLENT
}WmeNetworkIndex;

///Network direction enumerator
typedef enum _tagNetworkDirection
{
    DIRECTION_UPLINK,       ///< egress of your internet access
    DIRECTION_DOWNLINK,     ///< ingress of your internet access
    DIRECTION_BOTHLINK      ///< both direction
}WmeNetworkDirection;

static const WMEIID WMEIID_IWmeNetworkIndicator =
{ 0x39e18d00, 0x08cc, 0x4582, { 0x9b, 0x3c, 0x7c, 0x5a, 0x9e, 0x9c, 0xa4, 0xdc } };

class IWmeNetworkIndicator : public IWmeUnknown
{
public:
    virtual WmeNetworkIndex GetNetworkIndex(WmeNetworkDirection direction) = 0;

protected:
    virtual ~IWmeNetworkIndicator(){}
};

} //namespace

#endif //WME_NETWORK_INDICATOR_INTERFACE_H
