
#include "common.hlsl"
//現在メッシュフィールド専用

Texture2D g_Texture : register(t0);
Texture2D g_TextureSand : register(t1);
SamplerState g_SamplerState : register(s0);

PS_G_OUTPUT main(in PS_IN In)
{
    PS_G_OUTPUT Out;
    float4 finalColor;

    if (Material.TextureEnable)
    {
        finalColor = g_Texture.Sample(g_SamplerState, In.TexCoord)*In.Diffuse.a;
        finalColor += g_TextureSand.Sample(g_SamplerState, In.TexCoord) * (1.0 - In.Diffuse.a);
        
        finalColor *= In.Diffuse;
    }
    else
    {
        finalColor = In.Diffuse;
    }
    finalColor.a = 1.0;

     //アルファクリップ
    clip(finalColor.a - 0.01);
    
    if (Material.Emission.x > 0.0)//これ設定してないから後でマテリアルを設定するように変更する
    {
        //finalColor.rgb *= Material.Emission.x;
    }
 
    Out.SceneColor = finalColor;
    // In.Normalはローカル空間なので、ワールド行列で変換
    float3 worldNormal = normalize(mul(float4(In.Normal, 0.0f), World).xyz);
    Out.Normal = float4(worldNormal, 1.0f);
    
    // SV_TARGET2: World Position
    Out.WorldPosition = float4(In.WorldPos, 1.0f);
    return Out;
}
