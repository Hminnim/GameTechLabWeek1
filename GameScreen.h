#pragma once
#include "UIManager.h"


/*
enum class ScreenType
{
    None,
    Main, // 메인 화면
    Item, // 돌의 기능(아이템)
    Pause, // 중지
    Result, // 승리 화면

};
*/

struct ItemSlot
{
    int itemId = -1;
    bool isUsed = false;
};

class MainScreen : public UIElement
{

}

class ItemScreen : public UIElement{

}

class PauseScreen : public UIElement{

}

class ResultScreen :  public UIElement{

}

// Game Initialization (UI Registeration)
inline void InitGameUI()
{

}