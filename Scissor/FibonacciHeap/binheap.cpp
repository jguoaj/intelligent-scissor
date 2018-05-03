#define _BINHEAP_CPP

//***************************************************************************
// The binary heap implementation contained in BINHEAP.H and BINHEAP.CPP
// is Copyright (c) 1996 by John Boyer, All Rights Reserved.
//***************************************************************************

// #include <mem.h>
#include <malloc.h>
#include <string.h>
#include "binheap.h"

typedef BinHeapNode * PBinHeapNode;

// The PURE_CPP define is a switch that causes the heap array to be grown and
// shrunk using C++ new/delete and copying rather than using realloc()
// This helps us to see the true logarithmic behavior of Insert() and ExtractMin().
// Because realloc() doesn't move the block if it can be expanded in place
// the effect of Grow() and Shrink() sometimes becomes O(0) rather than
// O(1) per operation.  This variability makes it more difficult to
// measure the true logarithmic behavior of these operations.
// Note that growing and shrinking by one element at a time causes Insert
// and ExtractMin() to be O(n)-- only Insert() is O(n) using realloc since
// shrinking can always happen in-place.  The doubling is trying to address
// the fact that realloc may need to copy the array to a new location on a
// significant number of the expansions.
 
#define PURE_CPP

//===========================================================================
//===========================================================================

BinHeap::BinHeap()
{
     A = NULL;
     NumNodes = ArraySize = 0;
     OwnershipFlag = 0;
}

//===========================================================================
//===========================================================================

BinHeap::~BinHeap()
{
     if (A != NULL)
     {
	 for (int i=0; i < GetNumNodes(); i++)
	      if (GetOwnershipFlag())
		   delete A[i];
	      else A[i]->InitNode(REINITIALIZING);

#ifdef PURE_CPP
	 delete[] A;
#else
	 free(A);
#endif
	 A = NULL;
     }
     NumNodes = ArraySize = 0;
     OwnershipFlag = 0;
}

//===========================================================================
//===========================================================================

#include <stdio.h>

int  BinHeap::GrowArray()
{
     if (NumNodes < ArraySize) return OK;

     if (ArraySize == 0)
     {
#ifdef PURE_CPP
	 A = new PBinHeapNode[2];
#else
	 A = (BinHeapNode **) malloc(2*sizeof(BinHeapNode *));
#endif
	 if (A == NULL) return NOTOK;
	 ArraySize = 2;
         return OK;
     }

BinHeapNode **Temp;

#ifdef PURE_CPP
     Temp = new PBinHeapNode[(ArraySize<<1)];
     if (Temp != NULL)
     {
	 memcpy(Temp, A, ArraySize*sizeof(PBinHeapNode));
	 delete[] A;
     }
#else
     Temp = (BinHeapNode **) realloc(A, (ArraySize<<1)*sizeof(BinHeapNode *));
#endif

     if (Temp == NULL) return NOTOK;

     A = Temp;
     ArraySize <<= 1;
     return OK;
}

//===========================================================================
//===========================================================================

int  BinHeap::ShrinkArray()
{
     if (NumNodes >= (ArraySize >> 2)) return OK;

#ifdef PURE_CPP

BinHeapNode **Temp;

     Temp = new PBinHeapNode[(ArraySize>>1)];
     if (Temp != NULL)
     {
	 memcpy(Temp, A, ArraySize*sizeof(PBinHeapNode));
	 delete[] A;
     }
     else return NOTOK;
#else
     A = (BinHeapNode **) realloc(A, (ArraySize>>1)*sizeof(BinHeapNode *));
#endif

     ArraySize >>= 1;
     return OK;
}

//===========================================================================
//===========================================================================

void BinHeap::Heapify(int I)
{
int  L, R, Smallest;
BinHeapNode *Temp;

     while (1)
     {
         L = Left(I);
         R = Right(I);
         Smallest = I;

         if (L < GetNumNodes() && A[L]->KeyCmp(A[Smallest]) < 0)
	     Smallest = L;
         if (R < GetNumNodes() && A[R]->KeyCmp(A[Smallest]) < 0)
	     Smallest = R;

	 if (Smallest == I) break;

         Temp = A[I];
	 A[I] = A[Smallest];
	 A[Smallest] = Temp;

	 A[I]->Index = I;
	 A[Smallest]->Index = Smallest;

         I = Smallest;
     }
}        

//===========================================================================
//===========================================================================

int  BinHeap::Insert(BinHeapNode *NewNode)
{
int  I;

     if (GrowArray() != OK) return NOTOK;

     I = NumNodes;
     while (I > 0 && NewNode->KeyCmp(A[Parent(I)]) < 0)
     {
	 A[I] = A[Parent(I)];
         A[I]->Index = I;
	 I = Parent(I);
     }

     A[I] = NewNode;
     A[I]->Index = I;
     NumNodes ++;
     return OK;
}

//===========================================================================
//===========================================================================

BinHeapNode *BinHeap::ExtractMin()
{
BinHeapNode *theMin;

     if (NumNodes < 1) return NULL;

     theMin = A[0];
     A[0] = A[--NumNodes];
     A[0]->Index = 0;
     A[NumNodes] = NULL;
     Heapify(0);

     return theMin;
}

//===========================================================================
//===========================================================================

int  BinHeap::DecreaseKey(BinHeapNode *theNode, BinHeapNode& NewKey)
{
int  I;

     if (theNode==NULL || theNode->KeyCmp(&NewKey) < 0)
	 return NOTOK;

     theNode->KeyAssign(&NewKey);

     I = theNode->Index;
     while (I > 0 && theNode->KeyCmp(A[Parent(I)]) < 0)
     {
	 A[I] = A[Parent(I)];
         A[I]->Index = I;
	 I = Parent(I);
     }

     A[I] = theNode;
     A[I]->Index = I;

     return OK;
}
