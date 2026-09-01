#pragma once

#include "pch.h"

#include "inc/fmod.hpp"
#include "inc/fmod_errors.h"
#pragma comment(lib, "lib/fmodL_vc.lib")



class USoundManager
{
public:
	static USoundManager& GetInstance()
	{
		static USoundManager instance;
		return instance;
	}

	USoundManager(const USoundManager&) = delete;
	USoundManager& operator=(const USoundManager&) = delete;

	virtual bool Init();
	virtual void Update();
	virtual bool LoadSound(std::string soundName, const char* filename);
	virtual void LoadAllSounds(const std::string& folderPath);
	virtual void PlaySound(std::string soundName);
	virtual void Release();

private:
	USoundManager() = default;
	~USoundManager() = default;

	FMOD::System* SoundSystem = nullptr;
	FMOD::Sound* SoundEffect = nullptr;
	FMOD::Channel* SoundChannel = nullptr;
	std::unordered_map<std::string, FMOD::Sound*> SoundMap;

};
