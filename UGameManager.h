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

enum class ESkillType
{
	None,
	Freeze,
	Giant,
	Heavier,
	Mine,
	Repulse,
	WallCreate,
	Shotgun,
	Ghost,
	Magnet,
	Return
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

	// InGame State
	EPlayer CurrentPlayerTurn;
	ETurnState CurrentTurnState;
	ESkillType m_currentSelectedSkill;
	std::map<EPlayer, std::map<ESkillType, bool>> m_usedSkills;
	EGameResult CurrentGameResult = EGameResult::None;
	float m_frozenTimer = 0.0f;

	// Draft
	EPlayer CurrentDraftTurn;
	std::vector<ESkillType> RedDraftedSkills;
	std::vector<ESkillType> BlueDraftedSkills;

	void Update(std::vector<UPrimitive*>& primitives,float Deltatime);
	void InitGame();
	bool CanSelectBall(UBall* TargetBall);
	void CheckTurnEnd(std::vector<UPrimitive*>& primitives);
	void CheckGameOver(std::vector<UPrimitive*>& primitives);
	void CheckFrozenTurnSkip(std::vector<UPrimitive*>& primitives,float deltaTime);

	void SetCurrentSelectedSkill(ESkillType skill) { m_currentSelectedSkill = skill; }
	void ResetSkiils() { m_usedSkills.clear(); }
	void ConsumeSkill(EPlayer player, ESkillType skill) { m_usedSkills[player][skill] = true; }
	void ConsumeCurrentSkill() { m_usedSkills[CurrentPlayerTurn][m_currentSelectedSkill] = true; }
};