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

#ifndef OPTIMIZED_BVH_H
#define OPTIMIZED_BVH_H

//#define DEBUG_CHECK_DEQUANTIZATION 1
#ifdef DEBUG_CHECK_DEQUANTIZATION
#ifdef __SPU__
#define printf spu_printf
#endif //__SPU__

#include <stdio.h>
#include <stdlib.h>
#endif //DEBUG_CHECK_DEQUANTIZATION

#include "LinearMath/btVector3.h"
#include "LinearMath/btAlignedAllocator.h"


//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/vclang/html/vclrf__m128.asp



class btStridingMeshInterface;

//Note: currently we have 16 bytes per quantized node
#define MAX_SUBTREE_SIZE_IN_BYTES  2048

// 10 gives the potential for 1024 parts, with at most 2^21 (2097152) (minus one
// actually) triangles each (since the sign bit is reserved
#define MAX_NUM_PARTS_IN_BITS 10

///btQuantizedBvhNode is a compressed aabb node, 16 bytes.
///Node can be used for leafnode or internal node. Leafnodes can point to 32-bit triangle index (non-negative range).
ATTRIBUTE_ALIGNED16    (struct) btQuantizedBvhNode
{
    BT_DECLARE_ALIGNED_ALLOCATOR();

    //12 bytes
    unsigned short int    m_quantizedAabbMin[3];
    unsigned short int    m_quantizedAabbMax[3];
    //4 bytes
    int    m_escapeIndexOrTriangleIndex;

    bool isLeafNode() const
    {
        //skipindex is negative (internal node), triangleindex >=0 (leafnode)
        return (m_escapeIndexOrTriangleIndex >= 0);
    }
    int getEscapeIndex() const
    {
        btAssert(!isLeafNode());
        return -m_escapeIndexOrTriangleIndex;
    }
    int    getTriangleIndex() const
    {
        btAssert(isLeafNode());
        // Get only the lower bits where the triangle index is stored
        return (m_escapeIndexOrTriangleIndex&~((~0)<<(31-MAX_NUM_PARTS_IN_BITS)));
    }
    int    getPartId() const
    {
        btAssert(isLeafNode());
        // Get only the highest bits where the part index is stored
        return (m_escapeIndexOrTriangleIndex>>(31-MAX_NUM_PARTS_IN_BITS));
    }
}
;

/// btOptimizedBvhNode contains both internal and leaf node information.
/// Total node size is 44 bytes / node. You can use the compressed version of 16 bytes.
ATTRIBUTE_ALIGNED16 (struct) btOptimizedBvhNode
{
    BT_DECLARE_ALIGNED_ALLOCATOR();

    //32 bytes
    btVector3    m_aabbMinOrg;
    btVector3    m_aabbMaxOrg;

    //4
    int    m_escapeIndex;

    //8
    //for child nodes
    int    m_subPart;
    int    m_triangleIndex;
    int    m_padding[5];//bad, due to alignment


};


///btBvhSubtreeInfo provides info to gather a subtree of limited size
ATTRIBUTE_ALIGNED16(class) btBvhSubtreeInfo
{
public:
    BT_DECLARE_ALIGNED_ALLOCATOR();

    //12 bytes
    unsigned short int    m_quantizedAabbMin[3];
    unsigned short int    m_quantizedAabbMax[3];
    //4 bytes, points to the root of the subtree
    int            m_rootNodeIndex;
    //4 bytes
    int            m_subtreeSize;
    int            m_padding[3];

    btBvhSubtreeInfo()
    {
        //memset(&m_padding[0], 0, sizeof(m_padding));
    }


    void    setAabbFromQuantizeNode(const btQuantizedBvhNode& quantizedNode)
    {
        m_quantizedAabbMin[0] = quantizedNode.m_quantizedAabbMin[0];
        m_quantizedAabbMin[1] = quantizedNode.m_quantizedAabbMin[1];
        m_quantizedAabbMin[2] = quantizedNode.m_quantizedAabbMin[2];
        m_quantizedAabbMax[0] = quantizedNode.m_quantizedAabbMax[0];
        m_quantizedAabbMax[1] = quantizedNode.m_quantizedAabbMax[1];
        m_quantizedAabbMax[2] = quantizedNode.m_quantizedAabbMax[2];
    }
}
;


