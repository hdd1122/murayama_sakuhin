#include "common.hlsl"

Texture2D<float4> g_accumTex : register(t0); // 蓄積バッファ
Texture2D<float> g_revealTex : register(t1); // 透過率バッファ (R成分のみ使用)
SamplerState g_sampler : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};

PS_OUTPUT main(PS_INPUT input)
{
    // テクスチャ読み込み
    int3 pos = int3(input.position.xy, 0);
    float4 accum = g_accumTex.Load(pos);
    float reveal = g_revealTex.Load(pos).r;

    // WBOITの色の復元式
    
    // 平均色 = (蓄積された色) / (蓄積された重み [accum.a])
    // ゼロ除算回避のため max を使用
    float3 averageColor = accum.rgb / max(accum.a, 0.00001);

    // 最終的なアルファ (不透明度)
    float finalAlpha = 1.0 - reveal;

    // アルファがほぼ0のときのゴミ消し
    if (finalAlpha < 0.0001)
        discard;


    PS_OUTPUT Out;
    
    // シーンカラーに出力
    Out.SceneColor = float4(averageColor, finalAlpha);
    
    // -----------------------------------------------------------
    // ブルーム抽出 (閾値処理 & アルファ制御)
    // -----------------------------------------------------------
    
    // 閾値処理: 1.0 を超えた明るさだけを取り出す
    float3 bloomRGB = max(0.0f, averageColor - 1.0f);

    // ブルーム成分を持っているか判定
    float isBlooming = step(0.0001, dot(bloomRGB, bloomRGB));

    // ブルーム出力
    // RGB: 抽出した光 * 半透明度
    // A: 光っている時だけ」半透明度を入れる。光ってないなら 0.0 にする。
    Out.BloomContrib = float4(bloomRGB * finalAlpha, finalAlpha * isBlooming);


    return Out;
}
