#pragma once
#include "UUI.h"
#include <functional>

// 클릭 가능한 UI (UUI 상속)
class UButton : public UUI
{
    public:
        // 클릭됐을 때 실행할 함수
        void SetOnClick(std::function<void()> callback) { _onClick = callback; }

        // 한 번 쓰면 사라져야 하는 버튼인지 설정 (EX. ITEM)
        void SetOneTimeUse(bool oneTime) { _isOneTimeUse = oneTime; }

        // 마우스 좌표 영역 판정
        bool HitTest(float mouseX, float mouseY) const;

        // 클릭 판정 성공
        void OnClick();

        // 호버링, 클릭 여부에 따라 다른 텍스쳐를 바인딩하여 draw..
        // virtual void Render(URenderer& renderer) override;
        
    private:
        std::function<void()> _onClick;
        bool _isOneTimeUse = false; // 일회성 여부
};