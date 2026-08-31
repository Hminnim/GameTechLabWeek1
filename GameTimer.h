#pragma once
#include <windows.h>

class GameTimer {
private:
	LARGE_INTEGER Frequency;
	LARGE_INTEGER CurrentTime;
	LARGE_INTEGER LastTime;

public:
	GameTimer();

	float GetDeltaTime();
};