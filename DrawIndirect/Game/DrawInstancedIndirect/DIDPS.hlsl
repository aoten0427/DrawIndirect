#include"DID.hlsli"



Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

float4 main(PSPNTInput input) : SV_TARGET
{
	 // まずテクスチャカラーを取得
    float4 texColor = g_texture.Sample(g_sampler, input.tex);
    
    // 法線ライティング
    float3 lightdir = normalize(LightDir.xyz);
    float3 N1 = normalize(input.norm);
    float lightIntensity = saturate(dot(N1, -lightdir));
    
    // ライティングとテクスチャを組み合わせる
    float4 finalColor = texColor * (lightIntensity * Diffuse + Emissive);
    finalColor.a = texColor.a * Diffuse.a;
    
    return finalColor;

}