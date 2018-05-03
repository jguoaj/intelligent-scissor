#include "mainwindow.h"
#include "ui_mainwindow.h"

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qApp->installEventFilter( this );

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(ui->label);
    setCentralWidget(scrollArea);

    // ui->scrollArea->setWidgetResizable(true);
    // ui->scrollArea->setWidget(ui->label);
    // setCentralWidget(ui->scrollArea);

    // costgraph_weight = new cv::Mat[9];

    img_scale = 1.0;
    img_scale_min = 1.0;

    contour_enabled = false;
    scissor_enabled = false;
    first_seed_flag = false;
    finished_flag = false;
    dots_deleted = false;
    finished_asclosed = false;
    is_snap = false;

    DEBUG_MODE = false;

    Qimg = NULL;
    pathTree = NULL;
    Mask = NULL;
}


MainWindow::~MainWindow()
{
    delete ui;
    delete scrollArea;

    delete head_node;

    // delete[] costgraph_weight;
}

/* helper function starts here*/
// display the image

void MainWindow::display_image(){
    cv::Mat myImage;
    if (workstates == image_only)
        myImage = image;
    else if (workstates == image_only_contour)
        myImage = contour_image;
    else if (workstates == pixel_node)
        myImage = pixel_node_image;
    else if (workstates == cost_graph)
        myImage = cost_graph_image;
    else if (workstates == path_tree)
        myImage = path_tree_image;
    else if (workstates == min_path)
        myImage = min_path_image;

    QImage Q_img = QImage((const unsigned char*)(myImage.data), myImage.cols, myImage.rows, myImage.step, QImage::Format_RGB888);
    QPixmap p = QPixmap::fromImage(Q_img);
    ui->label->setPixmap( p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio) );

}

void MainWindow::display_image(cv::Mat im){

    QImage Q_img = QImage((const unsigned char*)(im.data), im.cols, im.rows, im.step, QImage::Format_RGB888);
    QPixmap p = QPixmap::fromImage(Q_img);
    ui->label->setPixmap( p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio) );

}

void MainWindow::print_node(pixelNode* n)
{
    pixelNode* p = n;
    while(p->getParent() != NULL){
        cout << p->getCol() << '\t' << p->getRow() << endl;
        p = p->getParent();
    }
}


void MainWindow::resetAll(){
    ui->actionPixel_Node->setChecked(false);
    ui->actionCost_Graph->setChecked(false);
    ui->actionMin_Path->setChecked(false);
    ui->actionPath_Tree->setChecked(false);

    ui->actionGaussian_5->setChecked(false);
    ui->actionGuassian_3->setChecked(false);
    ui->actionDisplay_Contour->setChecked(false);
    ui->actionScissor->setChecked(false);

    workstates = image_only;
    img_scale = img_scale_min;
    contour_enabled = false;
    scissor_enabled = false;
    first_seed_flag = false;
    finished_flag = false;
    dots_deleted = false;
    finished_asclosed = false;
    is_snap = false;
    return;
}

QImage MainWindow::drawPathTree(){
    int w = Qimg->width(),h = Qimg->height();
    QImage qi( 3*w, 3*h, Qimg->format() );
    qi.fill(qRgb(255,255,255));  // white

    for(int j = 0; j < h; j++)
        for(int i = 0; i < w; i++)
        {
                pixelNode *pn = pixelnodes[j][i];
                qi.setPixel(3*i + 1, 3*j + 1, Qimg->pixel(i,j));
                pixelNode *prevpn = pn->getParent();
                if(prevpn != NULL)
                {
                    int c = 3 * i + 1 + prevpn->getCol() - pn->getCol();
                    int r = 3 * j + 1 + prevpn->getRow() - pn->getRow();
                    qi.setPixel(c,r,qRgb(255,255,0));  // light yellow
                    c = 3 * prevpn->getCol() + 1 + pn->getCol()- prevpn->getCol();
                    r = 3 * prevpn->getRow() + 1 + pn->getRow() - prevpn->getRow();
                    qi.setPixel(c,r,qRgb(100,100,0));  // dark yellow
                }
        }
    return qi;
}

void MainWindow::computeCostFunc(){
    int w = Qimg->width(), h = Qimg->height();
    double maxD = -1.0;
    double length = 0.0;
    double DLink = 0.0;

    // find maxD
    for (int j = 0; j < h; j ++)
        for (int i = 0; i < w; i++) {
            pixelNode *pn=pixelnodes[j][i];
            for(int k=0;k<8;k++)
            {
                pn->setLinkCost(k,getDLink(i,j,k));
                if(pn->getLinkCost(k)>maxD)
                    maxD=pn->getLinkCost(k);
            }
        }

    for (int j = 0; j < h; j ++)
        for (int i = 0; i < w; i ++) {
            pixelNode *pn = pixelnodes[j][i];
            for (int k = 0; k < 8; k++) {
                length = k % 2 == 0 ? 1.0 : sqrt(2);
                DLink = pn->getLinkCost(k);
                pn->setLinkCost(k, (maxD - DLink) * length);
            }
        }
}

void MainWindow::channelTransform(QRgb rgb, int color[3]) {
    color[0] = qRed(rgb);
    color[1] = qGreen(rgb);
    color[2] = qBlue(rgb);
}

