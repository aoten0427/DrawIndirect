
cbuffer Parameters : register(b0)
{
    float4x4 WorldViewProj;
}

//VS
struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangetnt : TANGENT;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
};

//PS
struct PS_INPUT
{
    float4 position : SV_POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXCOORD;
};