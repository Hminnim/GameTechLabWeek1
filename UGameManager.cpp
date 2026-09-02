#include "pch.h"
#include "UGameManager.h"
#include "UBall.h"
#include "UWall.h"
#include "USceneManager.h"
#include "UGameSetting.h"

UGameManager::UGameManager()
{
}

UGameManager::~UGameManager()
{
}

void UGameManager::Update(std::vector<UPrimitive*>& primitives)
{
	// InGame 씬에서만 필요한 게임 판정
	if (USceneManager::GetInstance().GetCurrentSceneName() != "InGame")
	{
		return;
	}

	UGameManager::CheckTurnEnd(primitives);
	UGameManager::CheckGameOver(primitives);
}

void UGameManager::InitGame()
{
	CurrentPlayerTurn = EPlayer::Red;
	CurrentTurnState = ETurnState::WaitInput;
	CurrentGameResult = EGameResult::None;
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
			if (Wall != nullptr && Wall->Owner != CurrentPlayerTurn)
			{
				Wall->bIsDestroyed = true;
			}						
			UBall* ball = dynamic_cast<UBall*>(primitive);
			if (ball && ball->Owner == CurrentPlayerTurn)
			{
				ball->isFreezed = false;
				ball->bEnableFreeze = false;
				ball->bEnableWallCreate = false;
				ball->isSelfDestruct = false;
				ball->TargetRadius = UGameSetting::GetInstance().BallBaseRadius;
				ball->isSizeScaling = true;
				ball->TargetMass = UGameSetting::GetInstance().BallBaseRadius * 10.0f;
				ball->isMassScaling = true;				
			}
		}
		// 스킬 관련 업데이트 사항
		m_usedSkills[CurrentPlayerTurn][m_currentSelectedSkill] = true;

		CurrentPlayerTurn = (CurrentPlayerTurn == EPlayer::Red ? EPlayer::Blue : EPlayer::Red);
		CurrentTurnState = ETurnState::WaitInput;
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
	}
	// Red 승
	else if (RedBallCount > 0 && BlueBallCount == 0)
	{
		OutputDebugStringA("===== RED WIN! =====\n");
		CurrentTurnState = ETurnState::GameOver;
		CurrentGameResult = EGameResult::RedWin;
	}
	// 무승부
	else if (RedBallCount == 0 && BlueBallCount == 0)
	{
		OutputDebugStringA("===== DRAW! =====\n");
		CurrentTurnState = ETurnState::GameOver;		
		CurrentGameResult = EGameResult::Draw;
	}

	if (CurrentGameResult != EGameResult::None)
	{
		USceneManager::GetInstance().ChangeScene("GameOver");
	}
}