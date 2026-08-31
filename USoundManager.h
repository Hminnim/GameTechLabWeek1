#pragma once

#include "pch.h"

#include "inc/fmod.hpp"
#include "inc/fmod_errors.h"
#pragma comment(lib, "lib/fmodL_vc.lib")



class USoundManager
{
public:
	virtual bool Init();
	virtual void Update();
	virtual bool LoadSound(const char* filename);
	virtual void PlaySound();
	virtual void Release();
};
