
// MFCserialportDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "MFCserialport.h"
#include "MFCserialportDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <string>
#include <list>


using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define stMotor {0xFB, 0x53 ,0x03 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xFF}

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMFCserialportDlg 대화 상자



CMFCserialportDlg::CMFCserialportDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSERIALPORT_DIALOG, pParent)
	, m_str_comport(_T(" "))
	, m_str_baudrate(_T(" "))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCserialportDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COMPORT, m_combo_comport_list);
	DDX_CBString(pDX, IDC_COMBO_COMPORT, m_str_comport);
	DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_combo_baudrate_list);
	DDX_CBString(pDX, IDC_COMBO_BAUDRATE, m_str_baudrate);
	DDX_Control(pDX, IDC_EDIT_VIEW, m_edit_view);
	//DDX_Control(pDX, IDC_EDIT_SEND_DATA, m_edit_send_data);
}

BEGIN_MESSAGE_MAP(CMFCserialportDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_MESSAGE(WM_MYCLOSE, &CMFCserialportDlg::OnThreadCLosed)
	ON_MESSAGE(WM_MYRECEIVE, &CMFCserialportDlg::OnReceive)
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CMFCserialportDlg::OnBnClickedButtonConnect)
	ON_CBN_SELCHANGE(IDC_COMBO_COMPORT, &CMFCserialportDlg::OnCbnSelchangeComboComport)
	ON_CBN_SELCHANGE(IDC_COMBO_BAUDRATE, &CMFCserialportDlg::OnCbnSelchangeComboBaudrate)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CMFCserialportDlg::OnBnClickedButtonStop)
	ON_CBN_DROPDOWN(IDC_COMBO_COMPORT, &CMFCserialportDlg::OnCbnDropdownComboComport)
	ON_BN_CLICKED(IDC_BUTTON_FOWARD, &CMFCserialportDlg::OnBnClickedButtonFoward)
	ON_BN_CLICKED(IDC_BUTTON_BACKWARD, &CMFCserialportDlg::OnBnClickedButtonBackward)
	ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CMFCserialportDlg::OnBnClickedButtonPause)
	ON_BN_CLICKED(IDC_BUTTON_UP, &CMFCserialportDlg::OnBnClickedButtonUp)
	ON_BN_CLICKED(IDC_BUTTON_DOWN, &CMFCserialportDlg::OnBnClickedButtonDown)
END_MESSAGE_MAP()


// CMFCserialportDlg 메시지 처리기

BOOL CMFCserialportDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	m_combo_baudrate_list.AddString(_T("9600"));
	m_combo_baudrate_list.AddString(_T("19200"));

	comport_state = false;
	GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowTextW(_T("OPEN"));

	m_str_baudrate = _T("19200");
	UpdateData(FALSE);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CMFCserialportDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CMFCserialportDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CMFCserialportDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMFCserialportDlg::OnThreadCLosed(WPARAM length, LPARAM lpara) {
	((CMycomm*)lpara)->HandleClose();
	delete((CMycomm*)lpara);

	return 0;
}

LRESULT CMFCserialportDlg::OnReceive(WPARAM length, LPARAM lpara) {

	// mfc는 CString을 기본으로 하기 때문에 CString -> 16진수로 변환하여함
	CString ConvertToHex(CString data);
	CString str;
	unsigned char data[MAXBUF * 2];
	list<unsigned char> datalist;
	unsigned char cpy[MAXBUF * 2];
	if (m_comm) {
		m_comm->Receive(data, length);

		data[length] = _T('\0');
		str += _T("");
		for (int i = 0; i < length; i++) {
			str += data[i];
			datalist.push_back(data[i]);
			//cpy[i] = data[i];
		}
//		fb 44 18 
		str = ConvertToHex(str);						// str을 Hex값으로 변환.
		if (1) {
			OutputDebugString(str);
			m_edit_view.ReplaceSel(str);		
			m_edit_view.LineScroll(m_edit_view.GetLineCount());
		}
	}
}

