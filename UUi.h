#pragma once
#include <string>

class URenderer;

// 모든 UI 요소(아이콘, 배경, 버튼 등) Base Class
class UUI
{
    public:
        virtual ~UUI() = default;

        // virtual functions
        virtual bool Init(const std::string& texturePath, float x, float y, float width, float height);
        virtual void Update(float deltaTime) {};
        virtual void Render(URenderer& renderer);

        // Setter
        void SetActive(bool active) { _isActive = active; }
		void SetPosition(float x, float y) { _x = x; _y = y; }
        void SetRotation(float radian) { _rotation = radian; }

        // Check the status (item or not)
        bool IsActive() const { return _isActive; }

        // Getter
        float GetX() const { return _x; }
        float GetY() const { return _y; }
        float GetWidth() const { return _width; }
        float GetHeight() const { return _height; }

    protected:
        // position
        float _x = 0.0f;
        float _y = 0.0f;

        // roattion
        float _rotation = 0.0f;

        // scale
        float _width = 0.0f;
        float _height = 0.0f;

        // mesh + srv
        ID3D11Buffer* _mesh = nullptr;
        ID3D11ShaderResourceView* _srv = nullptr;
        std::string _textureKey;

        // false면 Render()에서 건너뜀 (일회성 사용)
        bool _isActive = true;
};
