#include "common.hlsl"
#include "raySetting.hlsl"
//SDFレイマーチ実験
Texture2D g_sceneTexture : register(t0);
Texture2D g_depthTexture : register(t1);
SamplerState g_sampler : register(s0);




// 法線を計算する
float3 calcNormal(float3 p)
{//ずらすしてあーだこーだで傾き的なのが
    const float2 e = float2(0.001, 0.0);
    return normalize(float3(
        map_scene(p + e.xyy) - map_scene(p - e.xyy),
        map_scene(p + e.yxy) - map_scene(p - e.yxy),
        map_scene(p + e.yyx) - map_scene(p - e.yyx)
    ));
}



PS_G_OUTPUT main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0)
{
    // ポリゴンオブジェクトの深度を取得
    float sceneDepth = g_depthTexture.Sample(g_sampler, uv).r;
    if (sceneDepth >= 1.0)
    {
        sceneDepth = 1000.0;
    } // 空は遠方とする
    
    float2 screenPos = uv * 2.0 - 1.0;
    screenPos.y = -screenPos.y;

  // 1. プロジェクション逆行列を使って、スクリーン座標をビュー空間の方向に変換
    float4 viewDir_h = mul(float4(screenPos, 1.0, 1.0), r_inverseProjection);
    float3 viewDir = normalize(viewDir_h.xyz / viewDir_h.w);
    
    // 2. ビュー空間の方向を、ビュー逆行列を使ってワールド空間の方向に変換
    float3 rayDir = normalize(mul(viewDir, (float3x3) r_inverseViewProjection));

    // 3. レイマーチングを実行
    float t = 0;
    float3 p = r_cameraPosition;

  
    
    for (int i = 0; i < 1000; i++)
    {
        float d = map_scene(p);
        //float d = map_sceneTest(p);
        if (d < 0.001)
        {
            break;
        }
        p += rayDir * d;
        t += d;
        if (t > 1000.0)
        {
            break;
        }
    }
    
    // 2. レイマーチングで計算したワールド座標pを、クリップ空間の座標に変換
    float4 clipPos = mul(float4(p, 1.0), mul(View, Projection));
    
    // 3. クリップ空間の座標から、深度値(0.0～1.0)を計算
    float raymarchDepth = clipPos.z / clipPos.w;

    // 4. 生の深度値同士で比較する
    if (t > 100.0 || raymarchDepth > sceneDepth)
    {
        // レイが何にも衝突しなかった、またはポリゴンの後ろだった場合
        // -> このピクセルは一切描画しない
        discard;
    }


    float3 normal = calcNormal(p);


// c. 最終的な色
    PS_G_OUTPUT Out;
    float3 finalColor = { 0.5, 0.2, 0.2 };
    
    // b. 最終的な出力を設定
    Out.SceneColor = float4(finalColor, 1);
    Out.Normal = float4(normal, 1);
    Out.WorldPosition = float4(p, 1.0f);
  
    
    //Out.BloomContrib = (0,0,0,0); // 繰り返す構造にこのブルームはごみ
    
    return Out;
}

// ソフトシャドウを計算する関数
float calcSoftShadow(float3 ro, float3 rd, float maxDist)
{
    float res = 1.0;
    float t = 0.01;
    float penumbra = 8.0; // 半影の柔らかさを決める係数

    for (int i = 0; i < 32; i++)
    {
        if (t > maxDist)
            break;
        
        float h = map_scene(ro + rd * t);
        if (h < 0.001)
            return 0.0;
        
        // tが小さい（光源に近い）遮蔽物ほど、影がシャープになる
        res = min(res, penumbra * h / t);
        
        t += h;
    }
    return saturate(res);
}

