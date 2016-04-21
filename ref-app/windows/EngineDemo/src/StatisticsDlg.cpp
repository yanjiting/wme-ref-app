// ..\src\StatisticsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EngineDemo.h"
#include "StatisticsDlg.h"
#include "DemoParameters.h"

// CStatisticsDlg dialog

IMPLEMENT_DYNAMIC(CStatisticsDlg, CDialog)

CStatisticsDlg::CStatisticsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStatisticsDlg::IDD, pParent)
{

}

CStatisticsDlg::~CStatisticsDlg()
{
}

void CStatisticsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CStatisticsDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CStatisticsDlg message handlers
void CStatisticsDlg::SetDefaultValue()
{
	CString strDefault = _T("-");
#if 0
	GetDlgItem(IDC_EDIT_OUTWARD_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_OUTWARD_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_OUTWARD_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_OUTWARD_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_OUTWARD_BYTES)->SetWindowTextW(strDefault);

	GetDlgItem(IDC_EDIT_INWARD_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_INWARD_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_INWARD_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_INWARD_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_INWARD_BYTES)->SetWindowTextW(strDefault);
#endif

	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BYTES)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_WIDTH)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_HEIGHT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_FPS)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BITRATE)->SetWindowTextW(strDefault);

	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BYTES)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_WIDTH)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_HEIGHT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_FPS)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BITRATE)->SetWindowTextW(strDefault);


	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BYTES)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BITRATE)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_FEC_ENABLED)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(strDefault);

	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_LOSSRATIO)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_RTT)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_JITTER)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKET_NUM)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BYTES)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BITRATE)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_FEC_ENABLED)->SetWindowTextW(strDefault);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(strDefault);
}

BOOL CStatisticsDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetDefaultValue();

	return TRUE;
}

void CStatisticsDlg::UpdateGeneralNetworkStatistics(WmeSessionStatistics *pSessionStatistics)
{
	CString strValue = _T("-");
#if 0
	if(!pSessionStatistics)
	{
		GetDlgItem(IDC_EDIT_OUTWARD_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_OUTWARD_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_OUTWARD_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_OUTWARD_PACKET_NUM)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_OUTWARD_BYTES)->SetWindowTextW(strValue);

		GetDlgItem(IDC_EDIT_INWARD_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_INWARD_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_INWARD_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_INWARD_PACKET_NUM)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_INWARD_BYTES)->SetWindowTextW(strValue);
		return;	
	}

	strValue.Format(_T("%u"), static_cast<unsigned int>(pSessionStatistics->stOutNetworkStat.fLossRatio*100));
	GetDlgItem(IDC_EDIT_OUTWARD_LOSSRATIO)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stOutNetworkStat.uJitter);
	GetDlgItem(IDC_EDIT_OUTWARD_JITTER)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stOutNetworkStat.uRoundTripTime);
	GetDlgItem(IDC_EDIT_OUTWARD_RTT)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stOutNetworkStat.uPackets);
	GetDlgItem(IDC_EDIT_OUTWARD_PACKET_NUM)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stOutNetworkStat.uBytes);
	GetDlgItem(IDC_EDIT_OUTWARD_BYTES)->SetWindowTextW(strValue);


	strValue.Format(_T("%u"), static_cast<unsigned int>(pSessionStatistics->stInNetworkStat.fLossRatio*100));
	GetDlgItem(IDC_EDIT_INWARD_LOSSRATIO)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stInNetworkStat.uJitter);
	GetDlgItem(IDC_EDIT_INWARD_JITTER)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stInNetworkStat.uRoundTripTime);
	GetDlgItem(IDC_EDIT_INWARD_RTT)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stInNetworkStat.uPackets);
	GetDlgItem(IDC_EDIT_INWARD_PACKET_NUM)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pSessionStatistics->stInNetworkStat.uBytes);
	GetDlgItem(IDC_EDIT_INWARD_BYTES)->SetWindowTextW(strValue);