double MainWindow::getDLink(int i, int j, int k) {
    double DLink = 0.0;
    double D[3];
    int c0[3], c1[3], c2[3], c3[3];
    switch (k) {
    case 0:
        if(j == 0 || j == Qimg->height() - 1 || i == Qimg->width() - 1) return -1.0; // boundary exceed;
        channelTransform(Qimg->pixel(i, j - 1), c0);
        channelTransform(Qimg->pixel(i + 1, j - 1), c1);
        channelTransform(Qimg->pixel(i, j + 1), c2);
        channelTransform(Qimg->pixel(i + 1, j + 1), c3);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs((c0[t] + c1[t]) / 2.0-(c2[t] + c3[t]) / 2.0) / 2.0;
        }
        break;
    case 1:
        if (j == 0 || i == Qimg->width() - 1) return -1.0;
        channelTransform(Qimg->pixel(i + 1, j), c0);
        channelTransform(Qimg->pixel(i, j - 1), c1);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs(1.0 * (c0[t] - c1[t])) / sqrt(2);
        }
        break;
    case 2:
        if (i ==0 || i == Qimg->width() - 1 || j == 0) return -1.0;
        channelTransform(Qimg->pixel(i - 1, j), c0);
        channelTransform(Qimg->pixel(i - 1, j - 1), c1);
        channelTransform(Qimg->pixel(i + 1, j), c2);
        channelTransform(Qimg->pixel(i + 1, j - 1), c3);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs((c0[t] + c1[t]) / 2.0 - (c2[t] + c3[t]) / 2.0) / 2.0;
        }
        break;
    case 3:
        if (i == 0 || j == 0) return -1.0;
        channelTransform(Qimg->pixel(i - 1, j), c0);
        channelTransform(Qimg->pixel(i, j - 1), c1);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs(1.0 * (c0[t] - c1[t])) / sqrt(2);
        }
        break;
    case 4:
        if (i == 0 || j == 0 || j == Qimg->height() -1) return -1.0;
        channelTransform(Qimg->pixel(i, j - 1), c0);
        channelTransform(Qimg->pixel(i - 1, j - 1), c1);
        channelTransform(Qimg->pixel(i - 1, j + 1), c2);
        channelTransform(Qimg->pixel(i, j + 1), c3);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs((c0[t] + c1[t]) / 2.0 - (c2[t] + c3[t]) / 2.0) / 2.0;
        }
        break;
    case 5:
        if (i == 0 || j == Qimg->height() - 1) return -1.0;
        channelTransform(Qimg->pixel(i - 1, j), c0);
        channelTransform(Qimg->pixel(i, j + 1), c1);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs(1.0 * (c0[t] - c1[t])) / sqrt(2);
        }
        break;
    case 6:
        if (i == 0 || i == Qimg->width() - 1 || j == Qimg->height() -1) return -1.0;
        channelTransform(Qimg->pixel(i - 1, j), c0);
        channelTransform(Qimg->pixel(i - 1, j + 1), c1);
        channelTransform(Qimg->pixel(i + 1, j), c2);
        channelTransform(Qimg->pixel(i + 1, j + 1), c3);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs((c0[t] + c1[t]) / 2.0 - (c2[t] + c3[t]) / 2.0) / 2.0;
        }
        break;
    case 7:
        if (i == Qimg->width() - 1 || j == Qimg->height() - 1) return -1.0;
        channelTransform(Qimg->pixel(i + 1, j), c0);
        channelTransform(Qimg->pixel(i, j + 1), c1);
        for (int t = 0; t <3; t ++) {
            D[t]=fabs(1.0 * (c0[t] - c1[t])) / sqrt(2);
        }
        break;
    default:
        return - 1.0;
    }
    for (int t = 0; t < 3; t ++)
        DLink += D[t] * D[t];
    DLink = sqrt(DLink) / 3;
    return DLink;
}

void MainWindow::getPath(int x, int y, vector<QPoint> & path) {
    pixelNode* npn = pixelnodes[y][x];
    while (npn->getParent() != NULL) {
        path.push_back(QPoint(npn->getCol(), npn->getRow()));
        npn = npn->getParent();
    }
    reverse(path.begin(), path.end());
}

void MainWindow::getSnapSeed() {
    int c = head_node->getCol();
    int r = head_node->getRow();

    // compute the edge by cv::canny
    Mat tmp_image = image.clone();
    Mat gray(tmp_image.rows,tmp_image.cols,CV_8UC1);
    cvtColor(tmp_image, gray, CV_BGR2GRAY);
    Mat edge;
    Canny(gray, edge, 30, 127, 3, true);

    // find the nearest neighbor on the edge
    double th = DBL_MAX;
    for (int i = 0; i < edge.cols; i ++) {
        for (int j = 0; j < edge.rows; j++) {
            if (edge.at<uchar>(j,i) == 255 && (i!=head_node->getCol()) && (j!=head_node->getRow())) {
                double d = (i-head_node->getCol())*(i-head_node->getCol()) + (j-head_node->getRow())*(j-head_node->getRow());
                if (d < th){
                    th = d;
                    c = i;
                    r = j;
                }
            }
        }
    }
    head_node = pixelnodes[r][c];
    return;
}

/* helper function ends here */

