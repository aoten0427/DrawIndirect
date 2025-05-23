
struct VSPNTInstanceInput
{
    float4 Pos : POSITION;
    float3 Normal : NORMAL;
    float3 Tangetnt : TANGENT;
    float4 Color : COLOR;
    float2 Tex : TEXCOORD;
    float4x4 mat : MATRIX;
    uint InstanceId : SV_InstanceID;
};

struct PSPNTInput
{
    float4 position : SV_POSITION;
    float3 norm : NORMAL;
    float2 tex : TEXCOORD;
};

cbuffer SimpleConstantBuffer : register(b0)
{
    float4x4 World : packoffset(c0); //このワールド行列は使用しない
    float4x4 View : packoffset(c4);
    float4x4 Projection : packoffset(c8);
    float4 LightDir : packoffset(c12);
    float4 Emissive : packoffset(c13);
    float4 Diffuse : packoffset(c14);
};