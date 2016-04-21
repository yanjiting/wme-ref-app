// D:\src\whitney_wme_sprint16\src\common\MediaSDK\wme\testbed\windows\EngineDemo\src\DumppingDataFlagDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EngineDemo.h"
#include "DumppingDataFlagDlg.h"
#include "WmeDataDump.h"

using namespace wme;
// CDumppingDataFlagDlg dialog

IMPLEMENT_DYNAMIC(CDumppingDataFlagDlg, CDialog)

CDumppingDataFlagDlg::CDumppingDataFlagDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDumppingDataFlagDlg::IDD, pParent)
{
	m_uDumpFlag = 0;
}

CDumppingDataFlagDlg::~CDumppingDataFlagDlg()
{
}

void CDumppingDataFlagDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDumppingDataFlagDlg, CDialog)
	ON_BN_CLICKED(IDOK, &CDumppingDataFlagDlg::OnBnClickedOk)
END_MESSAGE_MAP()

unsigned int CDumppingDataFlagDlg::GetDumpFlag()
{
	return m_uDumpFlag;
}
void CDumppingDataFlagDlg::SetDumpFlag(unsigned int uDumpFlag)
{
	m_uDumpFlag = uDumpFlag;
}

BOOL CDumppingDataFlagDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	if(m_uDumpFlag & WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL)
	{
		((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_LISTEN_CHANNEL))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_LISTEN_CHANNEL))->SetCheck(0);
	}

	if(m_uDumpFlag & WME_DATA_DUMP_VIDEO_RAW_CAPTURE)
	{
		((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_CAPTURE))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_CAPTURE))->SetCheck(0);
	}


	if(m_uDumpFlag & WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER)
	{
		((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_AFTER_DECODE_TO_RENDER))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_AFTER_DECODE_TO_RENDER))->SetCheck(0);
	}


	if(m_uDumpFlag & WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER)
	{
		((CButton*)GetDlgItem(IDC_CHECK_ENCODE_RTP_LAYER))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_ENCODE_RTP_LAYER))->SetCheck(0);
	}


	if(m_uDumpFlag & WME_DATA_DUMP_VIDEO_NAL_TO_DECODER)
	{
		((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_DECODER))->SetCheck(1);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_DECODER))->SetCheck(0);
	}

	return TRUE;
}
// CDumppingDataFlagDlg message handlers

void CDumppingDataFlagDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	m_uDumpFlag = 0;

	if(((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_DECODER))->GetCheck() == 1)
	{
		m_uDumpFlag = (m_uDumpFlag |  WME_DATA_DUMP_VIDEO_NAL_TO_DECODER);
	}


	if(((CButton*)GetDlgItem(IDC_CHECK_ENCODE_RTP_LAYER))->GetCheck() == 1)
	{
		m_uDumpFlag = (m_uDumpFlag |  WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER);
	}


	if(((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_AFTER_DECODE_TO_RENDER))->GetCheck() == 1)
	{
		m_uDumpFlag = (m_uDumpFlag | WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER);
	}


	if(((CButton*)GetDlgItem(IDC_CHECK_VIDEO_RAW_CAPTURE))->GetCheck() == 1)
	{
		m_uDumpFlag = (m_uDumpFlag | WME_DATA_DUMP_VIDEO_RAW_CAPTURE);
	}


	if(((CButton*)GetDlgItem(IDC_CHECK_NAL_TO_LISTEN_CHANNEL))->GetCheck() == 1)
	{
		m_uDumpFlag = (m_uDumpFlag | WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL);
	}

	OnOK();
}
