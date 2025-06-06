/*
Copyright (c) 2003-2006 Gino van den Bergen / Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/



#ifndef AABB_UTIL2
#define AABB_UTIL2

#include "btVector3.h"
#include "btMinMax.h"

SIMD_FORCE_INLINE void AabbExpand (btVector3& aabbMin,
                                   btVector3& aabbMax,
                                   const btVector3& expansionMin,
                                   const btVector3& expansionMax)
{
    aabbMin = aabbMin + expansionMin;
    aabbMax = aabbMax + expansionMax;
}


/// conservative test for overlap between two aabbs
SIMD_FORCE_INLINE bool TestAabbAgainstAabb2(const btVector3 &aabbMin1, const btVector3 &aabbMax1,
                                const btVector3 &aabbMin2, const btVector3 &aabbMax2)
{
    bool overlap = true;
    overlap = (aabbMin1[0] > aabbMax2[0] || aabbMax1[0] < aabbMin2[0]) ? false : overlap;
    overlap = (aabbMin1[2] > aabbMax2[2] || aabbMax1[2] < aabbMin2[2]) ? false : overlap;
    overlap = (aabbMin1[1] > aabbMax2[1] || aabbMax1[1] < aabbMin2[1]) ? false : overlap;
    return overlap;
}

/// conservative test for overlap between triangle and aabb
SIMD_FORCE_INLINE bool TestTriangleAgainstAabb2(const btVector3 *vertices,
                                    const btVector3 &aabbMin, const btVector3 &aabbMax)
{
    const btVector3 &p1 = vertices[0];
    const btVector3 &p2 = vertices[1];
    const btVector3 &p3 = vertices[2];

    if (btMin(btMin(p1[0], p2[0]), p3[0]) > aabbMax[0]) return false;
    if (btMax(btMax(p1[0], p2[0]), p3[0]) < aabbMin[0]) return false;

    if (btMin(btMin(p1[2], p2[2]), p3[2]) > aabbMax[2]) return false;
    if (btMax(btMax(p1[2], p2[2]), p3[2]) < aabbMin[2]) return false;
  
    if (btMin(btMin(p1[1], p2[1]), p3[1]) > aabbMax[1]) return false;
    if (btMax(btMax(p1[1], p2[1]), p3[1]) < aabbMin[1]) return false;
    return true;
}


SIMD_FORCE_INLINE int    btOutcode(const btVector3& p,const btVector3& halfExtent) 
{
    return (p.getX()  < -halfExtent.getX() ? 0x01 : 0x0) |    
           (p.getX() >  halfExtent.getX() ? 0x08 : 0x0) |
           (p.getY() < -halfExtent.getY() ? 0x02 : 0x0) |    
           (p.getY() >  halfExtent.getY() ? 0x10 : 0x0) |
           (p.getZ() < -halfExtent.getZ() ? 0x4 : 0x0) |    
           (p.getZ() >  halfExtent.getZ() ? 0x20 : 0x0);
}


SIMD_FORCE_INLINE bool btRayAabb2(const btVector3& rayFrom,
                                  const btVector3& rayInvDirection,
                                  const unsigned int raySign[3],
                                  const btVector3 bounds[2],
                                  btScalar& tmin,
                                  btScalar lambda_min,
                                  btScalar lambda_max)
{
    btScalar tmax, tymin, tymax, tzmin, tzmax;
    tmin = (bounds[raySign[0]][0] - rayFrom[0]) * rayInvDirection[0];
    tmax = (bounds[1-raySign[0]][0] - rayFrom[0]) * rayInvDirection[0];
    tymin = (bounds[raySign[1]][1] - rayFrom[1]) * rayInvDirection[1];
    tymax = (bounds[1-raySign[1]][1] - rayFrom[1]) * rayInvDirection[1];

    if ( (tmin > tymax) || (tymin > tmax) )
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    tzmin = (bounds[raySign[2]][2] - rayFrom[2]) * rayInvDirection[2];
    tzmax = (bounds[1-raySign[2]][2] - rayFrom[2]) * rayInvDirection[2];

    if ( (tmin > tzmax) || (tzmin > tmax) )
        return false;
    if (tzmin > tmin)
        tmin = tzmin;
    if (tzmax < tmax)
        tmax = tzmax;
    return ( (tmin < lambda_max) && (tmax > lambda_min) );
}

SIMD_FORCE_INLINE bool btRayAabb(const btVector3& rayFrom, 
                                 const btVector3& rayTo, 
                                 const btVector3& aabbMin, 
                                 const btVector3& aabbMax,
                      btScalar& param, btVector3& normal) 
{
    btVector3 aabbHalfExtent = (aabbMax-aabbMin)* btScalar(0.5);
    btVector3 aabbCenter = (aabbMax+aabbMin)* btScalar(0.5);
    btVector3    source = rayFrom - aabbCenter;
    btVector3    target = rayTo - aabbCenter;
    int    sourceOutcode = btOutcode(source,aabbHalfExtent);
    int targetOutcode = btOutcode(target,aabbHalfExtent);
    if ((sourceOutcode & targetOutcode) == 0x0)
    {
        btScalar lambda_enter = btScalar(0.0);
        btScalar lambda_exit  = param;
        btVector3 r = target - source;
        int i;
        btScalar    normSign = 1;
        btVector3    hitNormal(0,0,0);
        int bit=1;

        for (int j=0;j<2;j++)
        {
            for (i = 0; i != 3; ++i)
            {
                if (sourceOutcode & bit)
                {
                    btScalar lambda = (-source[i] - aabbHalfExtent[i]*normSign) / r[i];
                    if (lambda_enter <= lambda)
                    {
                        lambda_enter = lambda;
                        hitNormal.setValue(0,0,0);
                        hitNormal[i] = normSign;
                    }
                }
                else if (targetOutcode & bit) 
                {
                    btScalar lambda = (-source[i] - aabbHalfExtent[i]*normSign) / r[i];
                    btSetMin(lambda_exit, lambda);
                }
                bit<<=1;
            }
            normSign = btScalar(-1.);
        }
        if (lambda_enter <= lambda_exit)
        {
            param = lambda_enter;
            normal = hitNormal;
            return true;
        }
    }
    return false;
}


#endif


