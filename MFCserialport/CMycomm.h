#pragma once
#include <afxwin.h>
#include <queue>
#include <iostream>



#define QSIZE			120
#define MAXBUF			120
#define InBufSize		120
#define OutBufSize		120
#define ASCII_XON		0x11
#define ASCII_OFF		0x13
#define WM_MYRECEIVE	(WM_USER+1)
#define WM_MYCLOSE		(WM_USER+2)
#define F_COMTIME_400	0x00000040
#define PC_HEAD1		0xFB
#define PC_HEAD2		0x53

using namespace std;

class CMycomm :
	public CCmdTarget
{
public:
	CMycomm();
	CMycomm(CString port, CString baudrate, CString parity, CString databit, CString stopbit);
	~CMycomm();
public:
	HANDLE				m_hComDev;
	HWND				m_hWnd;

	BOOL				m_bIsOpened;
	CString				m_sComPort;
	CString				m_sBaudRate;
	CString				m_sParity;
	CString				m_sDataBit;
	CString				m_sStopBit;
	BOOL				m_bFlowChk;

	OVERLAPPED			m_OLR, m_OLW;			//overlapped read, write
	unsigned char		m_sInBuf[MAXBUF * 2];	//maxbuffersize
	int					m_nLength;
	CEvent*				m_pEvent;

public:
	void				Clear();
	int					Receive(unsigned char* Inbuf, int len);
	BOOL				Send(byte* outbuf, int len);
	BOOL				Create(HWND hWnd);
	void				HandleClose();
	void				Close();
	void				ResetSerial();
};

