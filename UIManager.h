#pragma once
#include <vector>
#include <unordered_map>

// UI 화면 종류
enum class ScreenType
{
    None,
    Main, // 메인 화면
    Item, // 돌의 기능(아이템)
    Pause, // 중지
    Result, // 승리 화면

};

// UI Element Base Class
class UIElement
{
public:
    virtual ~UIElement() = default;

    virtual void Init() {}
    virtual void Update(float deltaTime) {} 
    virtual void Render() {}
    virtual void Show() {}
    virtual void Hide() {}

    ScreenType GetType() const { return _type; }

protected:
    ScreenType _type = ScreenType::None;

};

// 싱글톤 UI Manager
class UIManager
{
public:
    // 싱글톤 접근
    static UIManager& GetInstance()
    {
        static UIManager instance;
        return instance;
    }

    // 복사/대입 방지
    UIManager(const UIManager&) = delete;
    UIManager& operator=(const UIManager&) = delete;

    // 게임 시작 시 화면 객체 등록
    void RegisterScreen(ScreenType type, UIElement* element)
    {
        element->Init();
        _registeredScreens[type] = element;          
    }

    // 화면 표시
    void ShowScreen(ScreenType type)
    {
        auto it = _registeredScreens.find(type);
        if (it == _registeredScreens.end())
            return;

        _activeScreens.push_back(it->second);
        it->second->Show();
    }

    // 화면 숨김
    void HideScreen(ScreenType type)
    {
        for (auto screen = _activeScreens.begin(); screen != _activeScreens.end(); ++screen)
        {
            if ((*screen)->GetType() == type)
            {
                (*screen)->Hide();
                _activeScreens.erase(screen);
                break;
            }
        }
    }

    // 맨 위 화면 숨김
    void HideTopScreen()
    {
        if (! _activeScreens.empty()){
            _activeScreens.back()->Hide();
            _activeScreens.pop_back();
        }
    } 

    // 모든 화면 정리
    void ClearAllScreens()
    {
        for (auto* screen : _activeScreens)
            screen->Hide();
        _activeScreens.clear();
    }

    // 매 프레임 갱신
    void Update(float deltaTime)
    {
        for (auto* screen : _activeScreens)
            screen->Update(deltaTime);
    }
    
    // 렌더링
    void Render()
    {
        for (auto* screen : _activeScreens) // stack 기반이므로 
            screen->Render();
    }
    
    // 화면 호출
    UIElement* GetScreen(ScreenType type)
    {
        auto it = _registeredScreens.find(type);
        return (it != _registeredScreens.end()) ? it->second : nullptr; // 화면 찾으면 UI 요소 반환, 못 찾으면 nullptr 반환

    }

private:
    UIManager() = default;
    ~UIManager() = default;

    std::unordered_map<ScreenType, UIElement*> _registeredScreens; // 등록된 화면들
    std::vector<UIElement*> _activeScreens; // 활성화 중인 화면들

};