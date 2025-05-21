#include"Model.hlsli"

PS_INPUT main(VS_INPUT input)
{
    
    PS_INPUT output = (PS_INPUT) 0;
    //テクスチャ情報セット
    output.tex = input.Tex;
    //法線情報セット
    output.norm = input.Normal;
   
    output.position = mul(float4(input.Pos, 1), WorldViewProj);
    
    return output;
}