#include "common.hlsl"
//SDFレイマーチの関数とかまとめ


float smoothMin(float a, float b, float w)
{
    const float x = (b - a) / w;
    return x <= -1 ? b : x >= 1 ? a : b - w * 0.25 * (x + 1) * (x + 1);
}
//GLSLのmodらしい
float GLmod(float x, float y)
{
    return x - y * floor(x / y);
}
float2 GLmod(float2 x, float2 y)
{
    return x - y * floor(x / y);
}
float3 GLmod(float3 x, float3 y)
{
    return x - y * floor(x / y);
}
float4 GLmod(float4 x, float4 y)
{
    return x - y * floor(x / y);
}

// 深度値をカメラからの距離に変換する関数
float LinearizeDepth(float depth)
{
    // g_projectionMatrixを定数バッファで渡す必要がある
    return Projection._43 / (depth - Projection._33);
    
}


// pをY軸周りに-angleラジアン回転させるヘルパー関数
float3 rotateY(float3 p, float angle)
{
    float s = sin(-angle);
    float c = cos(-angle);
    return float3(
        p.x * c - p.z * s,
        p.y,
        p.x * s + p.z * c
    );
}

float calcSoftShadow(float3 ro, float3 rd, float maxDist);

  // 二倍に拡大とかこんな感じ
//float scale = 2.0;
    //return sdf_sphere(p / scale, 1.0) * scale;


float3 onRep(float3 p, float3 interval)
{
    // 1. 座標を2倍して、剰余を計算し、最後に半分に戻す
    //    これにより、-interval/2 ～ +interval/2 の範囲で繰り返される
    return GLmod(p + interval * 0.5f, interval) - interval * 0.5f;
}

float2 onRep2(float2 p, float2 interval)
{
    return GLmod(p + interval * 0.5f, interval) - interval * 0.5f;
}

// SDF (Signed Distance Function): ある点pから、最も近い球体表面までの距離を返す
float sdf_sphere(float3 p, float radius)
{
    return length(p) - radius;

}

float sdf_sphere2(float3 p, float interval, float radius)
{
    
    return length(onRep(p, interval)) - radius;
}

float sdf_box(float3 p, float3 size)
{
    return length(max(abs(p) - size, 0.0));
}

float sdf_bar(float2 p, float width)
{
    float2 size = float2(width, width); // 正方形のサイズ
    float2 d = abs(p) - size;
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
}

//鉄骨用
float berDist(float2 p, float interval, float width)
{
    //return length(max(abs(onRep2(p,interval)) - width, 0.0));
    return length(max(abs(onRep2(p, interval)) - width, 0.0));
}

float tubeDist(float2 p, float interval, float width)
{
    return length(onRep2(p, interval)) - width;
}



float sdCross(float3 p, float c)
{
    p = abs(p);
    float dxy = max(p.x, p.y);
    float dyz = max(p.y, p.z);
    float dxz = max(p.x, p.z);
    return min(dxy, min(dyz, dxz)) - c;
}

float sdBox(float3 p, float3 b)
{
    p = abs(p) - b;
    return length(max(p, 0.0)) + min(max(p.x, max(p.y, p.z)), 0.0);
}

//#define ITERATIONS 5
//float deMengerSponge1(float3 p, float scale, float width)
//{
//    float d = sdBox(p, float3(1.0,1.0,1.0));
//    float s = 1.0;
//    for (int i = 0; i < ITERATIONS; i++)
//    {
//        float3 a = fmod(p * s, 2.0) - 1.0;
//        s *= scale;
//        float3 r = 1.0 - scale * abs(a);
//        float c = sdCross(r, width) / s;
//        d = max(d, c);
//    }
//    return d;
//}

//
float testmap1(float3 p, float time)
{
    p.x -= 4.0;
    p.z += time * 3.0;
    p = fmod(p, 8.0) - 4.0;
    for (int j = 0; j < 3; j++)
    {
        p.xy = abs(p.xy) - 0.3;
        p.yz = abs(p.yz) - sin(time * 2.0) * 0.3 + 0.1,
     p.xz = abs(p.xz) - 0.2;
    }
    return length(cross(p, float3(0.5, 0.5, 0.5))) - 0.1;
}

float3 foldX(float3 p)
{
    p.x = abs(p.x);
    return p;
}

float sdBoxFrame(float3 p, float3 b, float e)
{
    p = onRep(p, 10);
    p = abs(p) - b;
    float3 q = abs(p + e) - e;

    return min(min(
      length(max(float3(p.x, q.y, q.z), 0.0)) + min(max(p.x, max(q.y, q.z)), 0.0),
      length(max(float3(q.x, p.y, q.z), 0.0)) + min(max(q.x, max(p.y, q.z)), 0.0)),
      length(max(float3(q.x, q.y, p.z), 0.0)) + min(max(q.x, max(q.y, p.z)), 0.0));
}

float sdTorus(float3 p, float2 t)
{
    float2 q = float2(length(p.x) - t.x, p.y);
    return length(q) - t.y;
}