#endif
}

void CStatisticsDlg::UpdateLocalVideoStatistics(WmeVideoStatistics *pVideoStatistics, WmeNetworkStatistics *pNetworkStatistics)
{
	CString strValue = _T("-");

	if(!pVideoStatistics)
	{
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_WIDTH)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_HEIGHT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_FPS)->SetWindowTextW(strValue);	
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BITRATE)->SetWindowTextW(strValue);	
	}
	else
	{
		strValue.Format(_T("%u"), pVideoStatistics->uWidth);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_WIDTH)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pVideoStatistics->uHeight);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_HEIGHT)->SetWindowTextW(strValue);

		strValue.Format(_T("%.02f"), pVideoStatistics->fFrameRate);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_FPS)->SetWindowTextW(strValue);	

		strValue.Format(_T("%.02f"), pVideoStatistics->fBitRate/1000);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BITRATE)->SetWindowTextW(strValue);	
	}

	strValue = _T("-");
	if(!pNetworkStatistics)
	{
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BYTES)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_PACKET_NUM)->SetWindowTextW(strValue);
	}
	else
	{
		strValue.Format(_T("%u"), static_cast<unsigned int>(pNetworkStatistics->fLossRatio*100));
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_LOSSRATIO)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uRoundTripTime);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_RTT)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uJitter);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_JITTER)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uBytes);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BYTES)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
		GetDlgItem(IDC_EDIT_LOCAL_VIDEO_PACKET_NUM)->SetWindowTextW(strValue);
	}

#if 0
	strValue.Format(_T("%u"), pVideoStatistics->uWidth);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_WIDTH)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pVideoStatistics->uHeight);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_HEIGHT)->SetWindowTextW(strValue);

	strValue.Format(_T("%.02f"), pVideoStatistics->fFrameRate);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_FPS)->SetWindowTextW(strValue);

	strValue.Format(_T("%.02f"), pVideoStatistics->fBitRate);
	GetDlgItem(IDC_EDIT_LOCAL_VIDEO_BITRATE)->SetWindowTextW(strValue);
#endif
}

void CStatisticsDlg::UpdateRemoteVideoStatistics(WmeVideoStatistics *pVideoStatistics, WmeNetworkStatistics *pNetworkStatistics)
{
	CString strValue = _T("-");

	if(!pVideoStatistics)
	{
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_WIDTH)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_HEIGHT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_FPS)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BITRATE)->SetWindowTextW(strValue);
	}
	else
	{
		strValue.Format(_T("%u"), pVideoStatistics->uWidth);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_WIDTH)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pVideoStatistics->uHeight);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_HEIGHT)->SetWindowTextW(strValue);

		strValue.Format(_T("%.02f"), pVideoStatistics->fFrameRate);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_FPS)->SetWindowTextW(strValue);

		strValue.Format(_T("%.02f"), pVideoStatistics->fBitRate/1000);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BITRATE)->SetWindowTextW(strValue);
	}

	strValue = _T("-");
	if(!pNetworkStatistics)
	{
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_PACKET_NUM)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BYTES)->SetWindowTextW(strValue);
	}
	else
	{
		strValue.Format(_T("%u"), static_cast<unsigned int>(pNetworkStatistics->fLossRatio*100));
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_LOSSRATIO)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uRoundTripTime);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_RTT)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uJitter);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_JITTER)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uBytes);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BYTES)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
		GetDlgItem(IDC_EDIT_REMOTE_VIDEO_PACKET_NUM)->SetWindowTextW(strValue);
	}




#if 0
	strValue.Format(_T("%u"), pVideoStatistics->uWidth);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_WIDTH)->SetWindowTextW(strValue);

	strValue.Format(_T("%u"), pVideoStatistics->uHeight);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_HEIGHT)->SetWindowTextW(strValue);

	strValue.Format(_T("%.02f"), pVideoStatistics->fFrameRate);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_FPS)->SetWindowTextW(strValue);

	strValue.Format(_T("%.02f"), pVideoStatistics->fBitRate);
	GetDlgItem(IDC_EDIT_REMOTE_VIDEO_BITRATE)->SetWindowTextW(strValue);
