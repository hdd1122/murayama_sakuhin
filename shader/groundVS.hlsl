#include "common.hlsl"

void main(in VS_IN In, out PS_IN Out)
{
    // ワールド座標を計算
    Out.WorldPos = mul(float4(In.Position.xyz, 1.0f), World).xyz;

	// WVP変換（スクリーン座標の計算）
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);
    Out.Position = mul(float4(In.Position.xyz, 1.0f), wvp);
    
    Out.Normal = mul(float4(In.Normal.rgb, 0.0f), World).xyz;
    
    Out.TexCoord = In.TexCoord;
    Out.Diffuse = In.Diffuse * Material.Diffuse;
}
