#pragma once
#include "pch.h"

enum struct EKeyState
{
	None = 0,
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
	bool IsKeyUp(unsigned long InKeyType) const { return mKeyState[InKeyType] == EKeyState::Up; }

	POINT GetMousePos() const { return MousePos; }

private:
	HWND m_hWnd;

	EKeyState mKeyState[256];

	POINT MousePos;
};