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

	int ScreendWidth = 1400;
	int ScreenHeight = 1000;
};