// open a image
void MainWindow::on_actionOpen_triggered()
{
    resetAll();

    workstates = image_only;

//    if (Qimg != NULL) delete Qimg;
//    if (pathTree != NULL) delete pathTree;
//    if (Mask != NULL) delete Mask;
//    if (dots != NULL) delete dots;
//    if (paths != NULL) delete paths;

    QString fileName = QFileDialog::getOpenFileName(
                this, tr("Open Image"), ".", tr("Image File(*.png *.jpg *.jpeg *.bmp *.tif *.gif)"));
    bool is_gif = fileName.contains("gif");

    if (is_gif) {
        Magick::InitializeMagick("");
        Magick::Image mimage(fileName.toStdString());
        int w = mimage.columns();
        int h = mimage.rows();

        // Make OpenCV Mat of same size with 8-bit and 3 channels
        image = cv::Mat::zeros(h, w, CV_8UC3);
        // Unpack Magick++ pixels into OpenCV Mat structure
        mimage.write(0, 0, w, h,"BGR",Magick::CharPixel,image.data);
        contour_image = cv::Mat::zeros(h, w, CV_8UC3);
        mimage.write(0, 0, w, h,"BGR",Magick::CharPixel,contour_image.data);

    } else {
        image = cv::imread(fileName.toLatin1().data());
        contour_image = cv::imread(fileName.toLatin1().data());
        contour = cv::Mat::zeros(image.size(), CV_8UC3);
    }

    // convert cv::Mat to QImage
    cv::cvtColor(image, image, CV_BGR2RGB);
    cv::cvtColor(contour_image, contour_image, CV_BGR2RGB);
    //current_image = image.clone();
    //previous_image = image.clone();

    int width_im = image.cols;
    int height_im = image.rows;
    double width_m = ui->label->width();
    double height_m = ui->label->height();

    if (width_im < width_m || height_im < height_m) {
        img_scale = (width_m / width_im) > (height_m / height_im) ? (width_m / width_im) : (height_m / height_im);
        img_scale_min = img_scale;
    }

    QImage Q_img = QImage((const unsigned char*)(image.data), image.cols, image.rows, image.step, QImage::Format_RGB888);

    // i, height, row
    // j, width, col
    pixelnodes.resize(Q_img.height());
    for(int i=0; i<Q_img.height(); i++)
        for(int j=0; j<Q_img.width(); j++)
            pixelnodes[i].push_back(new pixelNode(j,i));

    Qimg = new QImage((const unsigned char*)(image.data), image.cols, image.rows, image.step, QImage::Format_RGB888);

    computeCostFunc();

    Mask = new QImage(Qimg->width(),Qimg->height(),Qimg->format());
    Mask->fill(qRgb(0, 0, 0));  // black
    head_node = NULL;
    //pathTree = new QImage(drawPathTree());
    pathTree = new QImage;
    dots = new vector<QPoint>;
    paths = new vector< vector<QPoint> >;

    ui->label->setPixmap(QPixmap::fromImage(Q_img).scaled(QPixmap::fromImage(Q_img).width()*img_scale,
                                                          QPixmap::fromImage(Q_img).height()*img_scale, Qt::KeepAspectRatio));
}


// Save image
void MainWindow::on_actionSave_Contour_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Save ", "Save image with contour marked?",
                                                              QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes){
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);

        strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
        std::string str(buffer);
        QImage conImg = QImage((const unsigned char*)(contour_image.data), contour_image.cols, contour_image.rows, contour_image.step, QImage::Format_RGB888);
        QImageWriter writer;
        writer.setFileName(QString::fromStdString("contour_"+str+".png"));
        writer.setFormat("png");
        writer.write(conImg);
    }
}


void MainWindow::on_actionSave_Mask_triggered()
{

    QMessageBox::StandardButton reply = QMessageBox::question(this, "Save Mask", "Save compositing mask for PhotoShop?",
                                                              QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes) {
        if (finished_asclosed) {
            if (paths->size() < 1) return;
            for (uint i = 0; i < paths->size(); i ++) {
                vector<QPoint> path = paths->at(i);
                for (uint j = 0; j < path.size(); j++)
                    Mask->setPixel(path[j], qRgb(255, 255, 255));
            }

            Mat tmp_mask = cv::Mat( Mask->height(), Mask->width(),
                                     CV_8UC3,
                                     const_cast<uchar*>(Mask->bits()),
                                     static_cast<size_t>(Mask->bytesPerLine())
                                     ).clone();

            cv::cvtColor(tmp_mask, tmp_mask, CV_BGR2GRAY);

            // Floodfill from point (0, 0)
            Mat im_floodfill = tmp_mask.clone();
            floodFill(im_floodfill, cv::Point(0,0), Scalar(255));

            // Invert floodfilled image
            Mat im_floodfill_inv;
            bitwise_not(im_floodfill, im_floodfill_inv);

            // Combine the two images to get the foreground.
            Mat im_out = (tmp_mask | im_floodfill_inv);
//            Mat im_out;
//            bitwise_and(image, image, im_out, im_floodfill_inv);
//            cv::cvtColor(im_out, im_out, CV_BGR2RGB);


            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
            std::string str(buffer);
            cv::imwrite("mask_"+str+".png", im_out);

        } else {
            if (paths->size() < 1) return;
            for (uint i = 0; i < paths->size(); i ++) {
                vector<QPoint> path = paths->at(i);
                for (uint j = 0; j < path.size(); j++)
                    Mask->setPixel(path[j], qRgb(255, 255, 255));
            }

            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
            std::string str(buffer);
            QImageWriter writer;
            writer.setFileName(QString::fromStdString("mask_"+str+".png"));
            writer.setFormat("png");
            writer.write(*Mask);
        }
    }
}


// close the program
void MainWindow::on_actionExit_triggered()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Exit", "Are you sure to close this program?",
                                                              QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes)
        QApplication::quit();
}


// zoom in and out
void MainWindow::on_actionZoom_In_triggered()
{
    img_scale += 0.1;
    display_image();
}


void MainWindow::on_actionZoom_Out_triggered()
{
    img_scale -= 0.1;
    display_image();
}


// display help message
void MainWindow::on_actionHelp_triggered()
{
    cout << "workstates is: " << workstates << endl;
    // this->print_node(current_node);
    // cout << "Image size is: " << image.rows << '\t' << image.cols << endl;

    // QString text = QString("Nothing for help !");
    // QMessageBox::about(this, "Help", text);
}


