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
#include <conio.h>
#include <ctype.h>
// #include <mem.h>
#include <string.h>
#include <time.h>

#include "fibheap.h"

class HeapNode : public FibHeapNode
{
      int N;

public:

      HeapNode() : FibHeapNode() { N = 0; };   

      virtual void operator =(FibHeapNode& RHS);
      virtual int  operator ==(FibHeapNode& RHS);
      virtual int  operator <(FibHeapNode& RHS);

      virtual void operator =(int NewKeyVal);
      virtual void Print();
      int GetKeyValue() { return N; };
      void SetKeyValue(int n) { N = n; };
};

void HeapNode::Print()
{
     FibHeapNode::Print();
     cout << N;
}

void HeapNode::operator =(int NewKeyVal)
{
HeapNode Temp;

     Temp.N = N = NewKeyVal;
     FHN_Assign(Temp);
}

void HeapNode::operator =(FibHeapNode& RHS)
{
     FHN_Assign(RHS);
     N = ((HeapNode&) RHS).N;
}

int  HeapNode::operator ==(FibHeapNode& RHS)
{
     if (FHN_Cmp(RHS)) return 0;
     return N == ((HeapNode&) RHS).N ? 1 : 0;
}

int  HeapNode::operator <(FibHeapNode& RHS)
{
int X;

     if ((X=FHN_Cmp(RHS)) != 0)
	  return X < 0 ? 1 : 0;

     return N < ((HeapNode&) RHS).N ? 1 : 0;
};

int IntCmp(const void *pA, const void *pB)
{
int A, B;

    A = *((const int *) pA);
    B = *((const int *) pB);
    if (A < B) return -1;
    if (A == B) return 0;
    return 1; 
}

#define USERINPUTMODE  (tolower(UserInputMode) == 'y')
#define TIMINGTESTMODE (tolower(TimingTestMode) == 't')

void main()
{
HeapNode Temp;
HeapNode *A = NULL, *Min;
int *X=NULL, *Y=NULL;
FibHeap *theHeap = NULL;
int Max, N, I;
long NumTests = 0;
char UserInputMode='y', TimingTestMode='r';

//     _InitEasyWin();

// How many elements will be inserted into the heap?

     cout << "User input mode (Y/N+return)? ";
     cin >> UserInputMode;

     if (!USERINPUTMODE)
     {
	  cout << "Timing Test or random test (T/R+return)? ";
	  cin >> TimingTestMode;
	  cout << "Hit ESC to stop\n";
     }

     cout << "Enter test size: ";
     cin >> Max;

// Setup for the Fibonacci heap

     if ((theHeap = new FibHeap) == NULL ||
	 (A = new HeapNode[Max+1]) == NULL ||
	 (X = new int[Max]) == NULL || (Y = new int[Max]) == NULL)
     {
	 cout << "Memory allocation failed-- ABORTING.\n";
         exit(-1);
     }

     theHeap->ClearHeapOwnership();

// Generate random sequences of the test size, insert
// them into the heap, and see that the extractions
// occur in order and correspond to the original input. 

//     randomize();
     while (1)
     {
	  if (USERINPUTMODE)
          {
	       cout << "Enter input values: ";
               N = Max; 
          }
	  else
	  {
	       if (TIMINGTESTMODE)
		   N = Max;
	       else
               {
	           cout << "Hit ESC to stop\n";
//		   N = random(Max-1)+1;
               }
	  }

	  if (kbhit() && getch()==27)
	      break;

// Generate or read the test sequence and build the heap

	  for (I = 0; I < N; I++)
          {
	       if (USERINPUTMODE)
	           cin >> X[I];
	       else X[I] = /* random(N) */ +1;

	       A[I] = X[I];
	       theHeap->Insert(&A[I]);
	  }

/* This loop was useful only for smaller test sets.
   Unusual behaviors were tracked down and fixed
   by having the particular numbers that generated
   the fault.

	  cout << "Size=" << N << "; Input = ";
	  for (I = 0; I < N; I++)
          {
               A[I].Print();
	       cout << ' ';
          }
	  cout << '\n';
*/

// Insert 0 then extract it.  This will cause the
// Fibonacci heap to get balanced.

	  theHeap->Insert(&Temp);
          theHeap->ExtractMin();

//cout << "Before DecreaseKey() Loop\n";
//theHeap->Print();

// Decrease all keys to their negative equivalents
// Turn the X array to negative as well

	  for (I=0; I < N; I++)
	  {
	       X[I] = -X[I];
	       Temp = A[I];
	       Temp.SetKeyValue(-A[I].GetKeyValue());
               theHeap->DecreaseKey(&A[I], Temp);
	  } 

//cout << "After DecreaseKey() Loop\n";
//theHeap->Print();

// Run the test.  Sort X array using qsort; Place
// 	sorted values in Y array using ExtractMin()
//	Basically, its qsort vs. heapsort; compare
//	X and Y arrays; If equal then pass test.

	  qsort(X, N, sizeof(int), IntCmp);
          memset(Y, 0, N*sizeof(int));

	  for (I = 0; I < N; I++)
          {
//cout << "Top of I loop\n";
//theHeap->Print();
	       if ((Min = ((HeapNode *) theHeap->ExtractMin())) == NULL)
                    break;
	       Y[I] = Min->GetKeyValue();
          }

	  if (memcmp(X, Y, N*sizeof(int)) != 0)
          {
	      cout << "An error occured-- aborting\n";
	      break;
	  }
	  else if (theHeap->Minimum() != NULL)
	  {
	      cout << "The ending heap is not empty-- aborting\n";
	      break;
          }

// Give positive feedback about passing another test

          NumTests ++;
	  if (!TimingTestMode)
	       cout << "N=" << N << ", Passed test #" << NumTests << '\n';
     }

     cout << "N=" << N << ", Passed test #" << NumTests << '\n';

// Cleanup; test heap ownership flag settings

     delete theHeap;
     delete[] A;
     delete X;
     delete Y;
}
