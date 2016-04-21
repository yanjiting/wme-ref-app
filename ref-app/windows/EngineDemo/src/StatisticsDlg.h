#pragma once

#include "WmeEngine.h"
using namespace wme;
// CStatisticsDlg dialog

class CStatisticsDlg : public CDialog
{
	DECLARE_DYNAMIC(CStatisticsDlg)

public:
	CStatisticsDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatisticsDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STATISTICS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	void SetDefaultValue();
	void UpdateGeneralNetworkStatistics(WmeSessionStatistics *pSessionStatistics);
	void UpdateLocalVideoStatistics(WmeVideoStatistics *pVideoStatistics, WmeNetworkStatistics *pNetworkStatistics);
	void UpdateRemoteVideoStatistics(WmeVideoStatistics *pVideoStatistics, WmeNetworkStatistics *pNetworkStatistics);
	void UpdateLocalAudioStatistics(WmeAudioStatistics *pAudioStatistics, WmeNetworkStatistics *pNetworkStatistics);
	void UpdateRemoteAudioStatistics(WmeAudioStatistics *pAudioStatistics, WmeNetworkStatistics *pNetworkStatistics);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
};