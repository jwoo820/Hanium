#include "pch.h"
#include "CMycomm.h"

// initial member
CMycomm::CMycomm() {
	m_sComPort = "";
	m_sBaudRate = "";
	m_sDataBit = "";
	m_sStopBit = "";
	m_bFlowChk = false;

}
// init serial setting
CMycomm::CMycomm(CString port, CString baudrate, CString parity, CString databit, CString stopbit) {
	m_sComPort = port;
	m_sBaudRate = baudrate;
	m_sParity = parity;
	m_sDataBit = databit;
	m_sStopBit = stopbit;
	m_bFlowChk = 1;
	m_bIsOpened = FALSE;
	m_nLength = 0;
	memset(m_sInBuf, 0, MAXBUF * 2);
	m_pEvent = new CEvent(FALSE, TRUE);
}
CMycomm::~CMycomm() {
	if (m_bIsOpened) {
		Close();
	}
	delete m_pEvent;
}

// start serial port
void CMycomm::ResetSerial() {
	DCB dcb;
	DWORD DErr;
	COMMTIMEOUTS CommTimeouts;

	if (!m_bIsOpened)
		return;

	ClearCommError(m_hComDev, &DErr, NULL);													//통신에러에 대한 정보를 받거나 통신 장치의 현재 상태를 보고
	SetupComm(m_hComDev, InBufSize, OutBufSize);											//통신장치 파라미터 초기화
	PurgeComm(m_hComDev, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);	//지정된 통신 자원의 입출력 버퍼를 삭제

	CommTimeouts.ReadIntervalTimeout = MAXDWORD;											//지정된 시간 데이터가 수신 되지 않으면 ReadFile 타임아웃
	CommTimeouts.ReadTotalTimeoutMultiplier = MAXWORD;
	CommTimeouts.ReadTotalTimeoutConstant = 3;

	CommTimeouts.WriteTotalTimeoutMultiplier = 0;											// 송신에는 interval 시간이 필요없음
	CommTimeouts.WriteTotalTimeoutConstant = 1000;

	SetCommTimeouts(m_hComDev, &CommTimeouts);

	memset(&dcb, 0, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);

	GetCommState(m_hComDev, &dcb);															//통신 디바이스 연결

	dcb.fBinary = TRUE;
	dcb.fParity = TRUE;

	if (m_sBaudRate == "9600")																//set BuadRate
		dcb.BaudRate = CBR_9600;
	else if (m_sBaudRate == "19200")
		dcb.BaudRate = CBR_19200;

	if (m_sParity == "None")																//set Parity
		dcb.Parity = NOPARITY;
	else if (m_sParity == "Even")
		dcb.Parity = EVENPARITY;
	else if (m_sParity == "Odd")
		dcb.Parity = ODDPARITY;

	if (m_sDataBit == "7 Bit")																//set dataBit
		dcb.ByteSize = 7;
	else if (m_sDataBit == "8 Bit")
		dcb.ByteSize = 8;

	if (m_sStopBit == "1 Bit")																//set stopBit
		dcb.StopBits = ONESTOPBIT;
	else if (m_sStopBit == "1.5 Bit")
		dcb.StopBits = ONE5STOPBITS;
	else if (m_sStopBit == "2 Bit")
		dcb.StopBits = TWOSTOPBITS;

	dcb.fRtsControl = RTS_CONTROL_ENABLE;													//request-t0-send control
	dcb.fDtrControl = DTR_CONTROL_ENABLE;
	dcb.fOutxDsrFlow = FALSE;

	if (m_bFlowChk) {
		dcb.fOutX = FALSE;
		dcb.fInX = FALSE;
		dcb.XonLim = 2048;
		dcb.XoffLim = 1024;
	}
	else {
		dcb.fOutxCtsFlow = TRUE;
		dcb.fRtsControl = RTS_CONTROL_HANDSHAKE;
	}

	SetCommState(m_hComDev, &dcb);															//현재 정보 핸들러로 넘김
	SetCommMask(m_hComDev, EV_RXCHAR);														//문자가 수신되면 입력버퍼에 저장
}