class btNodeOverlapCallback
{
public:
    virtual ~btNodeOverlapCallback() {};

    virtual void processNode(int subPart, int triangleIndex) = 0;
};

#include "LinearMath/btAlignedAllocator.h"
#include "LinearMath/btAlignedObjectArray.h"



///for code readability:
typedef btAlignedObjectArray<btOptimizedBvhNode>    NodeArray;
typedef btAlignedObjectArray<btQuantizedBvhNode>    QuantizedNodeArray;
typedef btAlignedObjectArray<btBvhSubtreeInfo>        BvhSubtreeInfoArray;


///OptimizedBvh store an AABB tree that can be quickly traversed on CPU (and SPU,GPU in future)
ATTRIBUTE_ALIGNED16(class) btOptimizedBvh
{
    NodeArray            m_leafNodes;
    NodeArray            m_contiguousNodes;

    QuantizedNodeArray    m_quantizedLeafNodes;
    
    QuantizedNodeArray    m_quantizedContiguousNodes;
    
    int                    m_curNodeIndex;


    //quantization data
    bool                m_useQuantization;
    btVector3            m_bvhAabbMin;
    btVector3            m_bvhAabbMax;
    btVector3            m_bvhQuantization;
public:
    BT_DECLARE_ALIGNED_ALLOCATOR();

    enum btTraversalMode
    {
        TRAVERSAL_STACKLESS = 0,
        TRAVERSAL_STACKLESS_CACHE_FRIENDLY,
        TRAVERSAL_RECURSIVE
    };
protected:

    btTraversalMode    m_traversalMode;
    
    BvhSubtreeInfoArray        m_SubtreeHeaders;

    //This is only used for serialization so we don't have to add serialization directly to btAlignedObjectArray
    int m_subtreeHeaderCount;


    ///two versions, one for quantized and normal nodes. This allows code-reuse while maintaining readability (no template/macro!)
    ///this might be refactored into a virtual, it is usually not calculated at run-time
    void    setInternalNodeAabbMin(int nodeIndex, const btVector3& aabbMin)
    {
        if (m_useQuantization)
        {
            quantize(&m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[0] ,aabbMin,0);
        } else
        {
            m_contiguousNodes[nodeIndex].m_aabbMinOrg = aabbMin;

        }
    }
    void    setInternalNodeAabbMax(int nodeIndex,const btVector3& aabbMax)
    {
        if (m_useQuantization)
        {
            quantize(&m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[0],aabbMax,1);
        } else
        {
            m_contiguousNodes[nodeIndex].m_aabbMaxOrg = aabbMax;
        }
    }

    btVector3 getAabbMin(int nodeIndex) const
    {
        if (m_useQuantization)
        {
            return unQuantize(&m_quantizedLeafNodes[nodeIndex].m_quantizedAabbMin[0]);
        }
        //non-quantized
        return m_leafNodes[nodeIndex].m_aabbMinOrg;

    }
    btVector3 getAabbMax(int nodeIndex) const
    {
        if (m_useQuantization)
        {
            return unQuantize(&m_quantizedLeafNodes[nodeIndex].m_quantizedAabbMax[0]);
        } 
        //non-quantized
        return m_leafNodes[nodeIndex].m_aabbMaxOrg;
        
    }

    
    void    setInternalNodeEscapeIndex(int nodeIndex, int escapeIndex)
    {
        if (m_useQuantization)
        {
            m_quantizedContiguousNodes[nodeIndex].m_escapeIndexOrTriangleIndex = -escapeIndex;
        } 
        else
        {
            m_contiguousNodes[nodeIndex].m_escapeIndex = escapeIndex;
        }

    }

    void mergeInternalNodeAabb(int nodeIndex,const btVector3& newAabbMin,const btVector3& newAabbMax) 
    {
        if (m_useQuantization)
        {
            unsigned short int quantizedAabbMin[3];
            unsigned short int quantizedAabbMax[3];
            quantize(quantizedAabbMin,newAabbMin,0);
            quantize(quantizedAabbMax,newAabbMax,1);
            for (int i=0;i<3;i++)
            {
                if (m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[i] > quantizedAabbMin[i])
                    m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMin[i] = quantizedAabbMin[i];

                if (m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[i] < quantizedAabbMax[i])
                    m_quantizedContiguousNodes[nodeIndex].m_quantizedAabbMax[i] = quantizedAabbMax[i];

            }
        } else
        {
            //non-quantized
            m_contiguousNodes[nodeIndex].m_aabbMinOrg.setMin(newAabbMin);
            m_contiguousNodes[nodeIndex].m_aabbMaxOrg.setMax(newAabbMax);        
        }
    }

    void    swapLeafNodes(int firstIndex,int secondIndex);

    void    assignInternalNodeFromLeafNode(int internalNode,int leafNodeIndex);

protected:

    

    void    buildTree    (int startIndex,int endIndex);

    int    calcSplittingAxis(int startIndex,int endIndex);

    int    sortAndCalcSplittingIndex(int startIndex,int endIndex,int splitAxis);
    
    void    walkStacklessTree(btNodeOverlapCallback* nodeCallback,const btVector3& aabbMin,const btVector3& aabbMax) const;

    void    walkStacklessQuantizedTreeAgainstRay(btNodeOverlapCallback* nodeCallback, const btVector3& raySource, const btVector3& rayTarget, const btVector3& aabbMin, const btVector3& aabbMax, int startNodeIndex,int endNodeIndex) const;
    void    walkStacklessQuantizedTree(btNodeOverlapCallback* nodeCallback,unsigned short int* quantizedQueryAabbMin,unsigned short int* quantizedQueryAabbMax,int startNodeIndex,int endNodeIndex) const;

    ///tree traversal designed for small-memory processors like PS3 SPU
    void    walkStacklessQuantizedTreeCacheFriendly(btNodeOverlapCallback* nodeCallback,unsigned short int* quantizedQueryAabbMin,unsigned short int* quantizedQueryAabbMax) const;

    ///use the 16-byte stackless 'skipindex' node tree to do a recursive traversal
    void    walkRecursiveQuantizedTreeAgainstQueryAabb(const btQuantizedBvhNode* currentNode,btNodeOverlapCallback* nodeCallback,unsigned short int* quantizedQueryAabbMin,unsigned short int* quantizedQueryAabbMax) const;

    ///use the 16-byte stackless 'skipindex' node tree to do a recursive traversal
    void    walkRecursiveQuantizedTreeAgainstQuantizedTree(const btQuantizedBvhNode* treeNodeA,const btQuantizedBvhNode* treeNodeB,btNodeOverlapCallback* nodeCallback) const;
    

#define USE_BANCHLESS 1
#ifdef USE_BANCHLESS
    //This block replaces the block below and uses no branches, and replaces the 8 bit return with a 32 bit return for improved performance (~3x on XBox 360)
    SIMD_FORCE_INLINE unsigned testQuantizedAabbAgainstQuantizedAabb(unsigned short int* aabbMin1,unsigned short int* aabbMax1,const unsigned short int* aabbMin2,const unsigned short int* aabbMax2) const
    {        
        return btSelect((unsigned)((aabbMin1[0] <= aabbMax2[0]) & (aabbMax1[0] >= aabbMin2[0])
            & (aabbMin1[2] <= aabbMax2[2]) & (aabbMax1[2] >= aabbMin2[2])
            & (aabbMin1[1] <= aabbMax2[1]) & (aabbMax1[1] >= aabbMin2[1])),
            1, 0);
    }
#else
    SIMD_FORCE_INLINE bool testQuantizedAabbAgainstQuantizedAabb(unsigned short int* aabbMin1,unsigned short int* aabbMax1,const unsigned short int* aabbMin2,const unsigned short int* aabbMax2) const
    {
        bool overlap = true;
        overlap = (aabbMin1[0] > aabbMax2[0] || aabbMax1[0] < aabbMin2[0]) ? false : overlap;
        overlap = (aabbMin1[2] > aabbMax2[2] || aabbMax1[2] < aabbMin2[2]) ? false : overlap;
        overlap = (aabbMin1[1] > aabbMax2[1] || aabbMax1[1] < aabbMin2[1]) ? false : overlap;
        return overlap;
    }
#endif //USE_BANCHLESS

    void    updateSubtreeHeaders(int leftChildNodexIndex,int rightChildNodexIndex);

public:
    btOptimizedBvh();

    virtual ~btOptimizedBvh();

    void    build(btStridingMeshInterface* triangles,bool useQuantizedAabbCompression, const btVector3& bvhAabbMin, const btVector3& bvhAabbMax);

    ///***************************************** expert/internal use only *************************
    void    setQuantizationValues(const btVector3& bvhAabbMin,const btVector3& bvhAabbMax,btScalar quantizationMargin=btScalar(1.0));
    QuantizedNodeArray&    getLeafNodeArray() {            return    m_quantizedLeafNodes;    }
    ///buildInternal is expert use only: assumes that setQuantizationValues and LeafNodeArray are initialized
    void    buildInternal();
    ///***************************************** expert/internal use only *************************

    void    reportAabbOverlappingNodex(btNodeOverlapCallback* nodeCallback,const btVector3& aabbMin,const btVector3& aabbMax) const;
    void    reportRayOverlappingNodex (btNodeOverlapCallback* nodeCallback, const btVector3& raySource, const btVector3& rayTarget) const;
    void    reportBoxCastOverlappingNodex(btNodeOverlapCallback* nodeCallback, const btVector3& raySource, const btVector3& rayTarget, const btVector3& aabbMin,const btVector3& aabbMax) const;

        SIMD_FORCE_INLINE void quantize(unsigned short* out, const btVector3& point,int isMax) const
    {

        btAssert(m_useQuantization);

        btAssert(point.getX() <= m_bvhAabbMax.getX());
        btAssert(point.getY() <= m_bvhAabbMax.getY());
        btAssert(point.getZ() <= m_bvhAabbMax.getZ());

        btAssert(point.getX() >= m_bvhAabbMin.getX());
        btAssert(point.getY() >= m_bvhAabbMin.getY());
        btAssert(point.getZ() >= m_bvhAabbMin.getZ());

        btVector3 v = (point - m_bvhAabbMin) * m_bvhQuantization;
        ///Make sure rounding is done in a way that unQuantize(quantizeWithClamp(...)) is conservative
        ///end-points always set the first bit, so that they are sorted properly (so that neighbouring AABBs overlap properly)
        ///todo: double-check this
        if (isMax)
        {
            out[0] = (unsigned short) (((unsigned short)(v.getX()+btScalar(1.)) | 1));
            out[1] = (unsigned short) (((unsigned short)(v.getY()+btScalar(1.)) | 1));
            out[2] = (unsigned short) (((unsigned short)(v.getZ()+btScalar(1.)) | 1));
        } else
        {
            out[0] = (unsigned short) (((unsigned short)(v.getX()) & 0xfffe));
            out[1] = (unsigned short) (((unsigned short)(v.getY()) & 0xfffe));
            out[2] = (unsigned short) (((unsigned short)(v.getZ()) & 0xfffe));
        }


#ifdef DEBUG_CHECK_DEQUANTIZATION
        btVector3 newPoint = unQuantize(out);
        if (isMax)
        {
            if (newPoint.getX() < point.getX())
            {
                printf("unconservative X, diffX = %f, oldX=%f,newX=%f\n",newPoint.getX()-point.getX(), newPoint.getX(),point.getX());
            }
            if (newPoint.getY() < point.getY())
            {
                printf("unconservative Y, diffY = %f, oldY=%f,newY=%f\n",newPoint.getY()-point.getY(), newPoint.getY(),point.getY());
            }
            if (newPoint.getZ() < point.getZ())
            {

                printf("unconservative Z, diffZ = %f, oldZ=%f,newZ=%f\n",newPoint.getZ()-point.getZ(), newPoint.getZ(),point.getZ());
            }
        } else
        {
            if (newPoint.getX() > point.getX())
            {
                printf("unconservative X, diffX = %f, oldX=%f,newX=%f\n",newPoint.getX()-point.getX(), newPoint.getX(),point.getX());
            }
            if (newPoint.getY() > point.getY())
            {
                printf("unconservative Y, diffY = %f, oldY=%f,newY=%f\n",newPoint.getY()-point.getY(), newPoint.getY(),point.getY());
            }
            if (newPoint.getZ() > point.getZ())
            {
                printf("unconservative Z, diffZ = %f, oldZ=%f,newZ=%f\n",newPoint.getZ()-point.getZ(), newPoint.getZ(),point.getZ());
            }
        }
#endif //DEBUG_CHECK_DEQUANTIZATION

    }


    SIMD_FORCE_INLINE void quantizeWithClamp(unsigned short* out, const btVector3& point2,int isMax) const
    {

        btAssert(m_useQuantization);

        btVector3 clampedPoint(point2);
        clampedPoint.setMax(m_bvhAabbMin);
        clampedPoint.setMin(m_bvhAabbMax);

        quantize(out,clampedPoint,isMax);

    }
    
    SIMD_FORCE_INLINE btVector3    unQuantize(const unsigned short* vecIn) const
    {
            btVector3    vecOut;
            vecOut.setValue(
            (btScalar)(vecIn[0]) / (m_bvhQuantization.getX()),
            (btScalar)(vecIn[1]) / (m_bvhQuantization.getY()),
            (btScalar)(vecIn[2]) / (m_bvhQuantization.getZ()));
            vecOut += m_bvhAabbMin;
            return vecOut;
    }

    ///setTraversalMode let's you choose between stackless, recursive or stackless cache friendly tree traversal. Note this is only implemented for quantized trees.
    void    setTraversalMode(btTraversalMode    traversalMode)
    {
        m_traversalMode = traversalMode;
    }

    void    refit(btStridingMeshInterface* triangles,const btVector3& aabbMin,const btVector3& aabbMax);

    void    refitPartial(btStridingMeshInterface* triangles,const btVector3& aabbMin, const btVector3& aabbMax);

    void    updateBvhNodes(btStridingMeshInterface* meshInterface,int firstNode,int endNode,int index);


    SIMD_FORCE_INLINE QuantizedNodeArray&    getQuantizedNodeArray()
    {    
        return    m_quantizedContiguousNodes;
    }


    SIMD_FORCE_INLINE BvhSubtreeInfoArray&    getSubtreeInfoArray()
    {
        return m_SubtreeHeaders;
    }


    /////Calculate space needed to store BVH for serialization
    unsigned calculateSerializeBufferSize();

    /// Data buffer MUST be 16 byte aligned
    bool serialize(void *o_alignedDataBuffer, unsigned i_dataBufferSize, bool i_swapEndian);

    ///deSerializeInPlace loads and initializes a BVH from a buffer in memory 'in place'
    static btOptimizedBvh *deSerializeInPlace(void *i_alignedDataBuffer, unsigned int i_dataBufferSize, bool i_swapEndian);

    static unsigned int getAlignmentSerializationPadding();

    SIMD_FORCE_INLINE bool isQuantized()
    {
        return m_useQuantization;
    }

private:
    // Special "copy" constructor that allows for in-place deserialization
    // Prevents btVector3's default constructor from being called, but doesn't inialize much else
    // ownsMemory should most likely be false if deserializing, and if you are not, don't call this (it also changes the function signature, which we need)
    btOptimizedBvh(btOptimizedBvh &other, bool ownsMemory);

}
;


#endif //OPTIMIZED_BVH_H