// Work Mode
void MainWindow::on_actionScissor_triggered(bool checked)
{
    scissor_enabled = checked;
}


void MainWindow::on_actionDisplay_Contour_triggered(bool checked)
{
    contour_enabled = checked;
    if (contour_enabled)
        workstates = image_only_contour;
    else
        workstates = image_only;

    display_image();
}


void MainWindow::on_actionReset_Contour_triggered()
{
    cout << "reset contour" << endl;
    contour_image = image.clone();
    resetAll();
    display_image();
}


// Debug Mode
void MainWindow::on_actionPixel_Node_triggered(bool checked)
{
    if (checked) {
        DEBUG_MODE = true;
        workstates = pixel_node;

        QImage* Q_img = new QImage((const unsigned char*)(image.data),image.cols,image.rows,image.step,QImage::Format_RGB888);
        int w=Q_img->width(), h=Q_img->height();
        QImage png(3*w,3*h,Q_img->format());
        png.fill( qRgb(0,0,0) );

        for(int j=0; j<h; j++)
            for(int i=0; i<w; i++)
                png.setPixel(3*i+1, 3*j+1, Q_img->pixel(i,j));

        QPixmap p = QPixmap::fromImage(png);
        ui->label->setPixmap(p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio));
        // QImage Q_img_tmp = Q_img->rgbSwapped();

        if (ui->actionCost_Graph->isChecked())
            ui->actionCost_Graph->setChecked(false);
        if (ui->actionPath_Tree->isChecked())
            ui->actionPath_Tree->setChecked(false);
        if (ui->actionMin_Path->isChecked())
            ui->actionMin_Path->setChecked(false);

        pixel_node_image = cv::Mat( png.height(), png.width(),
                                 CV_8UC3,
                                 const_cast<uchar*>(png.bits()),
                                 static_cast<size_t>(png.bytesPerLine())
                                 ).clone();
        delete Q_img;
    }
    else {
        DEBUG_MODE = false;
        ui->actionDisplay_Contour->setChecked(true);
        workstates = image_only_contour;
        display_image();
    }
}


void MainWindow::on_actionCost_Graph_triggered(bool checked)
{
    if (checked) {
        DEBUG_MODE = true;
        workstates = cost_graph;

        QImage* Q_img = new QImage((const unsigned char*)(image.data),image.cols,image.rows,image.step, QImage::Format_RGB888);
        int w=Q_img->width(), h=Q_img->height();
        QImage png(3*w,3*h,Q_img->format());
        png.fill( qRgb(255,255,255) );

        for(int j=0; j<h; j++) {
            for(int i=0; i<w; i++) {
                pixelNode *pn = pixelnodes[j][i];
                png.setPixel( 3*i+1, 3*j+1, Q_img->pixel(i,j) ); // i, j
                if (i + 1 < w) png.setPixel(3*i+2,3*j+1,qRgb(pn->getLinkCost(0) * 1.5,
                                                             pn->getLinkCost(0) * 1.5,
                                                             pn->getLinkCost(0) * 1.5)); // link 0
                if (i + 1 < w && j - 1 >=0) png.setPixel(3*i+2,3*j,qRgb(pn->getLinkCost(1) * 1.5,
                                                                        pn->getLinkCost(1) * 1.5,
                                                                        pn->getLinkCost(1) * 1.5)); // link 1
                if (j - 1 >= 0) png.setPixel(3*i+1,3*j,qRgb(pn->getLinkCost(2) * 1.5,
                                                            pn->getLinkCost(2) * 1.5,
                                                            pn->getLinkCost(2) * 1.5)); // link 2
                if (i - 1 >=0 && j - 1 >=0) png.setPixel(3*i,3*j,qRgb(pn->getLinkCost(3) * 1.5,
                                                                      pn->getLinkCost(3) * 1.5,
                                                                      pn->getLinkCost(3) * 1.5)); // link 3
                if (i - 1 >= 0) png.setPixel(3*i,3*j+1,qRgb(pn->getLinkCost(4) * 1.5,
                                                            pn->getLinkCost(4) * 1.5,
                                                            pn->getLinkCost(4) * 1.5)); // link 4
                if (i - 1 >= 0 && j + 1 < h) png.setPixel(3*i,3*j+2,qRgb(pn->getLinkCost(5) * 1.5,
                                                                         pn->getLinkCost(5) * 1.5,
                                                                         pn->getLinkCost(5) * 1.5)); // link 5
                if (j + 1 < h) png.setPixel(3*i+1,3*j+2,qRgb(pn->getLinkCost(6) * 1.5,
                                                             pn->getLinkCost(6) * 1.5,
                                                             pn->getLinkCost(6) * 1.5)); // link 6
                if (i + 1 < w && j + 1 < h) png.setPixel(3*i+2,3*j+2,qRgb(pn->getLinkCost(7) * 1.5,
                                                                          pn->getLinkCost(7) * 1.5,
                                                                          pn->getLinkCost(7) * 1.5)); // link 7
            }
        }
        QPixmap p = QPixmap::fromImage(png);
        ui->label->setPixmap(p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio));
        // QImage Q_img_tmp = Q_img->rgbSwapped();

        if (ui->actionPixel_Node->isChecked())
            ui->actionPixel_Node->setChecked(false);
        if (ui->actionPath_Tree->isChecked())
            ui->actionPath_Tree->setChecked(false);
        if (ui->actionMin_Path->isChecked())
            ui->actionMin_Path->setChecked(false);

        cost_graph_image = cv::Mat( png.height(), png.width(),
                                 CV_8UC3,
                                 const_cast<uchar*>(png.bits()),
                                 static_cast<size_t>(png.bytesPerLine())
                                 ).clone();
        delete Q_img;
    }
    else {
        DEBUG_MODE = false;
        ui->actionDisplay_Contour->setChecked(true);
        workstates = image_only_contour;
        display_image();
    }
}


