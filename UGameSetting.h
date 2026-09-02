#pragma once

class UGameSetting
{
public:
	static UGameSetting& GetInstance()
	{
		static UGameSetting instance;
		return instance;
	}

	UGameSetting() {}
	~UGameSetting() {}

	int ScreendWidth = 2040;
	int ScreenHeight = 1200;

	float MapMarginX = 300.0f;
	float MapMarginY = 100.0f;

	// 공들 규칙
	const float BallBaseRadius = 30.0f;
	const int BallsPerTeam = 5;
};