#include "common.hlsl"

// --- 入力: G-Buffer ---
Texture2D g_albedoTexture : register(t0);
Texture2D g_normalTexture : register(t1);
Texture2D g_worldPosTexture : register(t2);
Texture2DArray g_shadowMapTexture : register(t3); // シャドウマップも受け取る
SamplerState g_sampler : register(s0);
//SamplerComparisonState g_ShadowSampler : register(s1);//シャドウマップ用の比較サンプラー

SamplerComparisonState g_ShadowSampler : register(s1)
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = CLAMP;
    AddressV = CLAMP;
    ComparisonFunc = LESS_EQUAL;
};

float CalcShadow(float3 worldPos)
{
    // このピクセルのカメラからの距離を計算
    float cameraDepth = length(worldPos - zone.CameraPosition.xyz);
    
    int cascadeIndex = 0;
    if (cameraDepth > CascadeSplits.x)
        cascadeIndex = 1;
    if (cameraDepth > CascadeSplits.y)
        cascadeIndex = 2;

    float4 shadowCoord = mul(float4(worldPos, 1.0), matCascadeLightVP[cascadeIndex]);
   
    shadowCoord.xyz /= shadowCoord.w;

    
    shadowCoord.x = shadowCoord.x * 0.5 + 0.5;
    shadowCoord.y = shadowCoord.y * -0.5 + 0.5; // Yは反転
    
    float3 shadowSampleCoord = float3(shadowCoord.xy, cascadeIndex);
    
    float shadow = 0.0;
    float texelSize = 1.0 / 2048.0; // シャドウマップサイズC++の実装に合わせないとまずい

    for (int y = -1; y <= 1; y++)
    {
        for (int x = -1; x <= 1; x++)
        {
            shadow += g_shadowMapTexture.SampleCmpLevelZero(
          g_ShadowSampler,
         shadowSampleCoord + float3(x, y, 0) * texelSize,
        shadowCoord.z
        );
        }
    }
    shadow /= 9.0;
    
    return shadow;
}



PS_OUTPUT main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0)
{
    PS_OUTPUT Out;
   
    
    //gbuffer
    float4 albedo = g_albedoTexture.Sample(g_sampler, uv);
    
    //[0, 1] の範囲でエンコードされた法線を読み取る
    float3 normal_encoded = g_normalTexture.Sample(g_sampler, uv).rgb;
    // [-1, 1] の範囲にデコードする
    float3 normal = (normal_encoded * 2.0) - 1.0;
    
    float3 worldPos = g_worldPosTexture.Sample(g_sampler, uv).rgb;

    // G-Bufferが空(0,0,0)のピクセル(スカイドームなど)はライティングしない
    if (dot(worldPos, worldPos) < 0.0001f)
    {
        discard;
    }

    //ライティング
    float3 finalColor = 0;
    finalColor += albedo.rgb * Light.Ambient.rgb; // アンビエント

    float3 direction = normalize(Light.Direction.xyz);
    float diffuse = saturate(dot(normal, -direction));

    //影
    float shadow = CalcShadow(worldPos);
    diffuse *= shadow; //ディフューズに影を適用
   
    finalColor += albedo.rgb * Light.Diffuse.rgb * diffuse;
    
    //スペキュラ
    float3 eyev = normalize(worldPos.xyz - zone.CameraPosition.xyz);
    float3 halfv = normalize(eyev + (-direction)); //(Light.Directionはライトへの向きに)
    float specular = saturate(dot(normal, halfv));
    specular = pow(specular, 50);
    
    float specIntensity = 0.1;
    //スペキュラの色を決めて、調整
    finalColor.rgb += specular * Light.Diffuse.rgb * shadow * specIntensity;


    
    //フォグ処理
    // カメラとピクセル（ワールド座標）の距離を計算
    float dist = distance(worldPos, zone.CameraPosition.xyz);

    // フォグ係数を計算 (0.0 = フォグなし, 1.0 = 完全フォグ)
    // saturateで 0～1 にクランプするのが重要
    float fogFactor = saturate((dist - 400) / (600 - 400));

    // 元の色とフォグ色を線形補間
    float3 fogColor = float3(0.7f, 0.8f, 0.9f);
    finalColor = lerp(finalColor, fogColor, fogFactor);
    
 
    //出力
    Out.SceneColor = float4(finalColor, albedo.a);
    Out.BloomContrib = float4(0,0,0,0);

    return Out;
}
