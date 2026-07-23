
// common.hlsl

#ifndef __COMMON_HLSL__ // もしこの名前が定義されていなければ...
#define __COMMON_HLSL__ // 定義する！



cbuffer WorldBuffer : register(b0)
{
	matrix World;
}
cbuffer ViewBuffer : register(b1)
{
	matrix View;
}
cbuffer ProjectionBuffer : register(b2)
{
	matrix Projection;
}




struct MATERIAL
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Emission;
	float Shininess;
	bool TextureEnable;
	float2 Dummy;
};

cbuffer MaterialBuffer : register(b3)
{
	MATERIAL Material;
}




struct LIGHT
{
	bool Enable;
    bool CastsShadows;
	bool2 Dummy;
	float4 Direction;
	float4 Diffuse;
	float4 Ambient;
};

cbuffer LightBuffer : register(b4)
{
	LIGHT Light;
}


struct ZONE
{
    float4 CameraPosition;
    float TotalTime;
    float DeltaTime; // 4バイト
    bool2 Dummy;
    float4 PlayerPosition;
    float4 PlayerVelocity;
};


cbuffer ZoneBuffer : register(b5)
{
    ZONE zone;
}

//6.7

cbuffer CompositeParams : register(b7)
{
    float intensityA;
    float intensityB;
    bool2 dummy;
};


struct BloomColor
{
    float4 Color;
};

cbuffer BloomColor : register(b8)
{
    BloomColor bloomColor;
}


// C++からカメラ情報を受け取るための定数バッファ
cbuffer RaymarchingParams : register(b9)
{
    float3 r_cameraPosition;
    float r_time;
    float3 r_cameraForward;
    float r_screenWidth;
    float3 r_cameraRight;
    float r_screenHeight;
    float3 r_cameraUp;
    float r_dummy;
    matrix r_inverseViewProjection;
    matrix r_inverseProjection;
};


cbuffer CsmParams : register(b10)
{
    matrix matCascadeLightVP[3]; // CASCADE_COUNT
    float4 CascadeSplits; // (x, y, z) に分割深度が入る
};


//13は雲で使ってる


struct VS_IN
{
	float3 Position		: POSITION0;
	float3 Normal		: NORMAL0;
	float4 Diffuse		: COLOR0;
	float2 TexCoord		: TEXCOORD0;
};


struct PS_IN
{
	float4 Position		: SV_POSITION;
	float4 Diffuse		: COLOR0;
	float2 TexCoord		: TEXCOORD0;
    float3 WorldPos : TEXCOORD1;
    float3 Normal : TEXCOORD2;
    uint  UniqueID : TEXCOORD3;
    float Depth : DEPTH0;
};

struct PS_G_OUTPUT
{
    float4 SceneColor : SV_TARGET0; // 描画先0
    float4 Normal : SV_TARGET1; // 描画先1
    float4 WorldPosition : SV_TARGET2; // 描画先2
    float4 BloomContrib : SV_TARGET3;
};

struct PS_OUTPUT
{
    float4 SceneColor : SV_TARGET0; // 描画先0
    float4 BloomContrib : SV_TARGET1; //
};

struct VS_SHADOW_OUTPUT
{
    float4 Position : SV_POSITION;
};




// シーン全体のSDF（SDFベースは現在利用していない）
float map_sceneTest(float3 p)
{
 
    
    return length(p) - 10;
}

// z はカメラからの直線距離（w値）
float CalculateWeight(float z, float alpha)
{
    // 不透明度が高いほど重く
    float a = alpha;
    // 距離 z に基づく重み (McGuireの式の一種)
    // 距離が近いほど重いが、極端に爆発しないように max でガード
    float b = 1.0 - z * 0.001; // シーンの遠さに合わせて調整（0.001：1000mで0になる）
    b = saturate(b);
    
    return a * max(1e-2, 1e2 * pow(b, 3.0));
}
#endif // __COMMON_HLSL__ 終わり
