#ifndef _BINHEAP_H
#define _BINHEAP_H

//***************************************************************************
// The binary heap implementation contained in BINHEAP.H and BINHEAP.CPP
// is Copyright (c) 1996 by John Boyer, All Rights Reserved.
//***************************************************************************

// #include "const.h"

#define REINITIALIZING 1
#define OK 1
#define NOTOK 0

//======================================================
// Binary Heap Node Class
//======================================================

class BinHeapNode
{
friend class BinHeap;

     int Index;

protected:

     virtual int  KeyCmp(BinHeapNode *) { return 0; };
     virtual void KeyAssign(BinHeapNode *) {};

public:

     virtual void InitNode(int) { Index = -1; };
     virtual void Print() {};
};

//========================================================================
// Binary Heap Class
//========================================================================

class BinHeap
{
     BinHeapNode **A;
     long NumNodes, ArraySize;
     int OwnershipFlag;

// Private Functions

     int  GrowArray();
     int  ShrinkArray();

     int  Parent(int I) { return (I-1)/2; };
     int  Left(int I)   { return (I<<1)+1; };
     int  Right(int I)  { return (I<<1)+2; };

     void Heapify(int);

public:

// User Interface

     BinHeap();
     virtual ~BinHeap();

     BinHeapNode *Minimum() { return A[0]; };

     int  Insert(BinHeapNode *NewNode);
     BinHeapNode *ExtractMin();
     int DecreaseKey(BinHeapNode *theNode, BinHeapNode& NewKey);

//     int Delete(BinHeapNode *theNode);
//     void Union(BinHeap *OtherHeap);

// Extra utility functions

     int  GetOwnershipFlag() { return OwnershipFlag; };
     void SetOwnershipFlag() { OwnershipFlag = 1; };
     void ClearOwnershipFlag() { OwnershipFlag = 0; };

     long GetNumNodes() { return NumNodes; };
};

#endif
