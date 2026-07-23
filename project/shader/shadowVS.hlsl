#include "common.hlsl"

VS_SHADOW_OUTPUT main(VS_IN In)
{
    VS_SHADOW_OUTPUT Out;
    
    matrix wvp;
    wvp = mul(World, View);
    wvp = mul(wvp, Projection);

    float4 pos = float4(In.Position.xyz, 1.0f);
	
    Out.Position = mul(pos, wvp);
    return Out;
}
