#include "common.hlsl"

Texture2D g_small : register(t0);
SamplerState g_sampler : register(s0);


float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
    float4 c = g_small.Sample(g_sampler, uv);
    return c * intensityA;
}
