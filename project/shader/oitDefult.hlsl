// oitAccum_ps.hlsl

#include "common.hlsl" // 既存の共通ファイルをインクルード

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// 2つのレンダーターゲットに出力するための構造体
struct OIT_PS_OUTPUT
{
    float4 Accumulation : SV_Target0; // 色を蓄積するターゲット
    float4 Revealage : SV_Target1; // 透明度を蓄積するターゲット
};


OIT_PS_OUTPUT main(PS_IN In)
{
    OIT_PS_OUTPUT output;

    float4 baseColor = Material.Diffuse;
    
    if (Material.TextureEnable)
    {
        float4 texColor = g_texture.Sample(g_sampler, In.TexCoord);
        baseColor.rgb *= texColor.rgb;
        baseColor.a *= texColor.a;
    }

    // アルファ値が0.5未満のピクセルは、これ以降の処理をせず破棄する
    clip(baseColor.a - 0.5f);
    
    // 最終的な出力値を計算
    output.Accumulation = float4(baseColor.rgb * baseColor.a, baseColor.a);
    output.Revealage.r = baseColor.a;
    output.Revealage.gba = 0.0f;

    return output;
}
