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
    float instanceAge  : INSTANCE_AGE0;
    float instanceSize : INSTANCE_SIZE;
    float instanceRotaion : INSTANCE_ROT;
};

PS_IN main(VS_INPUT_VERTEX vertex, VS_INPUT_INSTANCE instance)
{
    PS_IN output;
    
    // --- ビルボード計算 ---
    float3 particleCenter = instance.instancePos;
    
    // カメラの右ベクトルと上ベクトルをビュー行列から取得
    float3 camRight = float3(View._11, View._21, View._31);
    float3 camUp = float3(View._12, View._22, View._32);
    
    // 板ポリゴンの頂点座標を、カメラ基準で回転させる
    float3 finalPos = particleCenter;
    finalPos += vertex.position.x * camRight * instance.instanceSize; // ★サイズを適用
    finalPos += vertex.position.y * camUp * instance.instanceSize; // ★サイズを適用
    
    
    // 最終的なスクリーン座標を計算
    output.Position = mul(float4(finalPos, 1.0f), mul(View, Projection));
    
    // ピクセルシェーダーに渡す情報
    output.TexCoord = vertex.uv;
    output.Diffuse = float4(1, 1, 1, instance.instanceLife); // 例: 寿命でアルファを変化

    return output;
 
}
