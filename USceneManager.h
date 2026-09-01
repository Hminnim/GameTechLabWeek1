#pragma once
//#include "UScene.h"
class UScene;
class URenderer;

class USceneManager
{
public:
    static USceneManager& GetInstance();

    void Initialize() {};

    void AddScene(const std::string& name, UScene* scene);
    void ChangeScene(const std::string& name);
    void Update(float deltaTime);
    void Render(URenderer& renderer);

private:
    USceneManager() = default;
    ~USceneManager() = default;

private:
    std::unordered_map<std::string, UScene*> m_scenes;
    UScene* m_currentScene = nullptr;
};

