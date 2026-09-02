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
};