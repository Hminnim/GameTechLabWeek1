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

void  USceneManager::ChangeScene(const std::string& name) 
{
	if (m_currentScene != nullptr) {
		m_currentScene->Exit();
	}
	m_currentScene = m_scenes[name];
	m_currentSceneName = name;
	if (m_currentScene != nullptr) {
		m_currentScene->Enter();
	}
}

void USceneManager::Update(float deltaTime)
{
	if (m_currentScene) m_currentScene->Update(deltaTime);
}

void USceneManager::Render(URenderer& renderer)
{
	if (m_currentScene) m_currentScene->Render(renderer);
}
