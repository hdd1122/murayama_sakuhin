#include "common.hlsl"



// 高品質な疑似乱数ハッシュ関数 (0.0～1.0)
float random(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2du;
    seed = seed ^ (seed >> 15u);
    return float(seed) * (1.0 / 4294967296.0);
}

// 2Dベクトルの外積のZ成分を計算するヘルパー関数
float cross2d(float2 a, float2 b)
{
    return a.x * b.y - a.y * b.x;
}

PS_OUTPUT main(in PS_IN In)
{
    float2 uv = In.TexCoord; // 0.0～1.0

    // パーティクルのユニークIDを種にして、三角形の3つのランダムな頂点を生成
    uint id = In.UniqueID;
    float2 a = float2(random(id * 7u), random(id * 11u)); // 異なる素数を掛けてシードをずらす
    float2 b = float2(random(id * 13u), random(id * 17u));
    float2 c = float2(random(id * 19u), random(id * 23u));

    // 現在のピクセル(uv)が、三角形の内側にあるかをチェック
    float w0 = cross2d(b - a, uv - a);
    float w1 = cross2d(c - b, uv - b);
    float w2 = cross2d(a - c, uv - c);

    // w0, w1, w2の符号が全て同じなら、点は三角形の内側
    if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 <= 0 && w1 <= 0 && w2 <= 0))
    {
        // --- 三角形の内側 ---
        float finalAlpha = max(In.Diffuse.a * 0.5f, 0.2f);

        
        // ベース色
        float3 baseColor = float3(0.7, 0.9, 1.0);

        // プリマルチプライド
        float3 sceneColor = baseColor * finalAlpha;

        // Bloomは見えている明るさ基準
        float bloomIntensity = 0.3f; // 調整用
        float3 bloomColor = sceneColor * bloomIntensity;

        PS_OUTPUT Out;
        Out.SceneColor = float4(sceneColor, finalAlpha);
        Out.BloomContrib = float4(bloomColor, 0.1);
        return Out;

    }
    else
    {
        // --- 三角形の外側 ---
        
        // ピクセルを描画しない
        discard;
    }

    PS_OUTPUT Out;
    Out.SceneColor = float4(0, 0, 0, 0); // discardしたので、ここは実行されない
    float4 testColor = (0.0, 0.0, 1.0, 1.0);
    Out.BloomContrib = float4(testColor.rgb * 0.1f, 1.0);
    return Out;
   
}
