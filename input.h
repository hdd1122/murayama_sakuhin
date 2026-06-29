#pragma once


class Input
{
private:
	static BYTE m_OldKeyState[256];
	static BYTE m_KeyState[256];
	static int  m_WheelDelta;

public:
	static void Init();
	static void Uninit();
	static void Update();

	static bool GetKeyPress( BYTE KeyCode );
	static bool GetKeyTrigger( BYTE KeyCode );

	//
	static int GetWheel() { 
		int temp = m_WheelDelta;
		m_WheelDelta = 0; // 返却すると同時にリセット
		return temp;
	}
	//Windowsからの通知をセットする関数
	static void SetWheel(int delta) { m_WheelDelta = delta; }
};
