/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "btCylinderShape.h"
#include "LinearMath/btPoint3.h"

btCylinderShape::btCylinderShape (const btVector3& halfExtents)
:btBoxShape(halfExtents),
m_upAxis(1)
{
    recalcLocalAabb();
}


btCylinderShapeX::btCylinderShapeX (const btVector3& halfExtents)
:btCylinderShape(halfExtents)
{
    m_upAxis = 0;
    recalcLocalAabb();
}


btCylinderShapeZ::btCylinderShapeZ (const btVector3& halfExtents)
:btCylinderShape(halfExtents)
{
    m_upAxis = 2;
    recalcLocalAabb();
}

void btCylinderShape::getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const
{
    //skip the box 'getAabb'
    btPolyhedralConvexShape::getAabb(t,aabbMin,aabbMax);
}


SIMD_FORCE_INLINE btVector3 CylinderLocalSupportX(const btVector3& halfExtents,const btVector3& v) 
{
const int cylinderUpAxis = 0;
const int XX = 1;
const int YY = 0;
const int ZZ = 2;

    //mapping depends on how cylinder local orientation is
    // extents of the cylinder is: X,Y is for radius, and Z for height


    btScalar radius = halfExtents[XX];
    btScalar halfHeight = halfExtents[cylinderUpAxis];


    btVector3 tmp;
    btScalar d ;

    btScalar s = btSqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
    if (s != btScalar(0.0))
    {
        d = radius / s;  
        tmp[XX] = v[XX] * d;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = v[ZZ] * d;
        return tmp;
    }
    else
    {
        tmp[XX] = radius;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = btScalar(0.0);
        return tmp;
    }


}






inline  btVector3 CylinderLocalSupportY(const btVector3& halfExtents,const btVector3& v) 
{

const int cylinderUpAxis = 1;
const int XX = 0;
const int YY = 1;
const int ZZ = 2;


    btScalar radius = halfExtents[XX];
    btScalar halfHeight = halfExtents[cylinderUpAxis];


    btVector3 tmp;
    btScalar d ;

    btScalar s = btSqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
    if (s != btScalar(0.0))
    {
        d = radius / s;  
        tmp[XX] = v[XX] * d;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = v[ZZ] * d;
        return tmp;
    }
    else
    {
        tmp[XX] = radius;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = btScalar(0.0);
        return tmp;
    }

}

inline btVector3 CylinderLocalSupportZ(const btVector3& halfExtents,const btVector3& v) 
{
const int cylinderUpAxis = 2;
const int XX = 0;
const int YY = 2;
const int ZZ = 1;

    //mapping depends on how cylinder local orientation is
    // extents of the cylinder is: X,Y is for radius, and Z for height


    btScalar radius = halfExtents[XX];
    btScalar halfHeight = halfExtents[cylinderUpAxis];


    btVector3 tmp;
    btScalar d ;

    btScalar s = btSqrt(v[XX] * v[XX] + v[ZZ] * v[ZZ]);
    if (s != btScalar(0.0))
    {
        d = radius / s;  
        tmp[XX] = v[XX] * d;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = v[ZZ] * d;
        return tmp;
    }
    else
    {
        tmp[XX] = radius;
        tmp[YY] = v[YY] < 0.0 ? -halfHeight : halfHeight;
        tmp[ZZ] = btScalar(0.0);
        return tmp;
    }


}

btVector3    btCylinderShapeX::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
{
    return CylinderLocalSupportX(getHalfExtentsWithoutMargin(),vec);
}


btVector3    btCylinderShapeZ::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
{
    return CylinderLocalSupportZ(getHalfExtentsWithoutMargin(),vec);
}
btVector3    btCylinderShape::localGetSupportingVertexWithoutMargin(const btVector3& vec)const
{
    return CylinderLocalSupportY(getHalfExtentsWithoutMargin(),vec);
}

void    btCylinderShape::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
    for (int i=0;i<numVectors;i++)
    {
        supportVerticesOut[i] = CylinderLocalSupportY(getHalfExtentsWithoutMargin(),vectors[i]);
    }
}

void    btCylinderShapeZ::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
    for (int i=0;i<numVectors;i++)
    {
        supportVerticesOut[i] = CylinderLocalSupportZ(getHalfExtentsWithoutMargin(),vectors[i]);
    }
}




void    btCylinderShapeX::batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
{
    for (int i=0;i<numVectors;i++)
    {
        supportVerticesOut[i] = CylinderLocalSupportX(getHalfExtentsWithoutMargin(),vectors[i]);
    }
}


