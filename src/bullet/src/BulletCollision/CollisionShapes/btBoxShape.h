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

#ifndef OBB_BOX_MINKOWSKI_H
#define OBB_BOX_MINKOWSKI_H

#include "btPolyhedralConvexShape.h"
#include "btCollisionMargin.h"
#include "BulletCollision/BroadphaseCollision/btBroadphaseProxy.h"
#include "LinearMath/btPoint3.h"
#include "LinearMath/btMinMax.h"

///btBoxShape implements both a feature based (vertex/edge/plane) and implicit (getSupportingVertex) Box
class btBoxShape: public btPolyhedralConvexShape
{

    //btVector3    m_boxHalfExtents1; //use m_implicitShapeDimensions instead


public:

    btVector3 getHalfExtentsWithMargin() const
    {
        btVector3 halfExtents = getHalfExtentsWithoutMargin();
        btVector3 margin(getMargin(),getMargin(),getMargin());
        halfExtents += margin;
        return halfExtents;
    }
    
    const btVector3& getHalfExtentsWithoutMargin() const
    {
        return m_implicitShapeDimensions;//changed in Bullet 2.63: assume the scaling and margin are included
    }
    

    virtual int    getShapeType() const { return BOX_SHAPE_PROXYTYPE;}

    virtual btVector3    localGetSupportingVertex(const btVector3& vec) const
    {
        btVector3 halfExtents = getHalfExtentsWithoutMargin();
        btVector3 margin(getMargin(),getMargin(),getMargin());
        halfExtents += margin;
        
        return btVector3(btFsels(vec.x(), halfExtents.x(), -halfExtents.x()),
            btFsels(vec.y(), halfExtents.y(), -halfExtents.y()),
            btFsels(vec.z(), halfExtents.z(), -halfExtents.z()));
    }

    SIMD_FORCE_INLINE  btVector3    localGetSupportingVertexWithoutMargin(const btVector3& vec)const
    {
        const btVector3& halfExtents = getHalfExtentsWithoutMargin();
        
        return btVector3(btFsels(vec.x(), halfExtents.x(), -halfExtents.x()),
            btFsels(vec.y(), halfExtents.y(), -halfExtents.y()),
            btFsels(vec.z(), halfExtents.z(), -halfExtents.z()));
    }

    virtual void    batchedUnitVectorGetSupportingVertexWithoutMargin(const btVector3* vectors,btVector3* supportVerticesOut,int numVectors) const
    {
        const btVector3& halfExtents = getHalfExtentsWithoutMargin();
    
        for (int i=0;i<numVectors;i++)
        {
            const btVector3& vec = vectors[i];
            supportVerticesOut[i].setValue(btFsels(vec.x(), halfExtents.x(), -halfExtents.x()),
                btFsels(vec.y(), halfExtents.y(), -halfExtents.y()),
                btFsels(vec.z(), halfExtents.z(), -halfExtents.z())); 
        }

    }


    btBoxShape( const btVector3& boxHalfExtents)
    {
        btVector3 margin(getMargin(),getMargin(),getMargin());
        m_implicitShapeDimensions = (boxHalfExtents * m_localScaling) - margin;
    };

    virtual void setMargin(btScalar collisionMargin)
    {
        //correct the m_implicitShapeDimensions for the margin
        btVector3 oldMargin(getMargin(),getMargin(),getMargin());
        btVector3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions+oldMargin;
        
        btConvexInternalShape::setMargin(collisionMargin);
        btVector3 newMargin(getMargin(),getMargin(),getMargin());
        m_implicitShapeDimensions = implicitShapeDimensionsWithMargin - newMargin;

    }
    virtual void    setLocalScaling(const btVector3& scaling)
    {
        btVector3 oldMargin(getMargin(),getMargin(),getMargin());
        btVector3 implicitShapeDimensionsWithMargin = m_implicitShapeDimensions+oldMargin;
        btVector3 unScaledImplicitShapeDimensionsWithMargin = implicitShapeDimensionsWithMargin / m_localScaling;

        btConvexInternalShape::setLocalScaling(scaling);

        m_implicitShapeDimensions = (unScaledImplicitShapeDimensionsWithMargin * m_localScaling) - oldMargin;

    }

    virtual void getAabb(const btTransform& t,btVector3& aabbMin,btVector3& aabbMax) const;

    

    virtual void    calculateLocalInertia(btScalar mass,btVector3& inertia) const;

    virtual void getPlane(btVector3& planeNormal,btPoint3& planeSupport,int i ) const
    {
        //this plane might not be aligned...
        btVector4 plane ;
        getPlaneEquation(plane,i);
        planeNormal = btVector3(plane.getX(),plane.getY(),plane.getZ());
        planeSupport = localGetSupportingVertex(-planeNormal);
    }

    
    virtual int getNumPlanes() const
    {
        return 6;
    }    
    
    virtual int    getNumVertices() const 
    {
        return 8;
    }

