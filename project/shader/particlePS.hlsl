#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

PS_OUTPUT main(in PS_IN In)
{
    // テクスチャをサンプリングし、頂点カラー(寿命アルファ)を掛ける

    PS_OUTPUT Out;
    Out.SceneColor = g_Texture.Sample(g_SamplerState, In.TexCoord) * In.Diffuse;
    Out.BloomContrib = float4(0.0, 0.0, 0.0, 1.0);
    return Out;
}
