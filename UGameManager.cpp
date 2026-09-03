#include "pch.h"
#include "UGameManager.h"
#include "UBall.h"
#include "UWall.h"
#include "USceneManager.h"
#include "UGameSetting.h"
#include "USoundManager.h"
#include "UEffectManager.h"

UGameManager::UGameManager()
{
}

UGameManager::~UGameManager()
{
}

void UGameManager::Update(std::vector<UPrimitive*>& primitives, float deltaTime)
{
	// InGame 씬에서만 필요한 게임 판정
	if (USceneManager::GetInstance().GetCurrentSceneName() != "InGame")
	{
		return;
	}

	UGameManager::CheckTurnEnd(primitives);
	UGameManager::CheckGameOver(primitives);
	UGameManager::CheckFrozenTurnSkip(primitives,deltaTime);
}

void UGameManager::InitGame()
{
	CurrentPlayerTurn = EPlayer::Red;
	CurrentTurnState = ETurnState::WaitInput;
	CurrentGameResult = EGameResult::None;
	m_currentSelectedSkill = ESkillType::None;
}

bool UGameManager::CanSelectBall(UBall* TargetBall)
{
	if (CurrentTurnState != ETurnState::WaitInput && TargetBall == nullptr)
	{
		return false;
	}
	if (TargetBall->isFreezed)
	{
		return false;
	}
	if (TargetBall->Owner == CurrentPlayerTurn)
	{
		return true;
	}
	else
	{
		return false;
	}

}

void UGameManager::CheckTurnEnd(std::vector<UPrimitive*>& primitives)
{
	if (CurrentTurnState != ETurnState::BallMoving)
	{
		return;
	}

	bool bIsAllBallStopped = true;

	// 모든 공을 검사해서 0.1 이상이면 아직 턴 진행 중 판정
	for (auto* primitive : primitives)
	{
		if (primitive->Velocity.Length() > 0.1f)
		{
			bIsAllBallStopped = false;
			break;
		}
	}

	// 모든 공이 멈췄을 때
	if (bIsAllBallStopped)
	{
		for (auto* primitive : primitives)
		{						
			
			UWall* Wall = dynamic_cast<UWall*>(primitive);
			if (Wall != nullptr && Wall->wallowner != CurrentPlayerTurn)
			{
				Wall->bIsDestroyed = true;
			}						
			UBall* ball = dynamic_cast<UBall*>(primitive);
			if (ball && ball->bEnableReturn)
			{
				ball->Location = ball->Returnpos;
			}
			if (ball && ball->Owner == CurrentPlayerTurn)
			{
				ball->RemoveAllSkill();
				if (ball->isShotgunbullet) ball->bIsDestroyed = true;
			}			
		}		

		CurrentPlayerTurn = (CurrentPlayerTurn == EPlayer::Red ? EPlayer::Blue : EPlayer::Red);		
		CurrentTurnState = ETurnState::WaitInput;

		USoundManager::GetInstance().PlaySound("change_turn");
	}
}

void UGameManager::CheckGameOver(std::vector<UPrimitive*>& primitives)
{
	if (CurrentTurnState != ETurnState::WaitInput)
	{
		return;
	}

	int RedBallCount = 0;
	int BlueBallCount = 0;

	for (auto* primitive : primitives)
	{
		if (UBall* ball = dynamic_cast<UBall*>(primitive))
		{
			if (ball->Owner == EPlayer::Red)
			{
				RedBallCount += 1;				
			}
			else if (ball->Owner == EPlayer::Blue)
			{
				BlueBallCount += 1;				
			}
		}		
	}

	// Blue 승
	if (RedBallCount == 0 && BlueBallCount > 0)
	{
		OutputDebugStringA("===== BLUE WIN! =====\n");
		CurrentTurnState = ETurnState::GameOver;
		CurrentGameResult = EGameResult::BlueWin;
		USoundManager::GetInstance().PlaySound("win");
	}
	// Red 승
	else if (RedBallCount > 0 && BlueBallCount == 0)
	{
		OutputDebugStringA("===== RED WIN! =====\n");
		CurrentTurnState = ETurnState::GameOver;
		CurrentGameResult = EGameResult::RedWin;
		USoundManager::GetInstance().PlaySound("win");
	}
	// 무승부
	else if (RedBallCount == 0 && BlueBallCount == 0)
	{
		OutputDebugStringA("===== DRAW! =====\n");
		CurrentTurnState = ETurnState::GameOver;		
		CurrentGameResult = EGameResult::Draw;
		USoundManager::GetInstance().PlaySound("win");
	}

	if (CurrentGameResult != EGameResult::None)
	{
		for (auto* primitive : primitives)
		{
			if (UBall* ball = dynamic_cast<UBall*>(primitive))
			{
				UEffectManager::GetInstance().ClearAura(ball);
			}
		}
		USceneManager::GetInstance().RequestChangeScene("GameOver");
	}

}

void UGameManager::CheckFrozenTurnSkip(std::vector<UPrimitive*>& primitives, float deltaTime)
{
	int myBallCount = 0;
	int frozenBallCount = 0;
	for (auto* primitive : primitives)
	{
		UBall* ball = dynamic_cast<UBall*>(primitive);
		if (ball != nullptr && !ball->bIsDestroyed && ball->Owner == CurrentPlayerTurn)
		{
			myBallCount++;
			if (ball->isFreezed)
			{
				frozenBallCount++;
			}
		}
	}

	if (myBallCount > 0 && myBallCount == frozenBallCount) 
	{
		m_frozenTimer += deltaTime;
		if (m_frozenTimer < 1.5f)
		{
			return;
		}
		
		m_frozenTimer = 0.0f;

		for (auto* primitive : primitives)
		{
			UBall* ball = dynamic_cast<UBall*>(primitive);
			if (ball != nullptr && ball->Owner == CurrentPlayerTurn)
			{
				ball->isFreezed = false;
			}
		}
		
		CurrentPlayerTurn = (CurrentPlayerTurn == EPlayer::Red ? EPlayer::Blue : EPlayer::Red);
		CurrentTurnState = ETurnState::WaitInput;

		std::string turnTexture = (CurrentPlayerTurn == EPlayer::Red) ? "Resources/Red_turn.png" : "Resources/Blue_turn.png";
		UEffectManager::GetInstance().PlayEffect(
			turnTexture,
			{ (float)UGameSetting::GetInstance().ScreendWidth * 0.5f, (float)UGameSetting::GetInstance().ScreenHeight * 0.5f },
			1.0f,
			DirectX::XMFLOAT2(1.0f, 1.0f),
			1
		);
	}
}