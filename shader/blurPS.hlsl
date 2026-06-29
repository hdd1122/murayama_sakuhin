Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

// C++から渡すパラメータ
cbuffer BlurParams : register(b6)
{
    // ブラーをかける方向
    float2 g_blurDirection;
    
    // 1ピクセルのUV座標におけるサイズ
    float2 g_texelSize;
};

// ガウス分布の重み
static const float g_weights[11] =
{
    0.000977, 0.007813, 0.035156, 0.109375, 0.219727,
    0.253906, // 中心
    0.219727, 0.109375, 0.035156, 0.007813, 0.000977
};


float4 main(float4 pos : SV_POSITION, float2 uv : TEXCOORD0) : SV_Target
{
    float4 finalColor = float4(0.0f, 0.0f, 0.0f, 1.0f);

    // 中心ピクセルから-5～+5ピクセルまで、合計11ピクセルをサンプリング
    for (int i = -5; i <= 5; i++)
    {
        // サンプリングする座標を計算
        float2 sampleUV = uv + g_blurDirection * g_texelSize * i;
        
        // テクスチャから色を取得
        float4 sampleColor = g_texture.Sample(g_sampler, sampleUV);
        
        // 重みを掛けて色を足し合わせる
        finalColor += sampleColor * g_weights[i + 5];
    }

    return finalColor;
}
