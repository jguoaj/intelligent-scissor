//***************************************************************************
// FIBTEST.CPP
//
// Test program for the F-heap implementation.
// Copyright (c) 1996 by John Boyer.
// See header file for free usage information.
//***************************************************************************

#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
// #include <conio.h>
#include <ctype.h>
// #include <mem.h>
#include <string.h>
#include <time.h>

#include "fibheap.h"

class HeapNode : public FibHeapNode
{
      float Key;

public:

      HeapNode() : FibHeapNode() { Key = 0; };   

      virtual void operator =(FibHeapNode& RHS);
      virtual int  operator ==(FibHeapNode& RHS);
      virtual int  operator <(FibHeapNode& RHS);

      virtual void operator =(float NewKeyVal);
      virtual void Print();
      float GetKeyValue() { return Key; };
      void SetKeyValue(float inkey) { Key = inkey; };
};

void HeapNode::Print()
{
     FibHeapNode::Print();
     cout << Key;
}

void HeapNode::operator =(float NewKeyVal)
{
HeapNode Temp;

     Temp.Key = Key = NewKeyVal;
     FHN_Assign(Temp);
}

void HeapNode::operator =(FibHeapNode& RHS)
{
     FHN_Assign(RHS);
     Key = ((HeapNode&) RHS).Key;
}

int  HeapNode::operator ==(FibHeapNode& RHS)
{
     if (FHN_Cmp(RHS)) return 0;
     return Key == ((HeapNode&) RHS).Key ? 1 : 0;
}

int  HeapNode::operator <(FibHeapNode& RHS)
{
int X;

     if ((X=FHN_Cmp(RHS)) != 0)
	  return X < 0 ? 1 : 0;

     return Key < ((HeapNode&) RHS).Key ? 1 : 0;
};
/*
int IntCmp(const void *pA, const void *pB)
{
int A, B;

    A = *((const int *) pA);
    B = *((const int *) pB);
    if (A < B) return -1;
    if (A == B) return 0;
    return 1; 
}
*/
void main()
{
HeapNode *Min;
HeapNode *A;
FibHeap *theHeap = NULL;
int Max=20;

// Setup for the Fibonacci heap

     if ((theHeap = new FibHeap) == NULL || (A = new HeapNode[Max+1]) == NULL)
     {
	 cout << "Memory allocation failed-- ABORTING.\n";
         exit(-1);
     }

//     theHeap->ClearHeapOwnership();

	 for (int i=0;i<Max;i++) 
		 A[i].SetKeyValue (-(float)i);
	 for (i=Max-1;i>=0;i--)
		 theHeap->Insert(&A[i]);
	
	 cout << "before #nodes = " << theHeap->GetNumNodes() << endl;

	 for (i=0;i<Max;i++) {
		 if ((Min = (HeapNode*) theHeap->ExtractMin ()) != NULL) {
			float key = -Min->GetKeyValue();
			cout << key << ' ';
		 }
	 }
	 cout << endl;

	 for (i=0;i<Max;i++)
		 cout << A[i].GetKeyValue() << ' ';

	 cout << "after #nodes = " << theHeap->GetNumNodes() << endl;
// Cleanup; test heap ownership flag settings

     delete theHeap;
}
