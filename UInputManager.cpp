#include "pch.h"
#include "UInputManager.h"

UInputManager::UInputManager()
{
	for (int i = 0; i < 256; i++)
	{
		mKeyState[i] = EKeyState::None;
		bCurrentKeys[i] = false;
		bPreviousKeys[i] = false;
	}
}

UInputManager::~UInputManager()
{
}

void UInputManager::Init(HWND hWnd)
{
	m_hWnd = hWnd;

	// 마우스 좌표 초기화
	POINT InitialMousePos;
	GetCursorPos(&InitialMousePos);
	ScreenToClient(m_hWnd, &InitialMousePos);
	MousePos = InitialMousePos;
}

void UInputManager::Update()
{
	for (int i = 0; i < 256; i++)
	{
		// 현재 키가 눌려져 있을 때
		if (bCurrentKeys[i])
		{
			if (bPreviousKeys[i])
			{
				mKeyState[i] = EKeyState::Press;
			}
			else
			{
				mKeyState[i] = EKeyState::Down;
			}
		}
		// 현재 키를 뗐을 때
		else
		{
			if (bPreviousKeys[i])
			{
				mKeyState[i] = EKeyState::Up;
			}
			else
			{
				mKeyState[i] = EKeyState::None;
			}
		}
		bPreviousKeys[i] = bCurrentKeys[i];
	}
}

void UInputManager::OnKeyDown(unsigned long InKeyType)
{
	bCurrentKeys[InKeyType] = true;
}

void UInputManager::OnKeyUp(unsigned long InKeyType)
{
	bCurrentKeys[InKeyType] = false;
}

void UInputManager::OnMouseMove(long x, long y)
{
	MousePos.x = x;
	MousePos.y = y;
}