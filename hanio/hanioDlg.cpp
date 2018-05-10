
// hanioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "hanio.h"
#include "hanioDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_GAME_OVER	WM_USER + 100
#define WM_AUTO_GAME	WM_USER + 101

#define TIME_ELAPSE		200

#define					LEN_BEGIN	40
#define					LEN_UNIT	10
#define					LEN_PADDING 0
#define					BTN_HEIGHT	20
#define					SIDE_LENGTH	8

// ChanioDlg dialog




ChanioDlg::ChanioDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(ChanioDlg::IDD, pParent)
	, m_CurCol(0)
	, m_Steps(0)
	, m_StepsMin(0)
	, m_Level(1)
	, m_Intervals(TIME_ELAPSE)
	, m_ActiveHanio(NULL)
	, m_hThread(NULL)
	, m_hEvent(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void ChanioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_COLUMN1, m_Column[0]);
	DDX_Control(pDX, IDC_STATIC_COLUMN2, m_Column[1]);
	DDX_Control(pDX, IDC_STATIC_COLUMN3, m_Column[2]);
}

BEGIN_MESSAGE_MAP(ChanioDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_GAME_OVER, ChanioDlg::OnGameOver)
	ON_MESSAGE(WM_AUTO_GAME, ChanioDlg::OnAutoGame)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// ChanioDlg message handlers

BOOL ChanioDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	config_data_t data;
	ReadIniFile(data);
	SetWindowPos(NULL, 0, 0, data.width, data.height, SWP_NOMOVE);
	m_Level = data.level;
	m_Intervals = data.intervals;

	GetDlgItem(IDC_STATIC)->SetWindowText(_T(""));
	CRect rect;
	GetClientRect(rect);
	for (int i=0; i<COLUMN_COUNT; i++)
	{
		m_Column[i].MoveWindow(rect.Width()/(COLUMN_COUNT+1)*(i+1), rect.Height()/6, 1, rect.Height()/6*4);
	}
	ResetHanio();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

BOOL ChanioDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN)  
	{  
		switch (pMsg->wParam)  
		{
		case VK_UP:
			Up();
			return TRUE;
		case VK_DOWN:
			Down();
			return TRUE;
		case VK_LEFT:
			Left();
			return TRUE;
		case VK_RIGHT:
			Right();
			return TRUE;
		case 'R':
			if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			{
				RestartGame();
			}
			break;
		case 'A':
			if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			{
				AutoGame();
			}
			break;
		case 'S':
			if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			{
				StopGame();
			}
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
			{
				GotoLevel(pMsg->wParam-'0');
			}
			break;
		default:  
			break;  
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void ChanioDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this);
		// Draw Focus
		CBrush* brush = CBrush::FromHandle((HBRUSH)CreateSolidBrush(BLACK_BRUSH));
		dc.SelectObject(brush);
		dc.Polygon(m_Triangle,COLUMN_COUNT);

		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR ChanioDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void ChanioDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	ExitThread();
	CDialogEx::OnClose();
}

LRESULT ChanioDlg::OnGameOver(WPARAM wparam, LPARAM lparam)
{
	MessageBox(_T("Game Over"));
	m_Level++;
	ResetHanio();
	WriteIniFile();
	return 0L;
}

LRESULT ChanioDlg::OnAutoGame(WPARAM wparam, LPARAM lparam)
{
	auto_game_e op = (auto_game_e)wparam;
	switch (op)
	{
	case AUTO_UP:
		Up();
		break;
	case AUTO_DOWN:
		Down();
		break;
	case AUTO_LEFT:
		Left();
		break;
	case AUTO_RIGHT:
		Right();
		break;
	default:
		break;
	}
	return 0L;
}

void ChanioDlg::ReadIniFile(config_data_t& data)
{
	CString exeName;
	AfxGetModuleFileName(AfxGetInstanceHandle(), exeName);
	CString exePath = exeName.Left(exeName.ReverseFind('.')+1);
	CString iniFileName = exePath + _T("ini");

	CRect rect;
	GetClientRect(rect);

	if (PathFileExists(iniFileName))
	{
		data.width = ::GetPrivateProfileInt(_T("windows"), _T("width"), rect.Width(), iniFileName);
		data.height = ::GetPrivateProfileInt(_T("windows"), _T("height"), rect.Height(), iniFileName);
		data.level = ::GetPrivateProfileInt(_T("hanio"), _T("level"), m_Level, iniFileName);
		data.intervals = ::GetPrivateProfileInt(_T("hanio"), _T("intervals"), TIME_ELAPSE, iniFileName);
	}
	else
	{
		// create ini file
		data.width = rect.Width();
		data.height = rect.Height();
		data.level = m_Level;
		data.intervals = TIME_ELAPSE;
		WriteIniFile(&data);
	}
}

