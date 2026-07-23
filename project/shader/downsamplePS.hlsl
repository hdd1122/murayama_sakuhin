#include "common.hlsl"
Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

//これを定義する代わりにブルームカラーの定数バッファを使う float4のx,yに格納しておく
//cbuffer cbChangedOnResize : register(b8)
//{
//    float2 TexelSize;
//};

float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
   // 入力テクスチャの1ピクセルサイズ
    float2 texelSize = bloomColor.Color.xy;

    // --- 13-tap 重み付きサンプリング ---
    // 中心 + 四隅 をサンプリングすることで、4x4ピクセル相当の範囲をカバー。
    // これにより、1ドット単位の細かい描画によるノイズを防ぐ。

    // 中心 (Linearサンプラーにより、実質2x2の中央を拾う)
    float3 center = g_texture.Sample(g_sampler, uv).rgb;

    // 四隅 (中心から 1 texel ずつずらす)
    float3 tl = g_texture.Sample(g_sampler, uv + texelSize * float2(-1.0, -1.0)).rgb; // 左上
    float3 tr = g_texture.Sample(g_sampler, uv + texelSize * float2(1.0, -1.0)).rgb; // 右上
    float3 bl = g_texture.Sample(g_sampler, uv + texelSize * float2(-1.0, 1.0)).rgb; // 左下
    float3 br = g_texture.Sample(g_sampler, uv + texelSize * float2(1.0, 1.0)).rgb; // 右下

    // 重み付け平均 (中心を強めに、周辺を弱めに混ぜる)
    // 0.5 (中心) + 0.125 * 4 (四隅) = 1.0
    float3 color = center * 0.5 + (tl + tr + bl + br) * 0.125;

    return float4(color, 1.0);
}
