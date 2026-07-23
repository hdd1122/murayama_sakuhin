#include "main.h"
#include "collision.h"
#include "obb.h"

//判定関連

// OBBを指定された軸に射影し、その区間を返す
Interval Collision::GetInterval(const Obb& obb, const Vector3& axis)
{
    // OBBの3つの回転軸を取得
    Vector3 rot[3];
    XMStoreFloat3((XMFLOAT3*)&rot[0], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[0]));
    XMStoreFloat3((XMFLOAT3*)&rot[1], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[1]));
    XMStoreFloat3((XMFLOAT3*)&rot[2], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[2]));

    // OBBの中心を軸に射影
    float center = Vector3::Dot(obb.Center, axis);

    // OBBの各軸方向の半径を、分離軸に射影した長さを計算し、合計する
    float radius =
        obb.Extents.x * fabs(Vector3::Dot(axis, rot[0])) +
        obb.Extents.y * fabs(Vector3::Dot(axis, rot[1])) +
        obb.Extents.z * fabs(Vector3::Dot(axis, rot[2]));

    // 中心から半径を引いた値が最小値、足した値が最大値
    Interval result;
    result.min = center - radius;
    result.max = center + radius;

    return result;
}

// 2つの区間(Interval)が重なっているかを判定
bool Collision::IsOverlapping(const Interval& intervalA, const Interval& intervalB)
{
    return intervalA.max >= intervalB.min && intervalB.max >= intervalA.min;
}

CollisionResult Collision::CheckCollision_OBB_OBB(const Obb& obbA, const Obb& obbB)
{
    CollisionResult result;
    float minOverlap = FLT_MAX;

    // 分離軸の候補を格納する配列
    Vector3 testAxes[15];
    int axisCount = 0;

    // 分離軸の候補を計算
   // XMVECTORとして回転軸をロード
    XMVECTOR rotA_v[3] = {
        XMLoadFloat3((XMFLOAT3*)&obbA.Rotation.m[0]),
        XMLoadFloat3((XMFLOAT3*)&obbA.Rotation.m[1]),
        XMLoadFloat3((XMFLOAT3*)&obbA.Rotation.m[2])
    };
    XMVECTOR rotB_v[3] = {
        XMLoadFloat3((XMFLOAT3*)&obbB.Rotation.m[0]),
        XMLoadFloat3((XMFLOAT3*)&obbB.Rotation.m[1]),
        XMLoadFloat3((XMFLOAT3*)&obbB.Rotation.m[2])
    };

    // Vector3に変換して格納
    Vector3 rotA[3], rotB[3];
    for (int i = 0; i < 3; ++i) XMStoreFloat3((XMFLOAT3*)&rotA[i], rotA_v[i]);
    for (int i = 0; i < 3; ++i) XMStoreFloat3((XMFLOAT3*)&rotB[i], rotB_v[i]);


    // 軸候補1：OBB Aの3軸
    for (int i = 0; i < 3; ++i) testAxes[axisCount++] = rotA[i];

    // 軸候補2：OBB Bの3軸
    for (int i = 0; i < 3; ++i) testAxes[axisCount++] = rotB[i];

    // 軸候補3：AとBの軸の外積 (9本)
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            //計算はXMVECTORで行う
            XMVECTOR crossProduct_v = XMVector3Cross(rotA_v[i], rotB_v[j]);

            if (XMVectorGetX(XMVector3LengthSq(crossProduct_v)) > 1e-6) {
                //配列に代入する前にVector3に変換する
                XMStoreFloat3((XMFLOAT3*)&testAxes[axisCount++], XMVector3Normalize(crossProduct_v));
            }
        }
    }

    // --- 各分離軸でテスト ---
    for (int i = 0; i < axisCount; ++i)
    {
        Interval intervalA = GetInterval(obbA, testAxes[i]);
        Interval intervalB = GetInterval(obbB, testAxes[i]);

        if (!IsOverlapping(intervalA, intervalB))
        {
            // 1つでも分離している軸があれば、衝突していない
            result.isColliding = false;
            return result;
        }
        else
        {
            float overlap = 0.0f;

           
            // Aの最大値とBの最大値のうち、小さい方を取得
            float minMax = std::min(intervalA.max, intervalB.max);

            // Aの最小値とBの最小値のうち、大きい方を取得
            float maxMin = std::max(intervalA.min, intervalB.min);

            // 2つの差が、重なっている部分の長さ
            overlap = minMax - maxMin;

            if (overlap < minOverlap)
            {
                // これまでで最も重なりが小さい軸をMTVとして記録
                minOverlap = overlap;
                result.mtv = testAxes[i] * overlap;

                // 2つのOBBの中心を結ぶベクトルを計算
                Vector3 diff = obbA.Center - obbB.Center;

                // 分離軸と中心ベクトルの内積を計算
                if (Vector3::Dot(diff, testAxes[i]) < 0)
                {
                    // 向きが逆の場合、分離軸を反転させる
                    result.mtv = testAxes[i] * -minOverlap;
                }
                else
                {
                    // 向きが正しい場合、そのまま
                    result.mtv = testAxes[i] * minOverlap;
                }
            }
            

        }
    }

    // 全ての軸で重なっていた場合、衝突している
    result.isColliding = true;
    return result;
}


CollisionResult Collision::CheckCollision_OBB_Sphere(const Obb& obb, const Sphere& sphere)
{
    CollisionResult result;
    result.isColliding = false;

    // OBBの3つの回転軸を取得
    Vector3 rot[3];
    XMStoreFloat3((XMFLOAT3*)&rot[0], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[0]));
    XMStoreFloat3((XMFLOAT3*)&rot[1], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[1]));
    XMStoreFloat3((XMFLOAT3*)&rot[2], XMLoadFloat3((XMFLOAT3*)&obb.Rotation.m[2]));

    // OBBの中心から球の中心へのベクトル
    Vector3 v = sphere.Center - obb.Center;

    // OBBのローカル空間における、球の中心に最も近い点を計算
    Vector3 closestPoint = obb.Center;

    for (int i = 0; i < 3; ++i)
    {
        // 球の中心をOBBの各軸に射影した距離
        float dist = Vector3::Dot(v, rot[i]);

        // OBBの範囲内（Extents）にクランプする
        float extent = (i == 0) ? obb.Extents.x : (i == 1) ? obb.Extents.y : obb.Extents.z;
        if (dist > extent) dist = extent;
        if (dist < -extent) dist = -extent;

        // クランプした距離を軸方向に足して、最近傍点を進める
        closestPoint += rot[i] * dist;
    }

    // 最近傍点から球の中心へ向かうベクトル
    Vector3 diff = sphere.Center - closestPoint;
    float sqDist = Vector3::Dot(diff, diff);

    // 距離の二乗が、球の半径の二乗以下なら衝突している
    if (sqDist <= sphere.Radius * sphere.Radius && sqDist > 0.0001f)
    {
        result.isColliding = true;

        float dist = sqrtf(sqDist);
        Vector3 normal = diff / dist; // 正規化された法線
        float penetration = sphere.Radius - dist; // めり込み量

        // 球をOBBの外へ押し出すベクトル
        result.mtv = normal * penetration;
    }
 
    return result;
}