void ChanioDlg::WriteIniFile(config_data_t* data)
{
	CString exeName;
	AfxGetModuleFileName(AfxGetInstanceHandle(), exeName);
	CString exePath = exeName.Left(exeName.ReverseFind('.')+1);
	CString iniFileName = exePath + _T("ini");

	CString buffer;
	if (data)
	{	
		buffer.Format(_T("%d"), data->width);
		WritePrivateProfileString(_T("windows"), _T("width"), buffer, iniFileName);
		buffer.Format(_T("%d"), data->height);
		WritePrivateProfileString(_T("windows"), _T("height"), buffer, iniFileName);
		buffer.Format(_T("%d"), data->level);
		WritePrivateProfileString(_T("hanio"), _T("level"), buffer, iniFileName);
		buffer.Format(_T("%d"), data->intervals);
		WritePrivateProfileString(_T("hanio"), _T("intervals"), buffer, iniFileName);
	}
	else
	{
		buffer.Format(_T("%d"), m_Level);
		WritePrivateProfileString(_T("hanio"), _T("level"), buffer, iniFileName);
	}
}

void ChanioDlg::ResetHanio()
{
	// clear
	m_Steps = 0;
	m_CurCol = 0;
	m_StepsMin = (0x00000001 << m_Level) - 1;
	for (int i=0; i<COLUMN_COUNT; i++)
	{
		while (!m_HanioData[i].empty())
		{
			hanio_data_t* data = m_HanioData[i].top();
			if (data->btn)
			{
				data->btn->DestroyWindow();
				delete data->btn;
			}
			m_HanioData[i].pop();
			delete data;
		}
	}
	if (m_ActiveHanio)
	{
		m_ActiveHanio->btn->DestroyWindow();
		delete m_ActiveHanio;
		m_ActiveHanio = NULL;
	}

	// init
	CRect rect;
	m_Column[m_CurCol].GetWindowRect(rect);
	ScreenToClient(&rect);
	CString text;
	for (int i=m_Level-1; i>=0; i--)
	{
		hanio_data_t* data = new hanio_data_t;
		data->index = i;
		CRect rt;
		rt.left = rect.left - (LEN_BEGIN+LEN_UNIT*i)/2;
		rt.top = rect.bottom-(BTN_HEIGHT*(m_Level-i)+LEN_PADDING*(m_Level-i-1));
		rt.right = rect.right+(LEN_BEGIN+LEN_UNIT*i)/2;
		rt.bottom = rect.bottom-((BTN_HEIGHT+LEN_PADDING)*(m_Level-i-1));
		text.Format(_T("%d"), data->index+1);
		data->btn = new CButton();
		data->btn->Create(text, WS_VISIBLE|BS_PUSHBUTTON|WS_DISABLED, rt, this, i+300);
		m_HanioData[0].push(data);
	}

	UpdateTriangle();
	IncreaseSteps();
}

void ChanioDlg::UpdateTriangle()
{
	CRect rect;
	m_Column[m_CurCol].GetWindowRect(rect);
	ScreenToClient(&rect);

	m_Triangle[0].x = rect.left;
	m_Triangle[0].y = rect.bottom+(BTN_HEIGHT+LEN_PADDING)/2;
	m_Triangle[1].x = m_Triangle[0].x-SIDE_LENGTH;
	m_Triangle[1].y = m_Triangle[0].y+SIDE_LENGTH;
	m_Triangle[2].x = m_Triangle[0].x+SIDE_LENGTH;
	m_Triangle[2].y = m_Triangle[0].y+SIDE_LENGTH;
	Invalidate(TRUE);
}

void ChanioDlg::Up()
{
	if (!m_ActiveHanio)
	{
		if (!m_HanioData[m_CurCol].empty())
		{
			m_ActiveHanio = m_HanioData[m_CurCol].top();
			CRect rect;
			m_ActiveHanio->btn->GetWindowRect(rect);
			ScreenToClient(&rect);

			CRect rt;
			m_Column[m_CurCol].GetWindowRect(rt);
			ScreenToClient(&rt);

			rect.OffsetRect(CSize(0, rt.top-rect.top-rect.Height()-LEN_PADDING));
			m_ActiveHanio->btn->MoveWindow(rect);
			m_HanioData[m_CurCol].pop();
			//IncreaseSteps();
		}
	}
}

void ChanioDlg::Down()
{
	if (m_ActiveHanio)
	{
		CRect rect;
		CRect rt;

		if (!m_HanioData[m_CurCol].empty())
		{
			hanio_data_t* data = m_HanioData[m_CurCol].top();
			if (data->index < m_ActiveHanio->index)
				return;

			m_ActiveHanio->btn->GetWindowRect(rect);
			ScreenToClient(&rect);

			data->btn->GetWindowRect(rt);
			ScreenToClient(&rt);

			rect.OffsetRect(CSize(0, rt.top-rect.bottom-LEN_PADDING));
		}
		else
		{
			m_ActiveHanio->btn->GetWindowRect(rect);
			ScreenToClient(&rect);

			CRect rt;
			m_Column[m_CurCol].GetWindowRect(rt);
			ScreenToClient(&rt);

			rect.OffsetRect(CSize(0, rt.bottom-rect.bottom));
		}
		
		m_ActiveHanio->btn->MoveWindow(rect);
		m_HanioData[m_CurCol].push(m_ActiveHanio);
		m_ActiveHanio = NULL;

		IncreaseSteps();
		GameOver();
	}
}

