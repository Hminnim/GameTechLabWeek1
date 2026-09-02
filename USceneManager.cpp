#include "pch.h"
#include "USceneManager.h"
#include "UScene.h"
#include "URenderer.h"

USceneManager& USceneManager::GetInstance()
{
    static USceneManager instance;
    return instance;
}

void USceneManager::AddScene(const std::string& name, UScene* scene) 
{
	scene->Initialize();
	m_scenes[name] = scene;
}

void USceneManager::RequestChangeScene(const std::string& name)
{
	if (m_hasPendingSceneChange) return;
	m_hasPendingSceneChange = true;
	m_pendingSceneName = name;
	if (m_currentScene)
	{
		m_currentScene->GetFadeOverlay().StartFadeOut(1.0f);
	}
}

void  USceneManager::ChangeScene(const std::string& name) 
{
	if (m_currentScene != nullptr) 
	{
		m_currentScene->Exit();
	}
	m_currentScene = m_scenes[name];
	m_currentSceneName = name;
	if (m_currentScene != nullptr) 
	{
		m_currentScene->Enter();
	}
}

void USceneManager::Update(float deltaTime)
{
	if (m_currentScene)
	{
		m_currentScene->GetFadeOverlay().Update(deltaTime);
	}

	if (m_hasPendingSceneChange && (m_currentScene == nullptr || !m_currentScene->GetFadeOverlay().IsFading()))
	{
		m_hasPendingSceneChange = false;
		std::string nextScene = m_pendingSceneName;
		ChangeScene(nextScene);
		if (m_currentScene)
		{
			m_currentScene->GetFadeOverlay().StartFadeIn(1.0f);
		}
	}

	if (m_currentScene)
	{
		m_currentScene->Update(deltaTime);
	}
}

void USceneManager::Render(URenderer& renderer)
{
	if (m_currentScene) m_currentScene->Render(renderer);
}