void MainWindow::on_actionPath_Tree_triggered(bool checked)
{
    if (checked) {
        if (!first_seed_flag){
            cout << "first seed not init" << endl;
            return;
        }

        DEBUG_MODE = true;
        workstates = path_tree;
        delete pathTree;
        pathTree = new QImage(drawPathTree());

        QPixmap p = QPixmap::fromImage(*pathTree);
        ui->label->resize( p.width()*img_scale, p.height()*img_scale );
        ui->label->setPixmap( p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio) );

        if (ui->actionPixel_Node->isChecked())
            ui->actionPixel_Node->setChecked(false);
        if (ui->actionCost_Graph->isChecked())
            ui->actionCost_Graph->setChecked(false);
        if (ui->actionMin_Path->isChecked())
            ui->actionMin_Path->setChecked(false);

        path_tree_image = cv::Mat( pathTree->height(), pathTree->width(),
                                 CV_8UC3,
                                 const_cast<uchar*>(pathTree->bits()),
                                 static_cast<size_t>(pathTree->bytesPerLine())
                                 ).clone();
    }
    else {
        DEBUG_MODE = false;
        ui->actionDisplay_Contour->setChecked(true);
        workstates = image_only_contour;
        display_image();
    }

}


void MainWindow::on_actionMin_Path_triggered(bool checked)
{
    if (checked) {
        if (!first_seed_flag){
            cout << "first seed not init" << endl;
            return;
        }

        DEBUG_MODE = true;
        workstates = min_path;
        delete pathTree;
        pathTree = new QImage(drawPathTree());

        QPixmap p = QPixmap::fromImage(*pathTree);
        ui->label->resize( p.width()*img_scale, p.height()*img_scale );
        ui->label->setPixmap( p.scaled(p.width()*img_scale, p.height()*img_scale, Qt::KeepAspectRatio) );

        if (ui->actionPixel_Node->isChecked())
            ui->actionPixel_Node->setChecked(false);
        if (ui->actionCost_Graph->isChecked())
            ui->actionCost_Graph->setChecked(false);
        if (ui->actionMin_Path->isChecked())
            ui->actionMin_Path->setChecked(false);

        min_path_image = cv::Mat( pathTree->height(), pathTree->width(),
                                 CV_8UC3,
                                 const_cast<uchar*>(pathTree->bits()),
                                 static_cast<size_t>(pathTree->bytesPerLine())
                                 ).clone();
    }
    else {
        DEBUG_MODE = false;
        ui->actionDisplay_Contour->setChecked(true);
        workstates = image_only_contour;
        display_image();
    }

}


// overloading QT functions
void MainWindow::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        ctrl_enabled = true;
    }
    return;
}

