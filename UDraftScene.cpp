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
    UGameManager::GetInstance().NumRemainDraftSkills = NumButtons;

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
                UGameManager::GetInstance().ChangeDraftTurn();
            }
            else
            {
                UGameManager::GetInstance().BlueDraftedSkills.push_back(CurrentSkill);
                UGameManager::GetInstance().ChangeDraftTurn();
            }

            this->SetDraftedUI(CurrentSkill);
            UGameManager::GetInstance().NumRemainDraftSkills--;

            // 한번 클릭 시 비활성화
            draftBtn->SetActive(false);
        });

        AddUI(draftBtn);
    }

    // 고른 스킬 UI
    float DraftedWidth = ScreenWidth * (120.0f / 2040.0f);
    float DraftedHeight = DraftedWidth;
    float DraftedX = ScreenWidth * (92.0f / 2040.0f);
    float RightDraftedX = ScreenWidth - DraftedX - DraftedWidth;
    int NumDrafted = 5;
    float DraftedYInterval = ScreenHeight / (NumDrafted + 1);

    for (int i = 0; i < (int)ESlot::MaxCount; ++i)
    {
        std::string defaulTex = "Resources/button_freeze.png";

        int yIndex = (i % 5) + 1;
        float slotY = (DraftedYInterval * yIndex) - (DraftedWidth * 0.5f);
        float slotX = (i < (int)ESlot::MaxCount / 2) ? DraftedX : RightDraftedX;

        UUI* draftedUI = new UUI();

        draftedUI->Init(defaulTex, slotX, slotY, DraftedWidth, DraftedHeight);

        draftedUI->SetActive(false);

        AddDraftedUI(draftedUI);
    }
}

void UDraftScene::Update(float deltaTime)
{
    UScene::Update(deltaTime);

    if (m_bisNowEnter)
    {
        m_fadeoverlay.StartFadeIn(1.0f);
        m_bisNowEnter = false;
    }

    if (UGameManager::GetInstance().NumRemainDraftSkills <= 0 && !m_bIsFadingOut)
    {
        m_fadeoverlay.StartFadeOut(1.0f);
        m_bIsFadingOut = true;           
    }

    if (m_bIsFadingOut)
    {
        if (!m_fadeoverlay.IsFading())
        {
            USceneManager::GetInstance().RequestChangeScene("InGame");
        }
    }

    m_fadeoverlay.Update(deltaTime);
}

void UDraftScene::Render(URenderer& renderer)
{
    UScene::Render(renderer);

    renderer.BeginSprite();
    for (UUI* draftedUI : UDraftScene::m_draftedUIs)
    {
        draftedUI->Render(renderer);
    }
    m_fadeoverlay.Render(renderer);
    renderer.EndSprite();
}

void UDraftScene::Enter()
{
    UGameManager::GetInstance().InitDraft();

    for (UUI* btn : UDraftScene::_uis)
    {
        btn->SetActive(true);
    }

    for (UUI* draftedUI : UDraftScene::m_draftedUIs)
    {
        draftedUI->SetActive(false);
    }

    UGameManager::GetInstance().NumRemainDraftSkills = DraftSkills.size();
}

void UDraftScene::SetDraftedUI(ESkillType skillType)
{
    int pickCount = (int)ESlot::MaxCount - UGameManager::GetInstance().NumRemainDraftSkills;
    int currentIdx = (pickCount / 2) + ((UGameManager::GetInstance().CurrentDraftTurn == EPlayer::Red) ? 0 : 5);

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

    std::string baseName = SkillNames[(int)skillType];
    std::string normalTex = "Resources/button_" + baseName + ".png";

    m_draftedUIs[currentIdx]->ChangeTextureByTexturePath(normalTex);
    m_draftedUIs[currentIdx]->SetActive(true);

}
