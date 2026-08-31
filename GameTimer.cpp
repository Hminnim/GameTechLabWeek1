#include "pch.h"
#include "GameTimer.h"

GameTimer::GameTimer()
{
	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&LastTime);
	CurrentTime = LastTime;
}

float GameTimer::GetDeltaTime()
{
	float DeltaTime = 0.0f;

	QueryPerformanceCounter(&CurrentTime);
	DeltaTime = (float)(CurrentTime.QuadPart - LastTime.QuadPart) / Frequency.QuadPart;
	LastTime = CurrentTime;

	return DeltaTime;
}