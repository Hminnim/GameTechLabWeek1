#pragma once

#include "pch.h"

#include "inc/fmod.hpp"
#include "inc/fmod_errors.h"
#pragma comment(lib, "lib/fmodL_vc.lib")



class USoundManager
{
	FMOD::System* SoundSystem = nullptr;
	FMOD::Sound* SoundEffect = nullptr;
	FMOD::Channel* SoundChannel = nullptr;
	std::unordered_map<std::string, FMOD::Sound*> SoundMap;

public:
	virtual bool Init();
	virtual void Update();
	virtual bool LoadSound(std::string soundName, const char* filename);
	virtual void PlaySound(std::string soundName);
	virtual void Release();
};
