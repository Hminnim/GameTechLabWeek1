#include "pch.h"
#include "UPlayerController.h"
#include "UInputManager.h"
#include "UEffectManager.h"
#include "USoundManager.h"

bool g_bHasPlayedChargeSound = false;

void UPlayerController::Update(std::vector<UPrimitive*>& primitives)
{
    if (UGameManager::GetInstance().CurrentTurnState != ETurnState::WaitInput)
    {
        return;
    }

    // 1. 공 선택 하기
    if (UInputManager::GetInstance().IsKeyDown(VK_LBUTTON))
    {
        POINT mouse = UInputManager::GetInstance().GetMousePos();
        for (auto* prim : primitives)
        {
            UBall* ball = dynamic_cast<UBall*>(prim);
            if (ball && UGameManager::GetInstance().CanSelectBall(ball))
            {
                float dx = ball->Location.x - (float)mouse.x;
                float dy = ball->Location.y - (float)mouse.y;
                if (dx * dx + dy * dy < ball->Radius * ball->Radius)
                {
                    if (ball != nullptr)
                        UEffectManager::GetInstance().ClearAura(SelectedBall);   // 이전 Ball Aura Effect 삭제

                    SelectedBall = ball; // 선택만!

                    // 선택된 공 뒤에 aura effect
                    UEffectManager::GetInstance().DrawAura(
                        SelectedBall,
                        "Resources/ball_aura.png",
                        DirectX::XMFLOAT2(SelectedBall->Location.x, SelectedBall->Location.y),
                        1.0f,
                        DirectX::XMFLOAT2(0.5f, 0.5f),
                        16
                    );
                    // OutputDebugStringA("DrawAura Finished\n");  // DEBUG
                    USoundManager::GetInstance().PlaySound("select_stone");
                    break;
                }
            }
        }
    }
    // 2. 우클릭 드래그: 발사 조작 (스킬 걸고 나서 쏘기!)
    if (SelectedBall != nullptr && UInputManager::GetInstance().IsKeyDown(VK_RBUTTON))
    {
        bIsDragging = true;
        SelectedBall->Velocity = FVector(0, 0, 0);
        UEffectManager::GetInstance().ClearAura(SelectedBall);   // aura Effect 해제
    }

    // 3. 드래그 중이면 매 프레임 화살표 update 하다가, 마우스 클릭 떼면 발사
    if (bIsDragging && SelectedBall != nullptr)
    {
        POINT mouse = UInputManager::GetInstance().GetMousePos();
        FVector launchVec(SelectedBall->Location.x - (float)mouse.x, SelectedBall->Location.y - (float)mouse.y, 0.0f);
        float pullDistance = launchVec.Length();
        if (!g_bHasPlayedChargeSound)
        {
            USoundManager::GetInstance().PlaySound("charge");
            g_bHasPlayedChargeSound = true;
        }
        if (UInputManager::GetInstance().IsKeyUp(VK_RBUTTON))
        {
            // 발사 처리
            g_bHasPlayedChargeSound = false;
            if (pullDistance > 5.0f)
            {
                float launchPower = 8.0f;
                SelectedBall->Velocity = launchVec * launchPower;
                float maxSpeed = 3000.0f;
                if (SelectedBall->Velocity.Length() > maxSpeed)
                {
                    SelectedBall->Velocity = (SelectedBall->Velocity / SelectedBall->Velocity.Length()) * maxSpeed;
                }

                // Shooting Effect 적용을 위한 발사각 계산 (좌클릭 -> 우클릭으로 변경)
                float launchAngle = atan2f(SelectedBall->Velocity.y, SelectedBall->Velocity.x);

                UEffectManager::GetInstance().PlayEffect(
                    "Resources/shooting.png",
                    DirectX::XMFLOAT2(SelectedBall->Location.x, SelectedBall->Location.y),
                    0.25f,
                    DirectX::XMFLOAT2(2.0f, 2.0f),
                    6,
                    false,
                    launchAngle
                );
				USoundManager::GetInstance().PlaySound("shoot");
            }

            UEffectManager::GetInstance().ClearAura(SelectedBall);
            SelectedBall = nullptr;
            UGameManager::GetInstance().CurrentTurnState = ETurnState::BallMoving;
            bIsDragging = false;
            UEffectManager::GetInstance().ClearArrow(); // 발사 완료 및 화살 삭제
        }
        else if (pullDistance > 5.0f)
        {
            // 드래그 중
            float launchAngle = atan2f(launchVec.y, launchVec.x);
            std::string arrowTexture = (UGameManager::GetInstance().CurrentPlayerTurn == EPlayer::Red)
                ? "Resources/Red_arrow.png"
                : "Resources/Blue_arrow.png";


            // 조준 방향
            FVector aimDir = launchVec / pullDistance;
            DirectX::XMFLOAT2 arrowPos =
            {
                SelectedBall->Location.x + aimDir.x * SelectedBall->Radius,
                SelectedBall->Location.y + aimDir.y * SelectedBall->Radius
            };

            // 당긴 거리에 비례하여 화살표 크기 조절
            float arrowScale = pullDistance / 200.0f;
            float minScale = 0.5f;
            float maxScale = 2.5f;
            arrowScale = std::clamp(arrowScale, minScale, maxScale);

            UEffectManager::GetInstance().DrawArrow(
                arrowTexture,
                arrowPos,
                launchAngle,
                1.0f,   // loop duration 
                DirectX::XMFLOAT2(arrowScale, 1.0f),
                30      // frame count
            );
        }
    }
    else
    {
        UEffectManager::GetInstance().ClearArrow();
    }
}

void UPlayerController::UseSkill(ESkillType skillType)
{    
    if (SelectedBall)
    {
        SelectedBall->ApplySkill(skillType);
    }
}
