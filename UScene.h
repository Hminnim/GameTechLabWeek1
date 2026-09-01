#pragma once
#include <d3d11.h>
#include <vector>
#include "UUI.h"
#include "UButton.h"

class UPrimitive;

// 한 화면을 구성하는 UPrimitive/UUI/UButton 묶음
// main.cpp에서 Scene[0].Render(); 처럼 사용
class UScene
{
    public:
        // 객체 추가
        void AddPrimitive(UPrimitive* primitive) { _primitives.push_back(primitive); }
        void AddUI(UUI* ui) { _uis.push_back(ui); }
        void AddButton(UButton* button) { _buttons.push_back(button); }

        // 렌더링 (월드 -> ui -> button)
        void Render(ID3D11DeviceContext* context);

        // 마우스 클릭 - 버튼 대응
        void HandleClick(float mouseX, float mouseY);

    private:
        std::vector<UPrimitive*> _primitives;
        std::vector<UUI*> _uis;
        std::vector<UButton*> _buttons;
};