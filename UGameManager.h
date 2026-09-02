#pragma once

class UPrimitive;
class UBall;

enum class EPlayer
{
	Red,
	Blue
};

enum class ETurnState
{
	WaitInput,
	BallMoving,
	GameOver
};

enum class EGameResult
{
	None,
	RedWin,
	BlueWin,
	Draw
};

class UGameManager
{
public:
	static UGameManager& GetInstance()
	{
		static UGameManager Instance;
		return Instance;
	}

	UGameManager();
	~UGameManager();
	UGameManager(const UGameManager&) = delete;
	UGameManager& operator=(const UGameManager&) = delete;

	// State
	EPlayer CurrentPlayerTurn;
	ETurnState CurrentTurnState;
	EGameResult CurrentGameResult = EGameResult::None;

	void Update(std::vector<UPrimitive*>& primitives);
	void InitGame();
	bool CanSelectBall(UBall* TargetBall);
	void CheckTurnEnd(std::vector<UPrimitive*>& primitives);
	void CheckGameOver(std::vector<UPrimitive*>& primitives);
};