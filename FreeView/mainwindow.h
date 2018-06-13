#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTimeLine>
#include <QtWidgets/QDialog>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/legacy/legacy.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/photo/photo.hpp>
#include <iostream>
#include <string>
#include "yuv.h"
#include "mAbout.h"

enum specialEffects {
	NONE = 0,
	GRAY = 1,
	DILATE = 2,
	ERODE = 3,
	HISTOGRAM = 4,
	EDGE_DETECT = 5,
	WATERSHED = 6
};

namespace Ui {
	class MainWindow;
}
class CIYuv;
class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_Button_Play_clicked();

	void on_Button_RePlay_clicked();

	void on_radioButton_View1_clicked();

	void on_radioButton_View2_clicked();

	void on_radioButton_View3_clicked();

	void on_radioButton_View4_clicked();

	void on_radioButton_View5_clicked();

	void on_radioButton_None_clicked();

	void on_radioButton_Gray_clicked(); 
	
	void on_radioButton_Dilate_clicked();

	void on_radioButton_Erode_clicked();

	void on_radioButton_Histogram_clicked();

	void on_radioButton_EdgeDetect_clicked();

	void on_radioButton_WaterShed_clicked();

	void on_SUBMENU_OPEN_triggered();

	void on_SUBMENU_CAPTURE_triggered();

	void on_SUBMENU_ABOUT_triggered();

	void on_Button_StepForward_clicked();

	void on_Button_StepBackward_clicked();

	void on_Button_MultiStepForward_clicked();

	void on_Button_MultiStepBackward_clicked();

	void on_Button_ResetToStart_clicked();

	void on_Button_ResetToEnd_clicked();

	void updateScreen(int poc);

private:
	Ui::MainWindow *ui;
	bool is_Open = false;
	bool is_Play = false;

	int viewNumber = 1;
	specialEffects selectEffect = NONE;

	QString FliePath[6]; //QString YuvName;
	FILE *viewAddress;
	CIYuv *yuvBuffer = NULL;

	int width = 1024;
	int high = 768;

	int frameNumber = 0;
	QGraphicsScene ms;
	QTimeLine mtime;
	
	mAbout *myAbout;

	void Gray(cv::Mat * src, cv::Mat * dst);

	void Dilate(cv::Mat * src, cv::Mat * dst);

	void Erode(cv::Mat * src, cv::Mat * dst);

	void histogramEqualiztion(cv::Mat * src, cv::Mat * dst);

	void edgeDetect(cv::Mat * src, cv::Mat * dst);

	void waterShed(cv::Mat * src, cv::Mat * dst);

protected:
	void wheelEvent(QWheelEvent *event);
	

};

#endif // MAINWINDOW_H
