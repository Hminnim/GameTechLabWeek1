#include "pch.h"
#include "UInputManager.h"

UInputManager::UInputManager()
{
}

UInputManager::~UInputManager()
{
}

void UInputManager::Init(HWND hWnd)
{
	m_hWnd = hWnd;

	for (int i = 0; i < 256; i++)
	{
		mKeyState[i] = EKeyState::None;
	}

	// 마우스 좌표 초기화
	POINT InitialMousePos;
	GetCursorPos(&InitialMousePos);
	ScreenToClient(m_hWnd, &InitialMousePos);
	MousePos = InitialMousePos;
}

void UInputManager::Update()
{
}

void UInputManager::OnKeyDown(unsigned long InKeyType)
{
	mKeyState[InKeyType] = EKeyState::Down;
}

void UInputManager::OnKeyUp(unsigned long InKeyType)
{
	mKeyState[InKeyType] = EKeyState::Up;
}

void UInputManager::OnMouseMove(long x, long y)
{
	MousePos.x = x;
	MousePos.y = y;
}
