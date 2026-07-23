#include "common.hlsl" // PS_IN, GBUFFER_PS_OUTPUT のため

// 戻り値の型が、G-Buffer用の出力構造体になる
PS_G_OUTPUT main(in PS_IN In)
{
    PS_G_OUTPUT Out;

    // まず、法線とワールド座標を決定する
    Out.Normal = float4(normalize(In.Normal), 1.0);
    Out.WorldPosition = float4(In.WorldPos, 1.0);


    // グリッド計算で、Albedoを決定
    float3 finalAlbedo; // 計算結果を保存するローカル変数

    float3 smoothNormal = normalize(In.Normal);
    float3 snappedNormal = round(smoothNormal);
    float snappedLengthSq = dot(snappedNormal, snappedNormal);

    if (snappedLengthSq > 1.0f)
    {
        // 角の部分の色
        finalAlbedo = float3(0.9f, 0.9f, 0.9f);
    }
    else
    {
        // 平らな面の部分のグリッド計算
        float3 blendWeights = abs(snappedNormal);
        float2 gridCoord = In.WorldPos.xz * blendWeights.y +
                           In.WorldPos.xy * blendWeights.z +
                           In.WorldPos.yz * blendWeights.x;
        
        float gridSpacing = 2.0f;
        float2 f = frac(gridCoord / gridSpacing);
        float lineWidth = 0.05f;

        if (f.x < lineWidth || f.x > (1.0 - lineWidth) ||
            f.y < lineWidth || f.y > (1.0 - lineWidth))
        {
            // グリッド線の色
            finalAlbedo = float3(0.2f, 0.2f, 0.2f);
        }
        else
        {
            // 地面の色
            finalAlbedo = float3(0.5f, 0.5f, 0.5f);
        }
    }

    // 出力
    Out.SceneColor = float4(finalAlbedo, 1.0f);
    
    return Out;
}
