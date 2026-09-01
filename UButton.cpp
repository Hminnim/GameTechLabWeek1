#pragma once
#include "pch.h"
#include "UButton.h"

// 버튼을 클릭했는지 확인하는 function
bool UButton::HitTest(float mouseX, float mouseY) const
{
    // 비활성화 버튼이면 pass
    if (!IsActive())
        return false;

    // 활성화 버튼 영역 내 클릭이면 true
    return  mouseX >= GetX() && mouseX <= GetX() + GetWidth() &&
            mouseY >= GetY() && mouseY <= GetY() + GetHeight();

}

// 클릭했을 경우 (HitTest() -> true) 실행할 function
void UButton::OnClick()
{
    // 동작할 함수가 등록되어 있다면 실행
    if (_onClick)
        _onClick();

    // 일회성이면 버튼 비활성화 처리 
    if (_isOneTimeUse)
        SetActive(false);
}