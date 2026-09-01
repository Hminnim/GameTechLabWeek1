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
        const std::vector<UPrimitive*>& GetPrimitives() const { return _primitives; }
        virtual void Initialize() {};
        virtual void Enter() {};
        virtual void Exit() {};

        // 객체 추가
        void AddPrimitive(UPrimitive* primitive) { _primitives.push_back(primitive); }
        void AddUI(UUI* ui) { _uis.push_back(ui); }
        void AddButton(UButton* button) { _buttons.push_back(button); }

        virtual void Update(float deltaTime);

        // 렌더링 (순서: world -> ui -> button)
        virtual void Render(URenderer& renderer);

        // 마우스 클릭 - 버튼 대응
        void HandleClick(float mouseX, float mouseY);

    private:
        std::vector<UPrimitive*> _primitives;
        std::vector<UUI*> _uis;
        std::vector<UButton*> _buttons;

        std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
};


class UTitleScene : public UScene
{
public:
	void Initialize() override;
	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
};

class UInGameScene : public UScene
{
public:
    void Initialize() override;

	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
};

class UGameOverScene : public UScene
{
public:
    void Initialize() override;

	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
};
