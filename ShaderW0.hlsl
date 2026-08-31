// ShaderW0.hlsl
cbuffer constants : register(b0)
{
    float3 Offset;
    float Scale;
    float3 Rotation;
    float Pad;
}

struct VS_INPUT
{
    float4 position : POSITION; // Input position from vertex buffer
    float4 color : COLOR; // Input color from vertex buffer
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // Transformed position to pass to the pixel shader
    float4 color : COLOR; // Color to pass to the pixel shader
};

float3x3 GetRotateX(float rad)
{
    float c = cos(rad), s = sin(rad);
    return float3x3(1, 0, 0, 0, c, -s, 0, s, c);
}
float3x3 GetRotateY(float rad)
{
    float c = cos(rad), s = sin(rad);
    return float3x3(c, 0, s, 0, 1, 0, -s, 0, c);
}
float3x3 GetRotateZ(float rad)
{
    float c = cos(rad), s = sin(rad);
    return float3x3(c, -s, 0, s, c, 0, 0, 0, 1);
}

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    float3 pos = input.position.xyz;
    
    pos *= Scale;
    
    pos = mul(pos, GetRotateX(Rotation.x));
    pos = mul(pos, GetRotateY(Rotation.y));
    pos = mul(pos, GetRotateZ(Rotation.z));
    
    pos += Offset;
    
    output.position = float4(pos, 1.0f);
    output.color = input.color;
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_TARGET
{
    // Output the color directly
    return input.color;
}
