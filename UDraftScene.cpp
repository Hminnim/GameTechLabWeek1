#include "pch.h"
#include "UDraftScene.h"
#include "UGameSetting.h"
#include "UGameManager.h"
#include "USceneManager.h"

void UDraftScene::Initialize()
{
    // 화면 크기에 따른 보정 ----------------------------------------------
    float ScreenWidth = (float)UGameSetting::GetInstance().ScreendWidth;
    float ScreenHeight = (float)UGameSetting::GetInstance().ScreenHeight;

    // 배경
    UUI* bg = new UUI();
    bg->Init("Resources/background_draft.png", 0, 0, ScreenWidth, ScreenHeight);
    AddUI(bg);

    // 버튼 크기와 간격
    float ButtonWidth = 100.0f;
    float ButtonHeight = 100.0f;
    float ButtonGap = 50.0f;
    float ButtonY = ScreenHeight * 0.5f - (ButtonHeight * 0.5f);

    // 들어갈 버튼 들의 이름과 스킬타입 목록(순서 동일)
    std::string SkillNames[] = {
        "none",
        "freeze",
        "giant",
        "heavier",
        "mine",
        "repulse",
        "wall",
        "shotgun",
        "ghost",
        "magnetic",
        "return"
    };
     DraftSkills = {
        ESkillType::Freeze, ESkillType::Giant, ESkillType::Heavier, ESkillType::Mine, ESkillType::Repulse,
        ESkillType::WallCreate, ESkillType::Shotgun, ESkillType::Ghost, ESkillType::Magnet, ESkillType::Return
    };

    int NumButtons = DraftSkills.size();
    NumRemainDraftSkills = NumButtons;

    float TotalWidth = (ButtonWidth * NumButtons) + (ButtonGap * (NumButtons - 1));
    float StartX = (ScreenWidth - TotalWidth) / 2.0f;

    for (int i = 0; i < NumButtons; i++)
    {
        ESkillType CurrentSkill = DraftSkills[i];

        std::string baseName = SkillNames[(int)CurrentSkill];
        std::string normalTex = "Resources/button_" + baseName + ".png";

        float currentX = StartX + i * (ButtonWidth + ButtonGap);

        UButton* draftBtn = new UButton();
        draftBtn->Init(normalTex, currentX, ButtonY, ButtonWidth, ButtonHeight);

        draftBtn->SetOnClick([this,draftBtn, CurrentSkill]() {
            bool bIsRedTurn = UGameManager::GetInstance().CurrentDraftTurn == EPlayer::Red;

            // 선택한 스킬 GameManager에게 전달
            if (bIsRedTurn)
            {
                UGameManager::GetInstance().RedDraftedSkills.push_back(CurrentSkill);
                UGameManager::GetInstance().CurrentDraftTurn = EPlayer::Blue;
            }
            else
            {
                UGameManager::GetInstance().BlueDraftedSkills.push_back(CurrentSkill);
                UGameManager::GetInstance().CurrentDraftTurn = EPlayer::Red;
            }

            NumRemainDraftSkills--;            

            // 한번 클릭 시 비활성화
            draftBtn->SetActive(false);
        });

        AddUI(draftBtn);
    }    
}

void UDraftScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);

    if (NumRemainDraftSkills <= 0)
    {
        USceneManager::GetInstance().ChangeScene("InGame");
    }
}

void UDraftScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);
}

void UDraftScene::Enter()
{
    // 골라둔 스킬 비우기
    UGameManager::GetInstance().RedDraftedSkills.clear();
    UGameManager::GetInstance().BlueDraftedSkills.clear();

    for (UUI* btn : UDraftScene::_uis)
    {
        btn->SetActive(true);
    }

    NumRemainDraftSkills = DraftSkills.size();
}