#endif
}

void CStatisticsDlg::UpdateLocalAudioStatistics(WmeAudioStatistics *pAudioStatistics, WmeNetworkStatistics *pNetworkStatistics)
{
	CString strValue = _T("-");

	if(!pAudioStatistics)
	{
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_FEC_ENABLED)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BITRATE)->SetWindowTextW(strValue);

	}
	else
	{
		strValue = _T("N");
		if(pAudioStatistics->bEnableCodecFEC)
		{
			strValue = _T("Y");
		}
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_FEC_ENABLED)->SetWindowTextW(strValue);

		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(_T("-"));

		strValue.Format(_T("%.02f"), (pAudioStatistics->uBitRate*1.0)/1000);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BITRATE)->SetWindowTextW(strValue);

	}

	strValue = _T("-");
	if(!pNetworkStatistics)
	{
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKET_NUM)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BYTES)->SetWindowTextW(strValue);
	}
	else
	{
		strValue.Format(_T("%u"), static_cast<unsigned int>(pNetworkStatistics->fLossRatio*100));
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_LOSSRATIO)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uRoundTripTime);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_RTT)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uJitter);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_JITTER)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uBytes);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BYTES)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
		GetDlgItem(IDC_EDIT_LOCAL_AUDIO_PACKET_NUM)->SetWindowTextW(strValue);
	}



#if 0
	strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
	GetDlgItem(IDC_EDIT_LOCAL_AUDIO_BITRATE)->SetWindowTextW(strValue);
#endif

}

void CStatisticsDlg::UpdateRemoteAudioStatistics(WmeAudioStatistics *pAudioStatistics, WmeNetworkStatistics *pNetworkStatistics)
{
	CString strValue = _T("-");

	if(!pAudioStatistics)
	{
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_FEC_ENABLED)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BITRATE)->SetWindowTextW(strValue);
	}
	else
	{
		strValue = _T("N");
		if(pAudioStatistics->bEnableCodecFEC)
		{
			strValue = _T("Y");
		}
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_FEC_ENABLED)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), static_cast<unsigned int>(pAudioStatistics->uCodecFECRecoveredPackets));
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKETS_RECOVED_BY_FEC)->SetWindowTextW(strValue);

		strValue.Format(_T("%.02f"), (pAudioStatistics->uBitRate*1.0)/1000);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BITRATE)->SetWindowTextW(strValue);
	}

	strValue = _T("-");
	if(!pNetworkStatistics)
	{
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_LOSSRATIO)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_RTT)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_JITTER)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKET_NUM)->SetWindowTextW(strValue);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BYTES)->SetWindowTextW(strValue);
	}
	else
	{
		strValue.Format(_T("%u"), static_cast<unsigned int>(pNetworkStatistics->fLossRatio*100));
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_LOSSRATIO)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uRoundTripTime);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_RTT)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uJitter);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_JITTER)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uBytes);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BYTES)->SetWindowTextW(strValue);

		strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
		GetDlgItem(IDC_EDIT_REMOTE_AUDIO_PACKET_NUM)->SetWindowTextW(strValue);
	}



#if 0
	strValue.Format(_T("%u"), pNetworkStatistics->uPackets);
	GetDlgItem(IDC_EDIT_REMOTE_AUDIO_BITRATE)->SetWindowTextW(strValue);
#endif
}

void CStatisticsDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: Add your message handler code here

}

void CStatisticsDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	if(m_pParentWnd)
	{
		m_pParentWnd->SendMessage(WM_MESSAGE_STATISTICS_WINDOW_CLOSE, 0, 0);
	}
	CDialog::OnClose();
}
