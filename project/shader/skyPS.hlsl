#include "common.hlsl"

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

// フォグの色（LightingPSで設定したものと全く同じ色にする）
static const float3 FOG_COLOR_HORIZON = float3(0.75f, 0.8f, 0.9f);

// フォグが空を侵食する強さ
static const float FOG_HEIGHT_POWER = 3.0f;

float4 main(in PS_IN In) : SV_TARGET
{
    // スカイドームのテクスチャ色を取得
    float4 skyColor = float4(0, 0, 0, 1);
    if (Material.TextureEnable)
    {
        skyColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
        // 空にディフューズライトは乗算しなくていいか？
        skyColor *= In.Diffuse;
    }
    else
    {
        skyColor = In.Diffuse;
    }


    // --------------------------------------------------------
    // スカイフォグ処理（高さブレンド）
    // --------------------------------------------------------

    // 視線ベクトルの計算（
    float3 viewDir = normalize(In.WorldPos - zone.CameraPosition.xyz);

    // saturate
    float heightFactor = saturate(viewDir.y);

    // フォグ係数の計算
    // 高さがあるほど、フォグ成分を減らす
    // powを使うことで、地平線付近だけ急激にフォグをかけるカーブを作る
    float fogFactor = 1.0 - pow(heightFactor, 1.0 / FOG_HEIGHT_POWER);
    fogFactor = saturate(fogFactor);

    // 適用
    // 地平線(fogFactor=1)ならFogColor、上空(fogFactor=0)ならSkyTexture
    float3 finalColor = lerp(skyColor.rgb, FOG_COLOR_HORIZON, fogFactor);


    // --------------------------------------------------------

    // アルファは不透明(1.0)で返す
    return float4(finalColor, 1.0f);
}
