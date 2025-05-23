#include"DID.hlsli"

Texture2D tex : register(t0);
SamplerState samLinear : register(s0);


float4 main(PS_INPUT input) : SV_TARGET
{
    //	Žw’è‚³‚ê‚½‰æ‘œ‚Ì•\Ž¦
    float4 color = tex.Sample(samLinear, input.tex);
    
    return color;
}