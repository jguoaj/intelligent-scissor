#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <fstream>
#include <queue>
#include <math.h>
#include <vector>

#include <QMainWindow>
#include <QScrollArea>
#include <QImage>
#include <QPoint>
#include <QMouseEvent>
#include <QImageWriter>

#include <QString>
#include <QFileDialog>
#include <QMessageBox>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Magick++.h>
#include <ctime>
#include "pixelnode.h"
#include "math.h"
#include "workstates.h"

using namespace cv;
using namespace std;
using namespace Magick;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    int workstates;

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    bool eventFilter(QObject *watched, QEvent *event);

    void updatePathTree();

    // helper function
    void print_node(pixelNode* n);

    void resetAll();

    void computeCostFunc();
    void channelTransform(QRgb rgb, int color[3]);
    void QImageToCVMat(const QImage &qimg,Mat &cvimg);
    double getDLink(int, int, int);

    void getPath(int, int, vector<QPoint>&);

    void getSnapSeed();

    QImage drawPathTree();


    //void Dijstras(pixelNode* p);
    //void costgraph_init();
    //void draw_contour(int x, int y);

    ~MainWindow();

private slots:
    void on_actionOpen_triggered();

    void on_actionSave_Contour_triggered();

    void on_actionSave_Mask_triggered();

    void on_actionExit_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionHelp_triggered();


    void on_actionScissor_triggered(bool checked);

    void on_actionDisplay_Contour_triggered(bool checked);

    void on_actionReset_Contour_triggered();


    void on_actionPixel_Node_triggered(bool checked);

    void on_actionCost_Graph_triggered(bool checked);

    void on_actionPath_Tree_triggered(bool checked);

    void on_actionMin_Path_triggered(bool checked);


    void on_actionGuassian_3_triggered();

    void on_actionGaussian_5_triggered();

    void on_actionSnapSeed_triggered(bool checked);

private:
    Ui::MainWindow *ui;

    Mat image;
    Mat contour;
    Mat contour_image;
    Mat tmp_contour;

    Mat pixel_node_image;
    Mat cost_graph_image;
    Mat path_tree_image;
    Mat min_path_image;


    QImage* Qimg;
    QImage* pathTree;
    QImage* Mask;


    vector<QPoint>* dots;
    vector< vector<QPoint> >* paths;
    //QPainter* painter;
    bool left_clicked = false;

    pixelNode* head_node;               // the head of node list, always the first seed
    pixelNode* current_node;            // store the address of current_node, for setting parent during click
    vector< vector<pixelNode*> > pixelnodes;   // pixelnodes[row][col]

    bool scissor_enabled;
    bool contour_enabled;
    bool ctrl_enabled;
    bool first_seed_flag;
    bool finished_flag;
    bool dots_deleted;
    bool finished_asclosed;
    bool is_snap;

    bool DEBUG_MODE;

    double img_scale;
    double img_scale_min;
    int idx;                    // mark the length of the nodes

    void display_image(cv::Mat im);
    void display_image();
    QScrollArea* scrollArea;

    // Dij algorithm
    //Mat* costgraph_weight;
    //Mat visitedMap;
    //Mat parentMap;
    //Mat graphCost;

    // click list
    //int* x_list;
    //int* y_list;
    //int list_size;

};


// compare function for priority queue
//struct compareQueue{
//    bool operator() ( pixelNode* a, pixelNode* b ){
//        return (a->getTotalCost() >= b->getTotalCost());
//    }
//};


#endif // MAINWINDOW_H



