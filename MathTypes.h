#pragma once
#include <math.h>

struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
};

// Structure for a 3D vector
struct FVector
{
    float x, y, z;
    FVector(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

    // 연산자 오버로딩
    FVector operator+(const FVector& v) const { return FVector(x + v.x, y + v.y, z + v.z); }
    FVector operator-(const FVector& v) const { return FVector(x - v.x, y - v.y, z - v.z); }
    FVector operator*(float s) const { return FVector(x * s, y * s, z * s); }
    FVector operator/(float s) const { return FVector(x / s, y / s, z / s); }
    void operator+=(const FVector& v) { x += v.x; y += v.y; z += v.z; }
    void operator-=(const FVector& v) { x -= v.x; y -= v.y; z -= v.z; }

    float LengthSquared() const
    {
        return x * x + y * y + z * z;
    }
    float Length() const {
        return (float)sqrt(LengthSquared());
    }
    float Dot(const FVector& v) const {
        return x * v.x + y * v.y + z * v.z;
    }
    FVector Cross(const FVector& v) const
    {
        return FVector(
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        );
    }
};