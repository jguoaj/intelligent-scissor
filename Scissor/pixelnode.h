#ifndef PIXELNODE_H
#define PIXELNODE_H

#include <stddef.h>
#include <QImage>
#include "fibheap.h"
#include "float.h"

class pixelNode: public FibHeapNode
{
public:
    // public data member
    enum State{INITIAL, ACTIVE, EXPANDED};
    State state;
    double totalCost;
    pixelNode *prevNode;


    // public function
    //double Column();
    //double Row();

    pixelNode();
    pixelNode(int col, int row);
    ~pixelNode();

    void setParent(pixelNode*);
    void setLinkCost(int link, double value);
    void resetTotalCost(double);
    void resetPrevNode();

    //return linkcost between this and pn, return -1 if they are not neighbor;
    //this is not used now
    double LinkCost(pixelNode *pn);

    //get column and row of neighbor in link direction
    void Neighbor(int link, int &col, int &row);

    virtual void operator =(FibHeapNode& RHS);
    virtual int  operator ==(FibHeapNode& RHS);
    virtual int  operator <(FibHeapNode& RHS);

    // Access function
    pixelNode* getParent();
    float getTotalCost();
    int getCol();
    int getRow();
    double getLinkCost(int link);     // return linkcost with linkID

private:
    int col, row;                     // pos in the image
    double linkcost[8];


};



#endif // PIXELNODE_H
