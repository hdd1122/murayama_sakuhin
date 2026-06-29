#include "common.hlsl"

struct PointLightData
{
    float3 Position; // 中心座標
    float Range; // 半径
    float3 Color; // 色
    float Intensity; // 強さ
};

StructuredBuffer<PointLightData> g_PointLightList : register(t11);

//ポイントライト用
struct P_VS_IN
{
 // C++の CreateVertexShader の定義順に合わせる
    float3 Position : POSITION;
    //これらはいらないが、頂点レイアウトに含まれているので定義しておく
    float3 Normal : NORMAL; 
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD;

    // インスタンシング用
    uint InstanceID : SV_InstanceID;
};

struct P_PS_IN
{
    float4 Position : SV_POSITION;
    
    // PSへ渡すデータ 補間しない
    nointerpolation float3 LightPos : TEXCOORD1;
    nointerpolation float LightRange : TEXCOORD2;
    nointerpolation float3 LightColor : TEXCOORD3; // 色 * 強さ
};

P_PS_IN main(P_VS_IN input)
{
    P_PS_IN output;

    // バッファからこのインスタンス用のライト情報を取得
    PointLightData data = g_PointLightList[input.InstanceID];

    // ワールド座標の手動計算
    // モデルは原点中心、半径1.0
    
    // モデルの見た目サイズを少し大きくする
    // モデルの内部のみ計算したいが同じ大きさだと形状がモデルの物になってしまうため
    float meshScale = data.Range * 1.2f;
    
    // 拡大
    float3 localPos = input.Position.xyz * meshScale;
    
    // 移動・ライトの位置へ
    float4 worldPos = float4(localPos + data.Position, 1.0);

    // 座標変換
    matrix vp = mul(View, Projection);
    output.Position = mul(worldPos, vp);

    // PSへライト情報をパススルー
    output.LightPos = data.Position;
    output.LightRange = data.Range;//ここはそのまま渡す
    // PSでの計算を減らすため、ここで強度を掛けておく
    output.LightColor = data.Color * data.Intensity;

    return output;
}