void ChanioDlg::Left()
{
	if (m_CurCol == 0)
		return;

	if (m_ActiveHanio)
	{
		CRect rect;
		CRect rt1;
		CRect rt2;

		m_Column[m_CurCol].GetWindowRect(rt1);
		ScreenToClient(&rt1);
		m_Column[m_CurCol-1].GetWindowRect(rt2);
		ScreenToClient(&rt2);

		m_ActiveHanio->btn->GetWindowRect(rect);
		ScreenToClient(&rect);

		rect.OffsetRect(CSize(rt2.left-rt1.left, 0));
		m_ActiveHanio->btn->MoveWindow(rect);
	}
	m_CurCol--;
	UpdateTriangle();
	//IncreaseSteps();
}

void ChanioDlg::Right()
{
	if (m_CurCol == COLUMN_COUNT-1)
		return;

	if (m_ActiveHanio)
	{
		CRect rect;
		CRect rt1;
		CRect rt2;

		m_Column[m_CurCol].GetWindowRect(rt1);
		ScreenToClient(&rt1);
		m_Column[m_CurCol+1].GetWindowRect(rt2);
		ScreenToClient(&rt2);

		m_ActiveHanio->btn->GetWindowRect(rect);
		ScreenToClient(&rect);

		rect.OffsetRect(CSize(rt2.left-rt1.left, 0));
		m_ActiveHanio->btn->MoveWindow(rect);
	}
	m_CurCol++;
	UpdateTriangle();
	//IncreaseSteps();
}

void ChanioDlg::GameOver()
{
	if (m_HanioData[COLUMN_COUNT-1].size() == m_Level)
	{
		PostMessage(WM_GAME_OVER, 0, 0);
	}
}

void ChanioDlg::IncreaseSteps()
{
	CString txt;
	txt.Format(_T("%d / %d"), m_Steps, m_StepsMin);
	GetDlgItem(IDC_STATIC)->SetWindowText(txt);
	m_Steps++;
}

void ChanioDlg::RestartGame()
{
	if (IDYES == MessageBox(_T("Are you want to restart game?"), NULL, MB_YESNO))
	{
		ExitThread();
		ResetHanio();
	}
}

void ChanioDlg::GotoLevel(int level)
{
	CString txt;
	txt.Format(_T("Are you want to goto level: %d ?"), level);
	if (IDYES == MessageBox(txt, NULL, MB_YESNO))
	{
		ExitThread();
		m_Level = level;
		ResetHanio();
	}
}

void ChanioDlg::AutoGame()
{
	if (IDYES == MessageBox(_T("Are you want to auto game?"), NULL, MB_YESNO))
	{
		ExitThread();
		ResetHanio();
		m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
		DWORD dwThreadId;
		m_hThread = CreateThread(NULL, 0, ThreadProc, this, CREATE_SUSPENDED, &dwThreadId);
		ResumeThread(m_hThread);
	}
}

void ChanioDlg::StopGame()
{
	if (IDYES == MessageBox(_T("Are you sure to stop auto game?"), NULL, MB_YESNO))
	{
		ExitThread();
	}
}

DWORD WINAPI ChanioDlg::ThreadProc(LPVOID param)
{
	ChanioDlg* _this = (ChanioDlg*)param;

	while (!_this->m_AutoGame.empty())
		_this->m_AutoGame.pop();

	_this->move(_this->m_Level, 0, 1, 2);
	_this->m_CurCol = 0;
	// ui show
	if (!_this->m_AutoGame.empty())
	{
		do
		{
			auto_game_e op = _this->m_AutoGame.front();
			_this->PostMessage(WM_AUTO_GAME, (WPARAM)op, NULL);
			_this->m_AutoGame.pop();
		} while (!_this->m_AutoGame.empty() && WAIT_TIMEOUT == WaitForSingleObject(_this->m_hEvent, _this->m_Intervals));
	}

	return 0;
}

void ChanioDlg::ExitThread()
{
	if (m_hThread)
	{
		if (m_hEvent)
		{
			SetEvent(m_hEvent);
		}
		WaitForSingleObject(m_hThread, INFINITE);
		CloseHandle(m_hEvent);
		m_hEvent = NULL;
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void ChanioDlg::move(int n, int from, int buffer, int to)
{
	if (n == 1)
	{
		//cout << "Move" << n << " from " << from << " to " << to << endl;
		while (from > m_CurCol)
		{
			m_AutoGame.push(AUTO_RIGHT);
			m_CurCol++;
		}
		while (from < m_CurCol)
		{
			m_AutoGame.push(AUTO_LEFT);
			m_CurCol--;
		}
		m_AutoGame.push(AUTO_UP);
		while (to > m_CurCol)
		{
			m_AutoGame.push(AUTO_RIGHT);
			m_CurCol++;
		}
		while (to < m_CurCol)
		{
			m_AutoGame.push(AUTO_LEFT);
			m_CurCol--;
		}
		m_AutoGame.push(AUTO_DOWN);
	}
	else
	{
		move(n-1, from, to, buffer);
		move(1, from, buffer, to);
		move(n-1, buffer, from, to);
	}
}
