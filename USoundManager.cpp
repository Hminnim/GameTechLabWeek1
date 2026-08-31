#include "pch.h"
#include "USoundManager.h"

FMOD::System* SoundSystem = nullptr;
FMOD::Sound* SoundEffect = nullptr;
FMOD::Channel* SoundChannel = nullptr;

bool USoundManager::Init()
{
	FMOD_RESULT result;
	result = FMOD::System_Create(&SoundSystem);
	if (result != FMOD_OK)
	{
		return false;
	}
	result = SoundSystem->init(512, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK)
	{
		return false;
	}
	return true;
}

void USoundManager::Update() {
	if (SoundSystem)
	{
		SoundSystem->update();
	}
}

bool USoundManager::LoadSound(const char* filename)
{
	if (!SoundSystem)
		return false;
	FMOD_RESULT result = SoundSystem->createSound(filename, FMOD_DEFAULT, nullptr, &SoundEffect);
	if (result != FMOD_OK)
	{
		return false;
	}
	return true;
}

void USoundManager::PlaySound()
{
	if (SoundSystem && SoundEffect)
	{
		SoundSystem->playSound(SoundEffect, nullptr, false, &SoundChannel);
	}
}

void USoundManager::Release()
{
	if (SoundEffect)
	{
		SoundEffect->release();
		SoundEffect = nullptr;
	}
	if (SoundSystem)
	{
		SoundSystem->close();
		SoundSystem->release();
		SoundSystem = nullptr;
	}
}