void MainWindow::keyReleaseEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Control) {
        ctrl_enabled = false;
    }
    return;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {

    // control + left click: first seed
    if ( (event->type() == QEvent::MouseButtonPress) && (ctrl_enabled) &&
         (strcmp(watched->metaObject()->className(), "MainWindow")) == 0 && !first_seed_flag)
    {
        QMouseEvent* me = static_cast<QMouseEvent*> (event);
        QPoint p = ui->label->mapFrom(this, me->pos());
        p /= img_scale;
        if (!scissor_enabled) {
            cout << "scissor is not enabled" << endl;
            return false;
        }

        if (finished_flag) {
            cout << "already finished, reset or save" << endl;
            return false;
        }

        head_node = pixelnodes[p.y()][p.x()];
        if (is_snap) getSnapSeed();
        current_node = head_node;

        // draw a dot
        dots->push_back( QPoint(head_node->getCol(), head_node->getRow()) );
        cv::circle(contour_image, cv::Point(head_node->getCol(),head_node->getRow()), 1, CV_RGB(0,0,255), 2);

        display_image();

        updatePathTree();

        first_seed_flag = true;

    }

    // left click: follwing seeds
    if ( (event->type() == QEvent::MouseButtonPress) && (!ctrl_enabled) &&
         (strcmp(watched->metaObject()->className(), "MainWindow")) == 0 )
    {
        if( DEBUG_MODE )
            return false;

        QMouseEvent* me = static_cast<QMouseEvent*> (event);
        QPoint p = ui->label->mapFrom(this, me->pos());
        p /= img_scale;

        // p.y() - image.rows - down direction
        // p.x() - image.cols - right direction
        if (!scissor_enabled) {
            cout << "scissor is not enabled" << endl;
            return false;
        }

        if (finished_flag) {
            cout << "already finished, reset or save" << endl;
            return false;
        }

        current_node = pixelnodes[p.y()][p.x()];

        vector<QPoint> path;
        getPath(p.x(), p.y(), path);
        dots->push_back(QPoint(p.x(), p.y()));

        // draw the path on the contour image
        cv::circle(contour_image, cv::Point(p.x(),p.y()), 1, CV_RGB(0,0,255), 2);
        for (uint i = 0; i < path.size() - 1; i ++) {
            cv::line(contour_image, cv::Point(path[i].x(), path[i].y()), cv::Point(path[i+1].x(), path[i+1].y()), CV_RGB(173,255,47), 1);
        }

        display_image();

        paths->push_back(path);

        updatePathTree();

    }

    // Mouse move event
    if ( (event->type() == QEvent::MouseMove) && ui->label->underMouse() && (first_seed_flag == true)){
        QMouseEvent* me = static_cast<QMouseEvent*> (event);
        QPoint p = me->pos();
        p /= img_scale;

        QString myText = QString("Intelligent Scissor ");
        statusBar()->showMessage(QString("(%1, %2) ").arg(p.x()).arg(p.y()) + myText);

        if (!scissor_enabled) {
            cout << "scissor not enabled" << endl;
            return false;
        }

        if (finished_flag) {
            cout << "already finished, reset or save" << endl;
            return false;
        }


        // different modes of mouse move event
        if (workstates != path_tree && workstates != min_path){
            // check boundary, this causes the crash
            if ( (p.y() > 0) && (p.x() > 0) &&
                 (p.y() < image.rows-1) && (p.x() < image.cols-1)){
                vector<QPoint> path;
                getPath(p.x(), p.y(), path);
                tmp_contour = contour_image.clone();

                if (path.size() < 1) return false;
                for (uint i = 0; i < path.size() - 1; i ++) {
                    cv::line(tmp_contour, cv::Point(path[i].x(), path[i].y()),
                                          cv::Point(path[i+1].x(), path[i+1].y()), CV_RGB(173,255,47), 1);
                }
                dots_deleted = false;

                display_image(tmp_contour);
            }
        }
        else if (workstates == min_path){
            // check boundary, this causes the crash
            if ( (p.y() > 0) && (p.x() > 0) &&
                 (p.y() < 3*image.rows-1) && (p.x() < 3*image.cols-1)){
                vector<QPoint> path;
                getPath( (p.x()-1)/3, (p.y()-1)/3, path );
                tmp_contour = min_path_image.clone();

                if (path.size() < 1) return false;
                for (uint i = 0; i < path.size() - 1; i ++) {
                    cv::line(tmp_contour, cv::Point(3*path[i].x()+1, 3*path[i].y()+1),
                                          cv::Point(3*path[i+1].x()+1, 3*path[i+1].y()+1), CV_RGB(0,0,255), 3); // red line
                }
                dots_deleted = false;

                display_image(tmp_contour);
            }
        }

    }


    if ( event->type() == QEvent::KeyPress){
        QKeyEvent* event_key = static_cast<QKeyEvent*> (event);

        // enter: finish the current
        if ( (event_key->key() == Qt::Key_Enter || event_key->key() == Qt::Key_Return) && (!ctrl_enabled) ){
            ui->actionScissor->setChecked(false);
            finished_flag = true;
        }

        // ctrl + enter: finished as closed
        if ( (event_key->key() == Qt::Key_Enter || event_key->key() == Qt::Key_Return) && ctrl_enabled ){
            finished_flag = true;
            vector<QPoint> path;
            getPath(head_node->getCol(), head_node->getRow(), path);
            paths->push_back(path);
            for (uint i = 0; i < path.size() - 1; i ++) {
                cv::line(contour_image, cv::Point(path[i].x(), path[i].y()), cv::Point(path[i+1].x(), path[i+1].y()), CV_RGB(173,255,47), 1);
            }

            display_image();

            finished_asclosed = true;

        }

        // backspace when scissoring
        if ( (event_key->key() == Qt::Key_Backspace) && scissor_enabled && (!dots_deleted) ){
            contour_image = image.clone();
            if (dots->size() > 0) {
                dots->pop_back();
                dots_deleted = true;
                if (dots->size() > 0) {
                    current_node = pixelnodes[dots->back().y()][dots->back().x()];
                    updatePathTree();
                } else {
                    this->on_actionReset_Contour_triggered();
                    return false;
                }
                for (uint i = 0; i < dots->size(); i ++) {
                    cv::circle(contour_image, cv::Point(dots->at(i).x(), dots->at(i).y()), 1, CV_RGB(0,0,255), 2);
                }
            }

            if (paths->size() > 0) {
                paths->pop_back();
                if (paths->size() == 0) return false;
                for (uint i = 0; i < paths->size(); i++) {
                    vector<QPoint> iter = paths->at(i);
                    for (uint j = 0; j < iter.size() - 1; j ++) {
                        cv::line(contour_image, cv::Point(iter[j].x(), iter[j].y()),
                                                cv::Point(iter[j+1].x(), iter[j+1].y()), CV_RGB(173,255,47), 1);
                    }
                }
            }

        }

        // backspace when not scissoring
        if ( (event_key->key() == Qt::Key_Backspace) && (!scissor_enabled) ){
            this->on_actionReset_Contour_triggered();
        }
    }

    return false;
}

void MainWindow::updatePathTree()
{
    int h = Qimg->height();
    int w = Qimg->width();

    for(int j=0;j<h;j++){
        for(int i=0;i<w;i++)
        {
                pixelnodes[j][i]->totalCost = DBL_MAX;
                pixelnodes[j][i]->state = pixelNode::INITIAL;
        }
    }

     current_node->resetTotalCost(0);
     current_node->resetPrevNode();

     FibHeap heap;
     heap.Insert(current_node);
     pixelNode* q;

     while (heap.Minimum() != NULL) {

         q = (pixelNode*) heap.ExtractMin();
         q->state = pixelNode::EXPANDED;
         int c, r;  // col and row

         for (int i = 0; i < 8 ;i ++) {
             q->Neighbor(i, c, r);

             // boundary check
             if (c >= 0  && c < Qimg->width() && r >= 0 && r < Qimg->height()) {
                 pixelNode* pn = pixelnodes[r][c];

                 // if pn has not been expanded
                 if (pn->state != pixelNode::EXPANDED) {
                     double linkcost = q->getLinkCost(i);

                     if (pn->state == pixelNode::INITIAL) {
                         pn->setParent(q);
                         pn->totalCost = linkcost + q->totalCost;
                         pn->state = pixelNode::ACTIVE;
                         heap.Insert(pn);
                     } else if (pn->state == pixelNode::ACTIVE) {
                         if (linkcost + q->totalCost < pn->totalCost) {
                             pn->setParent(q);
                             pixelNode newpn = pixelNode(pn->getCol(), pn->getRow());
                             newpn.totalCost = linkcost + q->totalCost;
                             heap.DecreaseKey(pn, newpn);
                         }
                     }

                 }
             }

         }  // end of Neighbor for loop
     }  // end of while loop
}