// close serial port
void CMycomm::Close() {
	if (!m_bIsOpened)
		return;

	m_bIsOpened = FALSE;
	SetCommMask(m_hComDev, 0);
	EscapeCommFunction(m_hComDev, CLRDTR);													//확장기능 수행 , ready signal?
	PurgeComm(m_hComDev, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	Sleep(500);
}

UINT CommThread(LPVOID lpData)
{
	extern short	g_nRemoteStatus;
	DWORD			ErrorFlags;
	COMSTAT			ComStat;
	DWORD			EvtMask;
	DWORD			Length;
	char			buf[MAXBUF];
	int				size;
	int				insize;


	CMycomm* Comm = (CMycomm*)lpData;

	while (Comm->m_bIsOpened)
	{
		EvtMask = 0;
		Length = 0;
		insize = 0;
		memset(buf, NULL, MAXBUF);
		WaitCommEvent(Comm->m_hComDev, &EvtMask, NULL);
		ClearCommError(Comm->m_hComDev, &ErrorFlags, &ComStat);
		if ((EvtMask & EV_RXCHAR) && ComStat.cbInQue) {
			if (ComStat.cbInQue > MAXBUF)
				size = MAXBUF;																		//MAXSIZE 초과 방지
			else
				size = ComStat.cbInQue;																//메모리 동적 할당
			do {
				ClearCommError(Comm->m_hComDev, &ErrorFlags, &ComStat);
				if (!ReadFile(Comm->m_hComDev, buf + insize, size, &Length, &(Comm->m_OLR))) {
					TRACE("Error in ReadFile\n");
					if (GetLastError() == ERROR_IO_PENDING) {
						if (WaitForSingleObject(Comm->m_OLR.hEvent, 1000) != WAIT_OBJECT_0)
							Length = 0;
						else
							GetOverlappedResult(Comm->m_hComDev, &(Comm->m_OLR), &Length, FALSE);
					}
					else
						Length = 0;
				}
				insize += Length;
			} while ((Length != 0) && (insize < size));
			ClearCommError(Comm->m_hComDev, &ErrorFlags, &ComStat);

			if (Comm->m_nLength + insize > MAXBUF * 2)										
				insize = (Comm->m_nLength + insize) - MAXBUF * 2;

			Comm->m_pEvent->ResetEvent();
			memcpy(Comm->m_sInBuf + Comm->m_nLength, buf, insize);
			Comm->m_nLength += insize;
			Comm->m_pEvent->SetEvent();
			LPARAM temp = (LPARAM)Comm;
			SendMessage(Comm->m_hWnd, WM_MYRECEIVE, Comm->m_nLength, temp);
		}
	}
	PurgeComm(Comm->m_hComDev, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	LPARAM temp = (LPARAM)Comm;
	SendMessage(Comm->m_hWnd, WM_MYCLOSE, 0, temp);
	return 0;
}
void CMycomm::HandleClose() {
	CloseHandle(m_hComDev);
	CloseHandle(m_OLR.hEvent);
	CloseHandle(m_OLW.hEvent);
}
BOOL CMycomm::Create(HWND hWnd) {

	m_hWnd = hWnd;

	m_hComDev = CreateFile(m_sComPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);

	if (m_hComDev != INVALID_HANDLE_VALUE)
		m_bIsOpened = TRUE;
	else
		return FALSE;

	ResetSerial();

	m_OLW.Offset = 0;
	m_OLW.OffsetHigh = 0;
	m_OLR.Offset = 0;
	m_OLR.OffsetHigh = 0;

	m_OLR.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_OLR.hEvent == NULL) {
		CloseHandle(m_OLR.hEvent);
		return FALSE;
	}
	m_OLW.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_OLW.hEvent == NULL) {
		CloseHandle(m_OLW.hEvent);
		return FALSE;
	}
	AfxBeginThread(CommThread, (LPVOID)this);
	EscapeCommFunction(m_hComDev, SETDTR);
	return TRUE;
}

// send data bytes
BOOL CMycomm::Send(byte* outbuf, int len) {

	BOOL	bRet = TRUE;
	DWORD	ErrorFlags;
	COMSTAT	ComStat;

	DWORD	BytesWritten;
	DWORD	BytesSent = 0;

	ClearCommError(m_hComDev, &ErrorFlags, &ComStat);
	if (!WriteFile(m_hComDev, outbuf, len, &BytesWritten, &m_OLW)) {
		if (GetLastError() == ERROR_IO_PENDING)
		{
			if (WaitForSingleObject(m_OLW.hEvent, 1000) != WAIT_OBJECT_0)
				bRet = FALSE;
			else
				GetOverlappedResult(m_hComDev, &m_OLW, &BytesWritten, FALSE);
		}
		else
			bRet = FALSE;
	}

	ClearCommError(m_hComDev, &ErrorFlags, &ComStat);
	Sleep(1000);				//time delay
	return bRet;
}

// receive data bytes
int  CMycomm::Receive(unsigned char* inbuf, int len) {

	CSingleLock lockObj((CSyncObject*)m_pEvent, FALSE);		//공유데이터 접근 제한

	if (len == 0)
		return -1;
	else if (len > MAXBUF)
		return -1;

	if (m_nLength == 0) {
		inbuf[0] = '\0';
		return 0;
	}

	else if (m_nLength <= len) {
		lockObj.Lock();
		memcpy(inbuf, m_sInBuf, m_nLength);
		memset(m_sInBuf, 0, MAXBUF * 2);
		int tmp = m_nLength;
		m_nLength = 0;
		lockObj.Unlock();
		return tmp;
	}
	else
	{
		lockObj.Lock();
		memcpy(inbuf, m_sInBuf, len);
		memmove(m_sInBuf, m_sInBuf + len, MAXBUF * 2 - len);
		m_nLength -= len;
		lockObj.Unlock();
		return len;
	}
}
void CMycomm::Clear() {
	PurgeComm(m_hComDev, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	memset(m_sInBuf, 0, MAXBUF * 2);
	m_nLength = 0;
}

