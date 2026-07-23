#include "common.hlsl"

Texture2D g_textureA : register(t0); // 解像度の低いテクスチャ
Texture2D g_textureB : register(t1); // 解像度の高いテクスチャ
SamplerState g_sampler : register(s0);


float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
    float4 colorA = g_textureA.Sample(g_sampler, uv);
    float4 colorB = g_textureB.Sample(g_sampler, uv);

    // 2つのテクスチャの色を、それぞれの強度に応じて足し合わせる
    return ((colorA * intensityA) + (colorB * intensityB));
}