    virtual int getNumEdges() const
    {
        return 12;
    }


    virtual void getVertex(int i,btVector3& vtx) const
    {
        btVector3 halfExtents = getHalfExtentsWithoutMargin();

        vtx = btVector3(
                halfExtents.x() * (1-(i&1)) - halfExtents.x() * (i&1),
                halfExtents.y() * (1-((i&2)>>1)) - halfExtents.y() * ((i&2)>>1),
                halfExtents.z() * (1-((i&4)>>2)) - halfExtents.z() * ((i&4)>>2));
    }
    

    virtual void    getPlaneEquation(btVector4& plane,int i) const
    {
        btVector3 halfExtents = getHalfExtentsWithoutMargin();

        switch (i)
        {
        case 0:
            plane.setValue(btScalar(1.),btScalar(0.),btScalar(0.));
            plane[3] = -halfExtents.x();
            break;
        case 1:
            plane.setValue(btScalar(-1.),btScalar(0.),btScalar(0.));
            plane[3] = -halfExtents.x();
            break;
        case 2:
            plane.setValue(btScalar(0.),btScalar(1.),btScalar(0.));
            plane[3] = -halfExtents.y();
            break;
        case 3:
            plane.setValue(btScalar(0.),btScalar(-1.),btScalar(0.));
            plane[3] = -halfExtents.y();
            break;
        case 4:
            plane.setValue(btScalar(0.),btScalar(0.),btScalar(1.));
            plane[3] = -halfExtents.z();
            break;
        case 5:
            plane.setValue(btScalar(0.),btScalar(0.),btScalar(-1.));
            plane[3] = -halfExtents.z();
            break;
        default:
            assert(0);
        }
    }

    
    virtual void getEdge(int i,btPoint3& pa,btPoint3& pb) const
    //virtual void getEdge(int i,Edge& edge) const
    {
        int edgeVert0 = 0;
        int edgeVert1 = 0;

        switch (i)
        {
        case 0:
                edgeVert0 = 0;
                edgeVert1 = 1;
            break;
        case 1:
                edgeVert0 = 0;
                edgeVert1 = 2;
            break;
        case 2:
            edgeVert0 = 1;
            edgeVert1 = 3;

            break;
        case 3:
            edgeVert0 = 2;
            edgeVert1 = 3;
            break;
        case 4:
            edgeVert0 = 0;
            edgeVert1 = 4;
            break;
        case 5:
            edgeVert0 = 1;
            edgeVert1 = 5;

            break;
        case 6:
            edgeVert0 = 2;
            edgeVert1 = 6;
            break;
        case 7:
            edgeVert0 = 3;
            edgeVert1 = 7;
            break;
        case 8:
            edgeVert0 = 4;
            edgeVert1 = 5;
            break;
        case 9:
            edgeVert0 = 4;
            edgeVert1 = 6;
            break;
        case 10:
            edgeVert0 = 5;
            edgeVert1 = 7;
            break;
        case 11:
            edgeVert0 = 6;
            edgeVert1 = 7;
            break;
        default:
            btAssert(0);

        }

        getVertex(edgeVert0,pa );
        getVertex(edgeVert1,pb );
    }




    
    virtual    bool isInside(const btPoint3& pt,btScalar tolerance) const
    {
        btVector3 halfExtents = getHalfExtentsWithoutMargin();

        //btScalar minDist = 2*tolerance;
        
        bool result =    (pt.x() <= (halfExtents.x()+tolerance)) &&
                        (pt.x() >= (-halfExtents.x()-tolerance)) &&
                        (pt.y() <= (halfExtents.y()+tolerance)) &&
                        (pt.y() >= (-halfExtents.y()-tolerance)) &&
                        (pt.z() <= (halfExtents.z()+tolerance)) &&
                        (pt.z() >= (-halfExtents.z()-tolerance));
        
        return result;
    }


    //debugging
    virtual const char*    getName()const
    {
        return "Box";
    }

    virtual int        getNumPreferredPenetrationDirections() const
    {
        return 6;
    }
    
    virtual void    getPreferredPenetrationDirection(int index, btVector3& penetrationVector) const
    {
        switch (index)
        {
        case 0:
            penetrationVector.setValue(btScalar(1.),btScalar(0.),btScalar(0.));
            break;
        case 1:
            penetrationVector.setValue(btScalar(-1.),btScalar(0.),btScalar(0.));
            break;
        case 2:
            penetrationVector.setValue(btScalar(0.),btScalar(1.),btScalar(0.));
            break;
        case 3:
            penetrationVector.setValue(btScalar(0.),btScalar(-1.),btScalar(0.));
            break;
        case 4:
            penetrationVector.setValue(btScalar(0.),btScalar(0.),btScalar(1.));
            break;
        case 5:
            penetrationVector.setValue(btScalar(0.),btScalar(0.),btScalar(-1.));
            break;
        default:
            assert(0);
        }
    }

};

#endif //OBB_BOX_MINKOWSKI_H