#define ITERATIONS 8
float dePseudoKleinian(float3 p)
{

    
    float3 csize = float3(0.90756, 0.92436, 0.90756);
    float size = 1.0;
    float3 c = float3(0.0, 0.0, 0.0);
    float defactor = 1.0;
    float3 offset = float3(0.0, 0.0, 0.0);
    float3 ap = p + 1.0;
    for (int i = 0; i < ITERATIONS; i++)
    {
        ap = p;
        p = 2.0 * clamp(p, -csize, csize) - p;
        float r2 = dot(p, p);
        float k = max(size / r2, 1.0);
        p *= k;
        defactor *= k;
        p += c;
    }
    float r = abs(0.5 * abs(p.y - offset.y) / defactor);
    return r;
}


// シーン全体のSDF: ここにオブジェクトを配置していく
float map_scene(float3 p)
{
    
    
    //float3 n = (1, 0, 0);
    //n = normalize(n);
    //p -= 2.0 * min(0.0, dot(p, n)) * n;
   
    float scale = 1.0;
    
    float size = 50;
    //float size = 50;
    
    float objPos = 50;
    //float objPos = 50;
    //float size = 12.6;
    float t = (sin(r_time * 0.5f) + 1.0f) * 0.5f;
    //float t = r_time;
    float3 nor = (1, 0, 0);
    nor = normalize(nor);
    
    
    //p = foldX(p);
   // t = r_dummy;
    
    // 中心(0,0,0)に半径1.0の球を1つだけ配置
   // return sdf_sphere(p, 1.0f);
    //return sdf_box(p, float3(1.0f,1.0f,1.0f));
   // return min(sdf_sphere(p, 1.2f), sdf_box(p, float3(1.0f, 1.0f, 1.0f)));
    
   //return sdf_sphere(onRep(p, float3(size,size,size)), 0.1);
    
    //丸ループ
    //return sdf_sphere(onRep(p, float3(size, size, size)), 2.0);
    
    
    //鉄骨的なやつ
        //rubeの引数のインターバルとサイズがなんか資料と違った
        //資料はhlslじゃなからっぽい
    float barSize = 3;
    
    float3 scaleP = p / scale;
    float barX = berDist(scaleP.yz, 1.0 * barSize, 0.1 * barSize);
    float barY = berDist(scaleP.xz, 1.0 * barSize, 0.1 * barSize);
    float barZ = berDist(scaleP.xy, 1.0 * barSize, 0.1 * barSize);
    
    float tubeX = tubeDist(scaleP.yz, 0.2 * barSize, 0.05 * barSize);
    float tubeY = tubeDist(scaleP.xz, 0.2 * barSize, 0.05 * barSize);
    float tubeZ = tubeDist(scaleP.xy, 0.2 * barSize, 0.05 * barSize);
    
    float kekka = max(max(max(min(min(
    barX, barY), barZ),
    -tubeX), -tubeY), -tubeZ) * scale;
    
    float hanni = sdf_box(p - float3(objPos, objPos, objPos), float3(size, size, size));
    //float hanni = sdf_box(p - float3(12.5, 12.5, 12.5), float3(size, size, size));

    //return hanni;
    //鉄骨結果
    //return max(kekka, hanni);
    //鉄骨結果変数
    float mazikekka = max(kekka, hanni);
    
    //
    float bar = sdf_bar(p.xy - float2(50, 19), 20.0);
    float kekkaCut = max(mazikekka, -bar);
    //切り抜き結果
    //return lerp(kekkaCut, mazikekka, t);
    
    //return max(dePseudoKleinian(p),hanni);
    
 
    float3 p2 = rotateY(scaleP, t);
    
    float barXr = berDist(p2.yz, 1.0, 0.1);
    float barYr = berDist(p2.xz, 1.0, 0.1);
    float barZr = berDist(p2.xy, 1.0, 0.1);
    
    float tubeXr = tubeDist(p2.yz, 0.2, 0.05);
    float tubeYr = tubeDist(p2.xz, 0.2, 0.05);
    float tubeZr = tubeDist(p2.xy, 0.2, 0.05);
    
  
    
    float kekkar = max(max(max(min(min(
    barXr, barYr), barZr),
    -tubeXr), -tubeYr), -tubeZr);
    
    float hannir = sdf_box(p2 - float3(objPos, objPos, objPos), float3(size, size, size));

    //return max(kekkar, hannir);
    float mazikekkar = max(kekkar, hannir);
    
    //float Cut2 = sdf_sphere2(p, 10, 3);
    float Cut2 = sdf_sphere(p - zone.PlayerPosition.rgb, 10);
    //Cut2 = max(Cut2, hanni);
  
    //return smoothMin(mazikekka, Cut2, 5);
    
    //return max(Cut2,hanni);
    //return lerp(mazikekkar, Cut2, t);
    //return max(mazikekka, -Cut2);
    //return max(mazikekka, mazikekkar);

    //return deMengerSponge1(p, 3, 5);
    
    //return testmap1(p, t);
    
    return lerp(mazikekka, max(sdBoxFrame(p2, float3(5, 5, 5), 0.2), hanni), r_time);

    //return sdTorus(p, float2(5, 5 * r_dummy));

}
