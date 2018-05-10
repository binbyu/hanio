
// hanioelg.h : header file
//

#pragma once
#include "afxwin.h"

#define COLUMN_COUNT			3

struct hanio_data_t
{
	int index; // n
	CButton *btn;
};

enum auto_game_e
{
	AUTO_UP,
	AUTO_DOWN,
	AUTO_LEFT,
	AUTO_RIGHT
};

struct config_data_t
{
	int width;
	int height;
	int level;
	int intervals;
};

// ChanioDlg dialog
class ChanioDlg : public CDialogEx
{
// Construction
public:
	ChanioDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_HANIO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClose();
	afx_msg LRESULT OnGameOver(WPARAM wparam, LPARAM lparam);
	afx_msg LRESULT OnAutoGame(WPARAM wparam, LPARAM lparam);
	DECLARE_MESSAGE_MAP()

private:
	void ReadIniFile(config_data_t& data);
	void WriteIniFile(config_data_t* data = NULL);
	void ResetHanio();
	void UpdateTriangle();
	void Up();
	void Down();
	void Left();
	void Right();
	void GameOver();
	void IncreaseSteps();
	void RestartGame();
	void GotoLevel(int level);
	void AutoGame();
	void StopGame();
	void move(int n, int from, int buffer, int to);
	static DWORD WINAPI ThreadProc(LPVOID param);
	void ExitThread();

public:
	CStatic m_Column[COLUMN_COUNT];
	CPoint m_Triangle[COLUMN_COUNT];
	int m_CurCol;
	int m_Steps;
	int m_StepsMin;
	int m_Level;
	int m_Intervals;
	hanio_data_t* m_ActiveHanio;
	std::stack<hanio_data_t*> m_HanioData[COLUMN_COUNT];
	std::queue<auto_game_e> m_AutoGame;
	HANDLE m_hThread;
	HANDLE m_hEvent;
};
