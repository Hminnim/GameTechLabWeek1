#include "pch.h"
#include "UGameManager.h"
#include "UBall.h"

UGameManager::UGameManager()
{
}

UGameManager::~UGameManager()
{
}

void UGameManager::Update(std::vector<UPrimitive*>& primitives)
{
	UGameManager::CheckTurnEnd(primitives);
	UGameManager::CheckGameOver(primitives);
}

void UGameManager::InitGame()
{
	CurrentPlayerTurn = EPlayer::Red;
	CurrentTurnState = ETurnState::WaitInput;
}

bool UGameManager::CanSelectBall(UBall* TargetBall)
{
	if (CurrentTurnState != ETurnState::WaitInput)
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
			UBall* ball = dynamic_cast<UBall*>(primitive);
			if (ball && ball->Owner == CurrentPlayerTurn)
			{
				ball->isFreezed = false;
			}
		}
		
		CurrentPlayerTurn = (CurrentPlayerTurn == EPlayer::Red ? EPlayer::Blue : EPlayer::Red);
		CurrentTurnState = ETurnState::WaitInput;
	}
}

void UGameManager::CheckGameOver(std::vector<UPrimitive*>& primitives)
{
	int RedBallCount = 0;
	int BlueBallCount = 0;

	for (auto* primitive : primitives)
	{
		if (primitive->Owner == EPlayer::Red)
		{
			RedBallCount += 1;
		}
		if (primitive->Owner == EPlayer::Blue)
		{
			BlueBallCount += 1;
		}
	}
}

