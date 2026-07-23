#include "common.hlsl"
//gbufferではない物でもレンダーターゲットの割り振りで使いまわしている

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

PS_G_OUTPUT main(in PS_IN In)
{
    PS_G_OUTPUT Out;
    float4 finalColor;

    //フィルターにこの部分はいらない
    //if (Material.TextureEnable)
    //{
    //    finalColor = g_Texture.Sample(g_SamplerState, In.TexCoord);
    //    finalColor *= In.Diffuse;
    //}
    //else
    //{
    //    finalColor = In.Diffuse;
    //}
 
    //輪郭抽出
    
    float offset = 1.0 / 500.0;
    float4 color[3][3];
    
    color[0][1] = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(-offset, 0.0));
    color[2][1] = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(offset, 0.0));
    color[1][0] = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(0.0, -offset));
    color[1][2] = g_Texture.Sample(g_SamplerState, In.TexCoord + float2(0.0, offset));
    color[1][1] = g_Texture.Sample(g_SamplerState, In.TexCoord);
        
    finalColor = color[0][1]
        + color[2][1]
        + color[1][0]
        + color[1][2]
        + color[1][1] * -4.0;
    
    finalColor.a = 1.0;
   
    
    Out.SceneColor = finalColor;
    
    //フィルターだとgbufferのレンダーターゲットをセットしてないからここからの処理は不要
    
    // In.Normalはローカル空間なので、ワールド行列で変換
    float3 worldNormal = normalize(mul(float4(In.Normal, 0.0f), World).xyz);
    Out.Normal = float4(worldNormal, 1.0f);
    
    // SV_TARGET2: World Position
    Out.WorldPosition = float4(In.WorldPos, 1.0f);
    
    //blenderからのエミッションはマテリアルじゃなくbloomColorに
    Out.BloomContrib = bloomColor.Color;
    
   
    return Out;
}
