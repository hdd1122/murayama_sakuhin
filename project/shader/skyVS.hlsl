//
#include "common.hlsl"


void main(in VS_IN In, out PS_IN Out)
{
	
	
	 // ワールド座標を計算
    Out.WorldPos = mul(float4(In.Position.xyz, 1.0f), World).xyz;
	
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    float4 pos = float4(In.Position.xyz, 1.0f);
	
    Out.Position = mul(pos, wvp);
    Out.Position.z = Out.Position.w;
    
    Out.TexCoord = In.TexCoord;
    Out.Diffuse = In.Diffuse * Material.Diffuse;
	
    Out.Normal = In.Normal;
	//
    //Out.Depth = Out.Position.z / Out.Position.w;
    Out.Depth = Out.Position.z;

}

