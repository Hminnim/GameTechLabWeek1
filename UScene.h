#pragma once
#include <d3d11.h>
#include <vector>
#include "UUI.h"
#include "UButton.h"
#include "UMap.h"
#include "UGameManager.h"
#include "UPlayerController.h"
#include "UFadeOverlay.h"

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
        void AddPrimitive(UPrimitive* primitive) { _pendingPrimitives.push_back(primitive); }
        void AddUI(UUI* ui) { _uis.push_back(ui); }
        void AddButton(UButton* button) { _buttons.push_back(button); }

		virtual void SetBackground(UUI* background) { _background = background; }
		void SetMap(UUI* map) { _map = map; }

        virtual void Update(float deltaTime);

        // 렌더링 (순서: world -> ui -> button)
        virtual void Render(URenderer& renderer);

        // 마우스 클릭 - 버튼 대응
        void HandleClick(float mouseX, float mouseY);

        UFadeOverlay& GetFadeOverlay() { return m_fadeOverlay; }

    protected:
        std::vector<UPrimitive*> _primitives;
        std::vector<UPrimitive*> _pendingPrimitives;
    private:     
        std::vector<UUI*> _uis;
        std::vector<UButton*> _buttons;
        UUI* _background;
        UUI* _map;

		UFadeOverlay m_fadeOverlay;

        std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;
};


class UTitleScene : public UScene
{
public:
	void Initialize() override;

	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
};


struct FSlotData 
{
    ESkillType AssignedSkill = ESkillType::None;
    float x, y;
};

enum class ESlot
{
	Slot1,
	Slot2,
	Slot3,
	Slot4,
	Slot5,
    Slot6,
    Slot7,
    Slot8,
    Slot9,
    Slot10,
    MaxCount
};

class UInGameScene : public UScene
{
public:
    void Initialize() override;
    virtual void SetBackground(UUI* backgroundBlue, UUI* backgroundRed) { m_backgrounds[EPlayer::Blue] = backgroundBlue; m_backgrounds[EPlayer::Red] = backgroundRed; }

	void AddSkillButton(USkillButton* skillButton) { m_skillButtons.push_back(skillButton); }
	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;
    void Enter() override;

private:
	std::unordered_map<EPlayer, UUI*> m_backgrounds;
	std::map<ESlot, FSlotData> m_slotData;
	std::vector<USkillButton*> m_skillButtons;
    
    UPlayerController PlayerController;
};

class UGameOverScene : public UScene
{
public:
    void Initialize() override;

	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;

    virtual void SetBackground(UUI* backgroundRedWin, UUI* backgroundBlueWin, UUI* backgroundDraw) {
        _resultBackgrounds[EGameResult::RedWin] = backgroundRedWin, _resultBackgrounds[EGameResult::BlueWin] = backgroundBlueWin
            , _resultBackgrounds[EGameResult::Draw] = backgroundDraw;
    }

    void Enter() override;

private:
    std::unordered_map<EGameResult, UUI*> _resultBackgrounds;
};

class DraftScene : public UScene
{
public:
	void Initialize() override;

	void Update(float deltaTime) override;
	void Render(URenderer& renderer) override;

	void Enter() override;

private:
	std::vector<USkillButton*> m_pickStageSkillButtons; // Init 에서 위치 계산해서 때려넣

};