void CMFCserialportDlg::OnBnClickedButtonConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (comport_state)
	{
		if (m_comm) {
			m_comm->Close();
			m_comm = NULL;
			AfxMessageBox(_T("COM 포트닫힘"));
			comport_state = false;
			GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText(_T("OPEN"));
			//GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(false);
		}
	}
	else {
		m_comm = new CMycomm(_T("\\\\.\\") + m_str_comport, m_str_baudrate, _T("None"), _T("8 Bit"), _T("1 Bit"));		//init serial Info
		if (m_comm->Create(GetSafeHwnd()) != 0)//통신포트를 열고  윈도우의 핸들을 넘김
		{
			AfxMessageBox(_T("COM 포트 열림"));
			comport_state = true;
			GetDlgItem(IDC_BUTTON_CONNECT)->SetWindowText(_T("CLOSE"));
			//GetDlgItem(IDC_BUTTON_SEND)->EnableWindow(true);
		}
		else {
			AfxMessageBox(_T("ERROR!"));
		}
	}
}


void CMFCserialportDlg::OnCbnSelchangeComboComport()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
}


void CMFCserialportDlg::OnCbnSelchangeComboBaudrate()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	UpdateData();
}


void CMFCserialportDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte stop[] = { 0xFB,0x53,0x01,0x62,0x00,0x0A,0x00,0x0E,0xFF };								//test bytes
	CheckSum(stop);
	m_comm->Send(stop, sizeof(stop) / sizeof(stop[0]));											//send bytes
}

// 
void CMFCserialportDlg::OnCbnDropdownComboComport()													//openComport auto
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	HKEY hKey;

	RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), &hKey);

	TCHAR szData[20], szName[100];
	DWORD index = 0, dwSize = 100, dwSize2 = 20, dwType = REG_SZ;
	m_combo_comport_list.ResetContent();
	memset(szData, 0x00, sizeof(szData));
	memset(szName, 0x00, sizeof(szName));

	while (ERROR_SUCCESS == RegEnumValue(hKey, index, szName, &dwSize, NULL, NULL, NULL, NULL)) {
		index++;
		RegQueryValueEx(hKey, szName, NULL, &dwType, (LPBYTE)szData, &dwSize2);
		m_combo_comport_list.AddString(CString(szData));

		memset(szData, 0x00, sizeof(szData));
		memset(szName, 0x00, sizeof(szName));
		dwSize = 100;
		dwSize2 = 20;
	}

	RegCloseKey(hKey);
}


void CMFCserialportDlg::OnBnClickedButtonFoward()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte foward[] = { 0xFB,0x53,0x01,0x62,0x00,0xA,0x00,0x0D,0xFF };
	CheckSum(foward);
	m_comm->Send(foward, sizeof(foward) / sizeof(foward[0]));
}


void CMFCserialportDlg::OnBnClickedButtonBackward()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte backward[] = { 0xFB,0x53,0x01,0x62,0x00,0xA,0x00,0x0B,0xFF };
	CheckSum(backward);
	m_comm->Send(backward, sizeof(backward) / sizeof(backward[0]));
}


void CMFCserialportDlg::OnBnClickedButtonPause()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte backward[] = { 0xFB,0x53,0x01,0x62,0x00,0x00,0x00,0x0E,0xFF };
	CheckSum(backward);
	m_comm->Send(backward, sizeof(backward) / sizeof(backward[0]));
}


void CMFCserialportDlg::OnBnClickedButtonUp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte backward[] = { 0xFB,0x53,0xA1,0x64,0x00,0x00,0x00,0x05,0xFF };
	CheckSum(backward);
	m_comm->Send(backward, sizeof(backward) / sizeof(backward[0]));
}


void CMFCserialportDlg::OnBnClickedButtonDown()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	byte backward[] = { 0xFB,0x53,0xA1,0x64,0x00,0x00,0x00,0x09,0xFF };
	CheckSum(backward);

	m_comm->Send(backward, sizeof(backward) / sizeof(backward[0]));
}



void CMFCserialportDlg::CheckSum(byte* data)
{
	// TODO: 여기에 구현 코드 추가.
	unsigned int check_sum = 0;
	for (int i = 0; i < 8; i++) {
		check_sum += data[i];
	}
	data[8] = check_sum;
}

/* CString 을 Hex값으로 변환 함수 */
CString ConvertToHex(CString data)
{
	CString returnvalue;

	for (int x = 0; x < data.GetLength(); x++) {
		CString temporary;
		int value = (int)(data[x]);
		temporary.Format(_T("%02X "), value);
		returnvalue += temporary;
	}
	return returnvalue;
}