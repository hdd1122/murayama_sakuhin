#pragma once

struct Obb;

struct Sphere {
    Vector3 Center;
    float Radius;
};

struct CollisionResult
{
    bool isColliding = false;
    Vector3 mtv; //最小押し出しベクトル
};
//
struct Interval
{
    float min;
    float max;
};

class Collision
{


public:
    static Interval GetInterval(const Obb& obb, const Vector3& axis);
    static bool IsOverlapping(const Interval& intervalA, const Interval& intervalB);
	static CollisionResult CheckCollision_OBB_OBB(const Obb& obbA, const Obb& obbB);
    static CollisionResult CheckCollision_OBB_Sphere(const Obb& obb, const Sphere& sphere);
};

