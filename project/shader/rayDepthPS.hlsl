#include "common.hlsl"
#include "raySetting.hlsl"

//SDFレイマーチングの深度用　現在使用しておらず

Texture2D g_sceneTexture : register(t0);
Texture2D g_depthTexture : register(t1);
SamplerState g_sampler : register(s0);



// 法線を計算する
float3 calcNormal(float3 p)
{ //ずらすして傾き的なのが
    const float2 e = float2(0.001, 0.0);
    return normalize(float3(
        map_scene(p + e.xyy) - map_scene(p - e.xyy),
        map_scene(p + e.yxy) - map_scene(p - e.yxy),
        map_scene(p + e.yyx) - map_scene(p - e.yyx)
    ));
}



void main(float4 pos : SV_POSITION,
    float2 uv : TEXCOORD0,
    out float out_depth : SV_Depth)
{
    // ポリゴンオブジェクトの深度を取得
    float sceneDepth = g_depthTexture.Sample(g_sampler, uv).r;
    if (sceneDepth >= 1.0)
    {
        sceneDepth = 1000.0;
    } // 空は遠方とする
    
    float2 screenPos = uv * 2.0 - 1.0;
    screenPos.y = -screenPos.y;

  // プロジェクション逆行列を使って、スクリーン座標をビュー空間の方向に変換
    float4 viewDir_h = mul(float4(screenPos, 1.0, 1.0), r_inverseProjection);
    float3 viewDir = normalize(viewDir_h.xyz / viewDir_h.w);
    
    // ビュー空間の方向を、ビュー逆行列を使ってワールド空間の方向に変換
    float3 rayDir = normalize(mul(viewDir, (float3x3) r_inverseViewProjection));

    // レイマーチングを実行
    float t = 0;
    float3 p = r_cameraPosition;

  
    
    for (int i = 0; i < 100; i++)
    {
        float d = map_scene(p);
        //float d = map_sceneTest(p);
        if (d < 0.001)
        {
            break;
        }
        p += rayDir * d;
        t += d;
        if (t > 100.0)
        {
            //これを設定しないとスカイドームなどのオブジェクトが描画出来なくなる
            out_depth = 1.0;
            return;
        }
    }
    
    // レイマーチングで計算したワールド座標pを、クリップ空間の座標に変換
    float4 clipPos = mul(float4(p, 1.0), mul(View, Projection));
    
    // クリップ空間の座標から、深度値(0.0～1.0)を計算
    float raymarchDepth = clipPos.z / clipPos.w;

    out_depth = raymarchDepth;


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

