#pragma once
#include "pch.h"

enum struct EKeyState
{
	None = 0,
	Press,
	Down,
	Up	
};

class UInputManager
{
public:
	static UInputManager& GetInstance() {
		static UInputManager Instance;
		return Instance;
	}

	UInputManager();
	~UInputManager();
	UInputManager(const UInputManager&) = delete;
	UInputManager& operator=(const UInputManager&) = delete;

	void Init(HWND hWnd);
	void Update();

	void OnKeyDown(unsigned long InKeyType);
	void OnKeyUp(unsigned long InKeyType);

	void OnMouseMove(long x, long y);

	bool IsKeyDown(unsigned long InKeyType) const { return mKeyState[InKeyType] == EKeyState::Down; }
	bool IsKeyPress(unsigned long InKeyType) const { return mKeyState[InKeyType] == EKeyState::Press; }
	bool IsKeyUp(unsigned long InKeyType) const { return mKeyState[InKeyType] == EKeyState::Up; }

	POINT GetMousePos() const { return MousePos; }

private:
	HWND m_hWnd;
	POINT MousePos;

	EKeyState mKeyState[256];
	bool bCurrentKeys[256];
	bool bPreviousKeys[256];
};