// Gaussian filter Smooth
void MainWindow::on_actionGuassian_3_triggered()
{
    if ( !image.empty() ){
        cv::GaussianBlur(image, image, cv::Size(3, 3), 3, 3);
        contour_image = image.clone();

        delete Qimg;
        Qimg = new QImage((const unsigned char*)(image.data),image.cols,image.rows, image.step, QImage::Format_RGB888);
        computeCostFunc();

        delete Mask;
        Mask = new QImage(Qimg->width(),Qimg->height(),Qimg->format());
        Mask->fill(qRgb(0, 0, 0));

        display_image();
    }
}

void MainWindow::on_actionGaussian_5_triggered()
{
    if ( !image.empty() ){
        cv::GaussianBlur(image, image, cv::Size(5, 5), 5, 5);
        contour_image = image.clone();

        delete Qimg;
        Qimg = new QImage((const unsigned char*)(image.data),image.cols,image.rows,image.step, QImage::Format_RGB888);
        computeCostFunc();

        delete Mask;
        Mask = new QImage(Qimg->width(),Qimg->height(),Qimg->format());
        Mask->fill(qRgb(0, 0, 0));

        display_image();
    }
}

void MainWindow::on_actionSnapSeed_triggered(bool checked)
{
    is_snap = checked;
}



/*
void MainWindow::on_actionGaussian_5_triggered(bool checked){

    if (!image.empty() && checked){
        if (!ui->actionGuassian_3->isChecked()) {
            previous_image = image.clone();
        } else {
            ui->actionGuassian_3->setChecked(false);
            image = previous_image.clone();
        }
        cv::GaussianBlur(image, image, cv::Size(5, 5), 5, 5);
        contour_image = image.clone();
        current_image = image;
        computeCostFunc();

        delete Mask;
        Mask = new QImage(Qimg->width(),Qimg->height(),Qimg->format());
        Mask->fill(qRgb(0, 0, 0));
        display_image(image);
    }
    else if (!image.empty() && !checked) {
        image = previous_image;
        current_image = previous_image;
        computeCostFunc();

        delete Mask;
        Mask = new QImage(Qimg->width(),Qimg->height(),Qimg->format());
        Mask->fill(qRgb(0, 0, 0));
        display_image(current_image);
    }
}
*/



