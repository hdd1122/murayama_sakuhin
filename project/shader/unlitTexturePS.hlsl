
#include "common.hlsl"
//汎用オブジェクトシェーダー

Texture2D		g_Texture : register(t0);
Texture2D       g_EmissionTexture : register(t1);

SamplerState	g_SamplerState : register(s0);

PS_G_OUTPUT main(in PS_IN In)
{
    PS_G_OUTPUT Out;
    float4 finalColor;

    if (Material.TextureEnable)
    {
        finalColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
        finalColor *= In.Diffuse;
    }
    else
    {
        finalColor = In.Diffuse;
    }
 
         //アルファクリップ
    clip(finalColor.a - 0.01);
    
 
    Out.SceneColor = finalColor;
    // In.Normalはローカル空間なので、ワールド行列で変換
    float3 worldNormal = normalize(mul(float4(In.Normal, 0.0f), World).xyz);
    Out.Normal = float4(worldNormal, 1.0f);
    
    // SV_TARGET2: World Position
    Out.WorldPosition = float4(In.WorldPos, 1.0f);
    
    
// 基本のエミッション色を計算（texがない場合黒が入る0,0,0）
    float3 texEmission = g_EmissionTexture.Sample(g_SamplerState, In.TexCoord).rgb;
    float3 matEmission = bloomColor.Color.rgb;
    float intensity = bloomColor.Color.w;
    float3 finalBloom = (texEmission + matEmission) * intensity;


    // 距離減衰係数の計算
    float dist = distance(zone.CameraPosition.xyz, In.WorldPos);

    float BloomFadeStart = 50.0;
    float BloomFadeEnd = 200.0;
    
    // 最低限残したい明るさの割合 (0.0 ～ 1.0)
    float MinBloomIntensity = 0.6;

    // 基本係数を作る
    float baseWeight = 1.0 - smoothstep(BloomFadeStart, BloomFadeEnd, dist);
    
    // lerpを使って、「Min ～ 1.0」の範囲にマッピングする
    // baseWeightが 1.0 なら 1.0 になる
    // baseWeightが 0.0 なら MinBloomIntensity になる
    float distanceWeight = lerp(MinBloomIntensity, 1.0, baseWeight);
    
    // 減衰を適用
    finalBloom *= distanceWeight;

    Out.BloomContrib = float4(finalBloom, 1.0f);
    
    //ディファードでのライティング時にブルームのかかるオブジェクトが暗くなるのを防ぐ簡易実装
    Out.SceneColor.rgb += finalBloom;
   
    return Out;
}
