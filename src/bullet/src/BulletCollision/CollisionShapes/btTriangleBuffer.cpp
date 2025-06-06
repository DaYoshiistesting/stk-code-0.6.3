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

#include "btTriangleBuffer.h"


///example usage of this class:
//            btTriangleBuffer    triBuf;
//            concaveShape->processAllTriangles(&triBuf,aabbMin, aabbMax);
//            for (int i=0;i<triBuf.getNumTriangles();i++)
//            {
//                const btTriangle& tri = triBuf.getTriangle(i);
//                //do something useful here with the triangle
//            }




void btTriangleBuffer::processTriangle(btVector3* triangle,int partId,int  triangleIndex)
{
        btTriangle    tri;
        tri.m_vertex0 = triangle[0];
        tri.m_vertex1 = triangle[1];
        tri.m_vertex2 = triangle[2];
        tri.m_partId = partId;
        tri.m_triangleIndex = triangleIndex;
            
        m_triangleBuffer.push_back(tri);
}

