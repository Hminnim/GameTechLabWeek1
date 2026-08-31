#include "pch.h"
#include "USoundManager.h"

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

bool USoundManager::LoadSound(std::string soundName, const char* filename)
{
	if (!SoundSystem)
		return false;
	FMOD_RESULT result = SoundSystem->createSound(filename, FMOD_DEFAULT, nullptr, &SoundEffect);
	SoundMap.insert({ soundName, SoundEffect });
	if (result != FMOD_OK)
	{
		return false;
	}
	return true;
}

void USoundManager::PlaySound(std::string soundName)
{
	auto it = SoundMap.find(soundName);
	if (it != SoundMap.end())
	{
		SoundSystem->playSound(it->second, nullptr, false, &SoundChannel);
	}
}

void USoundManager::Release()
{
	for (auto& pair : SoundMap)
	{
		if (pair.second)
		{
			pair.second->release();
		}
	}
	SoundMap.clear();
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