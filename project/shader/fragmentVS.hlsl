#include "common.hlsl" // View, Projection行列を使うため

// スロット0: Per-Vertex Data (板ポリゴンの頂点)
struct VS_INPUT_VERTEX
{
    float3 position : POSITION0;
    float2 uv : TEXCOORD0;
};

// スロット1: Per-Instance Data (各パーティクルの情報)
struct VS_INPUT_INSTANCE
{
    float3 instancePos : INSTANCE_POS0;
    float3 instanceVel : INSTANCE_VEL0;
    float instanceLife : INSTANCE_LIFE0;
    float instanceAge : INSTANCE_AGE0;
    float instanceSize : INSTANCE_SIZE;
    float instanceRotaion : INSTANCE_ROT;
    float instanceAngularVelocity : INSTANCE_AVEL;
    uint IsActive     :  INSTANCE_ACTIVE;
};

PS_IN main(VS_INPUT_VERTEX vertex, VS_INPUT_INSTANCE instance, uint instanceID : SV_InstanceID)
{
    PS_IN output;
     // --- 2D回転行列を作成 ---
    float angle = instance.instanceRotaion; //インスタンスごとの角度を受け取る
    float s = sin(angle);
    float c = cos(angle);
    float2x2 rotMat = float2x2(c, -s, s, c);

    // --- 板ポリゴンの頂点座標を回転させる ---
    float2 rotatedPos = mul(vertex.position.xy, rotMat);
    
    // --- ビルボード計算 ---
    float3 camRight = float3(View._11, View._21, View._31);
    float3 camUp = float3(View._12, View._22, View._32);
    
    float3 finalPos = instance.instancePos;
    finalPos += rotatedPos.x * camRight * instance.instanceSize;
    finalPos += rotatedPos.y * camUp * instance.instanceSize;
    
    
    output.Position = mul(float4(finalPos, 1.0f), mul(View, Projection));
    output.TexCoord = vertex.uv;
    output.Diffuse = float4(1, 1, 1, instance.instanceLife); // 寿命をアルファとして渡す
    output.UniqueID = instanceID;

    return output;
}
