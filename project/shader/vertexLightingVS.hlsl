
#include "common.hlsl"


void main(in VS_IN In, out PS_IN Out)
{
	
	
	 // ワールド座標を計算
    Out.WorldPos = mul(float4(In.Position.xyz, 1.0f), World).xyz;
	
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    float4 worldNormal, normal;
    normal = float4(In.Normal.xyz, 0.0f);
    worldNormal = mul(normal, World);
    worldNormal = normalize(worldNormal);
    
    float3 lightDirection = normalize(Light.Direction.xyz);
    float light = -dot(lightDirection, worldNormal.xyz) * 0.5;
    light = saturate(light);
    
    Out.Diffuse = In.Diffuse * Material.Diffuse * light * Light.Diffuse;
    Out.Diffuse += In.Diffuse * Material.Ambient * Light.Ambient;
    Out.Diffuse += Material.Emission;
    Out.Diffuse.a = In.Diffuse.a * Material.Diffuse.a;
    
    
    float4 pos = float4(In.Position.xyz, 1.0f);
	
    Out.Position = mul(pos, wvp);
    Out.TexCoord = In.TexCoord;
    Out.Normal = In.Normal;
	

}