/*
void MainWindow::draw_contour(int x, int y){

    int draw_value;
    cv::Vec3b fill(0,255,0);
    //contour = cv::Mat::zeros(image.size(), CV_8UC3);
    contour_image = image.clone();

    while( this->parentMap.at<uchar>( cv::Point(x,y) ) != 255 ){

        draw_value = this->parentMap.at<uchar>( cv::Point(x,y) );

        contour_image.at<cv::Vec3b>( cv::Point(x,y) ) = fill;
        //contour.at<cv::Vec3b>( cv::Point(x,y) ) = fill;

        //cout << "draw value is: " << draw_value << endl;

        //  0   1   2
        //  3   4   5
        //  6   7   8
        switch (draw_value) {
        case 0:
            y -= 1;
            x -= 1;
            continue;
        case 1:
            y -= 1;
            continue;
        case 2:
            y -= 1;
            x += 1;
            continue;
        case 3:
            x -= 1;
            continue;
        case 4:
            cout << "self loop detected" << endl;
            exit(1);
            break;
        case 5:
            x += 1;
            continue;
        case 6:
            y += 1;
            x -= 1;
            continue;
        case 7:
            y += 1;
            continue;
        case 8:
            y += 1;
            x += 1;
            continue;
        //default:
        //    continue;
        }
    }
}


// compute 8 number of link graph, which could be accessed separatelyss
void MainWindow::costgraph_init(){
    if( scissor_enabled == false){
        cout << " scissor not enabled" << endl;
        return;
    }

    //  0   1   2
    //  3   4   5
    //  6   7   8
    // Kernel[4] specify self-loop to be inf cost
    // this actualy could be ignored in the main while loop, since we only explore 8 neightbors.
    cv::Mat Kernel[9];

    Kernel[0] = (cv::Mat_<float>(3,3)  <<      0,  0.707,      0,
                                          -0.707,      0,      0,
                                               0,      0,      0  );
    Kernel[1] = (cv::Mat_<float>(3,3)<<    -0.25,      0,   0.25,
                                           -0.25,      0,   0.25,
                                               0,      0,      0  );
    Kernel[2] = (cv::Mat_<float>(3,3)<<        0,  0.707,      0,
                                               0,      0, -0.707,
                                               0,      0,      0  );
    Kernel[3] = (cv::Mat_<float>(3,3)<<     0.25,   0.25,      0,
                                               0,      0,      0,
                                           -0.25,  -0.25,      0  );
    Kernel[4] = (cv::Mat_<float>(3,3)<<        0,      0,      0,
                                               0, 100000,      0,
                                               0,      0,      0  );
    Kernel[5] = (cv::Mat_<float>(3,3)<<        0,  -0.25,  -0.25,
                                               0,      0,      0,
                                               0,   0.25,   0.25  );
    Kernel[6] = (cv::Mat_<float>(3,3)<<        0,      0,      0,
                                          -0.707,      0,      0,
                                               0,  0.707,      0  );
    Kernel[7] = (cv::Mat_<float>(3,3)<<        0,      0,      0,
                                           -0.25,      0,  -0.25,
                                            0.25,      0,  -0.25  );
    Kernel[8] = (cv::Mat_<float>(3,3)<<        0,      0,      0,
                                               0,      0,  0.707,
                                               0, -0.707,      0  );

    cv::Mat temp;
    float maxD = -1.0;

    // compute 8 direction cost graphs separately
    for(int i=0; i<=8; i++){
        cv::filter2D(image, temp, CV_32FC3, Kernel[i]);

        vector<cv::Mat> rgbChannels(3);
        cv::split(temp, rgbChannels);

        cv::Mat tmp = cv::Mat::zeros(image.rows,image.cols,CV_32FC1);

        cv::Mat B = rgbChannels[0];
        cv::Mat R = rgbChannels[0];
        cv::Mat G = rgbChannels[0];

        B = 255 - B;
        R = 255 - R;
        G = 255 - G;

        // elementwise multiplication
        tmp = B.mul(B) + R.mul(R) + G.mul(G);
        tmp.copyTo(costgraph_weight[i]);

        // cout << "tmp is: \n\n\n\n\n\n\n" << tmp << endl;
        // cv::imshow("test", tmp);
    }

    for (int i = 0; i < costgraph_weight->rows; i ++) {
        for (int j = 0; j < costgraph_weight->cols; j ++) {
            if (costgraph_weight->at<float>(cv::Point(i,j)) > maxD) {
                maxD = costgraph_weight->at<float>(cv::Point(i,j));
            }
        }
    }


    for (int i = 0; i < costgraph_weight->rows; i ++) {
        for (int j = 0; j < costgraph_weight->cols; j ++) {
            cout << costgraph_weight->at<float>(cv::Point(i,j)) << endl;
            costgraph_weight->at<float>(cv::Point(i,j)) = maxD - costgraph_weight->at<float>(cv::Point(i,j));
            cout << costgraph_weight->at<float>(cv::Point(i,j)) << endl;
        }
    }

}


// main algorithm
// find shortest path from current click seed
void MainWindow::Dijstras(pixelNode* seed){

    // initialize the priority queue pq to be empty;
    priority_queue<pixelNode*, std::vector<pixelNode*>, compareQueue> pqueue;

    // initialize each node to the INITIAL state;
    visitedMap = cv::Mat::zeros(image.size(), CV_8UC1);
    parentMap = cv::Mat::zeros(image.size(), CV_8UC1);
    //activeMap = cv::Mat::zeros(image.size(), CV_8UC1);

    // set the total cost of seed to be zero and make seed the root of the minimum path tree ( pointing to NULL ) ;
    // parentMap stores the parent of each pixel, 255: root, others listed below

    graphCost = cv::Mat::ones(image.size(), CV_32F);
    graphCost *= 100000000.0;
    graphCost.at<float>(cv::Point(seed->getCol(),seed->getRow())) = 0.0;
    parentMap.at<uchar>(cv::Point(seed->getCol(),seed->getRow())) = 255;

    // insert seed into pq;
    pqueue.push(seed);

    // init
    cv::Point r;
    int costgraph_index;
    cv::Mat costgraph_tmp;


    // while pq is not empty
    while(!pqueue.empty()){

        // extract the node q with the minimum total cost in pq;
        pixelNode* q = pqueue.top();
        pqueue.pop();

        while (visitedMap.at<uchar>(cv::Point(q->getCol(), q->getRow())) == 1) {
            q = pqueue.top();
            pqueue.pop();
            if (pqueue.empty()){
                cout << "every node explored" << endl;
                break;
            }
        }

        // mark q as EXPANDED;
        visitedMap.at<uchar>(cv::Point(q->getCol(),q->getRow())) = 1;


        // for each neighbor node r of q
        for(int i=-1; i<=1; i++){

            for(int j=-1; j<=1; j++){

                if( i==0 && j==0 )
                    continue;

                // note the i, j
                r = cv::Point( q->getCol()+j, q->getRow()+i );

                // if r has been EXPANDED
                if (visitedMap.at<uchar>(r) == 1)
                    continue;

                costgraph_index = (i+1)*3 + (j+1);
                costgraph_tmp = costgraph_weight[costgraph_index];

                float oldCost = graphCost.at<float>(r);
                float newCost = graphCost.at<float>(cv::Point(q->getCol(),q->getRow()))
                                + costgraph_tmp.at<float>(cv::Point(q->getCol(),q->getRow()));

                // cout << "newCost is: " << newCost << endl;

                if (newCost < oldCost){
                    // cout << "update" << endl;

                    graphCost.at<float>(r) = newCost;

                    parentMap.at<uchar>(r) = (1-i)*3 + (1-j);

                    // check boundary
                    if ( (q->getCol()+j) > 0 && (q->getRow()+i) > 0 &&
                         (q->getCol()+j) < (image.cols-1) && (q->getRow()+i) < (image.rows-1)) {
                        pqueue.push(new pixelNode(r.x, r.y) );
                    }
                }
            }
        } // end of neightbor search loop

        // satety check
        if (pqueue.empty()){
            cout << "every node explored" << endl;
            break;
        }

    } // end of while loop

}
*/





