#pragma once
#include "UUI.h"
#include <functional>

// 클릭 가능한 UI (UUI 상속)
class UButton : public UUI
{
    public:
		void SetUsedTexture(const std::string& textureKey) { _usedTextureKey = textureKey;}

        // 클릭됐을 때 실행할 함수
        void SetOnClick(std::function<void()> callback) { _onClick = callback; }

        // 마우스 좌표 영역 판정
        bool HitTest(float mouseX, float mouseY) const;

        // 클릭 판정 성공
        void OnClick();

        virtual void Render(URenderer& renderer) override;
        
    private:
        std::string _usedTextureKey;
        std::function<void()> _onClick;
		bool _isUsed = false;
};