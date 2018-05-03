// BINTEST.CPP

#include <stdlib.h>
#include <iostream.h>
#include <stdio.h>
#include <conio.h>
#include <ctype.h>
#include <mem.h>
#include <time.h>

#include "binheap.h"

class HeapNode : public BinHeapNode
{
      int N;

protected:

     virtual int  KeyCmp(BinHeapNode *Node)
	  { return N < ((HeapNode *) Node)->N
		   ? -1 : (N == ((HeapNode *) Node)->N ? 0 : 1); };
     virtual void KeyAssign(BinHeapNode *Node)
	  { N = ((HeapNode *) Node)->N; };

public:

      virtual void InitNode(int Mode)
	   { BinHeapNode::InitNode(Mode); N = 0; };

      HeapNode()  { InitNode(CONSTRUCTING); };
      ~HeapNode() { InitNode(DESTROYING); }   

      virtual void operator =(HeapNode& RHS) { N = RHS.N; };
      virtual int  operator ==(HeapNode& RHS) { return N==RHS.N?1:0; };
      virtual int  operator <(HeapNode& RHS) { return N<RHS.N?1:0; };

      virtual void operator =(int NewKeyVal) { N=NewKeyVal; };
      virtual void Print() { cout << N; };
      int GetKeyValue() { return N; };
      void SetKeyValue(int n) { N = n; };
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
BinHeap *theHeap = NULL;
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

     if ((theHeap = new BinHeap) == NULL ||
	 (A = new HeapNode[Max+1]) == NULL ||
	 (X = new int[Max]) == NULL || (Y = new int[Max]) == NULL)
     {
	 cout << "Memory allocation failed-- ABORTING.\n";
         exit(-1);
     }

     theHeap->ClearOwnershipFlag();

// Generate random sequences of the test size, insert
// them into the heap, and see that the extractions
// occur in order and correspond to the original input. 

     randomize();
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
		   N = random(Max-1)+1;
               }
	  }

	  if (kbhit() && getch()==27)
	      break;

// Generate or read the test sequence and build the heap

	  for (I = 0; I < N; I++)
          {
	       if (USERINPUTMODE)
	           cin >> X[I];
	       else X[I] = random(N)+1;

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
