#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QString>
#include <QFileDialog>
#include <QMouseEvent>


#define CLIP3(x, min, max) ((x)<(min)?(min):((x)>(max)?(max):(x)))

//IplImage *bgr = NULL;

cv::Mat bgr(768, 1024, CV_8UC3);

double curScale = 1.0;
const double ScrollStep = 0.1;
//--------------------------------------KEY_FLAGS-----------------------------------//
bool is_BackwardPlay = false;
int presentFrame_BackwardPlay = 0;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	myAbout(new mAbout(this))
{
	mtime.stop();
	ui->setupUi(this);
	ui->graphicsView->setScene(&ms);
	connect(&mtime, SIGNAL(frameChanged(int)), this, SLOT(updateScreen(int)));
}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::on_Button_Play_clicked()
{
	if (is_Open == false)
		return;

	is_BackwardPlay = false;
	if (is_Play == false) {
		is_Play = true;
		ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\232\202\345\201\234", Q_NULLPTR)); //暂停;
		ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\346\232\202\345\201\234", Q_NULLPTR)); //暂停;

		mtime.setFrameRange(frameNumber, 100);
		double totalSec = (100 - frameNumber) / (double)25;
		mtime.setDuration(totalSec * 1000);
		mtime.setCurveShape(QTimeLine::LinearCurve);
		mtime.start();
	}
	else {
		is_Play = false;
		ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\222\255\346\224\276", Q_NULLPTR)); //播放;
		ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\345\233\236\346\224\276", Q_NULLPTR)); //回放;

		mtime.stop();
		//frameNumber = mtime.currentFrame();
	}
}

void MainWindow::on_Button_RePlay_clicked()
{
	if (is_Open == false)
		return;

	if (is_Play == false) {
		is_BackwardPlay = true;
		is_Play = true;
		ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\232\202\345\201\234", Q_NULLPTR));
		ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\346\232\202\345\201\234", Q_NULLPTR));

		presentFrame_BackwardPlay = frameNumber;
		mtime.setFrameRange(0, frameNumber);
		double totalSec = frameNumber / (double)25;
		mtime.setDuration(totalSec * 1000);
		mtime.setCurveShape(QTimeLine::LinearCurve);
		mtime.start();
	}
	else {
		is_Play = false;
		is_BackwardPlay = false;
		ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\222\255\346\224\276", Q_NULLPTR));
		ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\345\233\236\346\224\276", Q_NULLPTR));

		mtime.stop();
	}
}

// Histogram Equalization;
void MainWindow::histogramEqualiztion(cv::Mat *src, cv::Mat *dst)
{
	cv::vector<cv::Mat> channels;
	cv::Mat imageBlueChannel;
	cv::Mat imageGreenChannel;
	cv::Mat imageRedChannel;

	split(*src, channels);
	imageBlueChannel = channels.at(0);
	imageGreenChannel = channels.at(1);
	imageRedChannel = channels.at(2);

	equalizeHist(imageBlueChannel, imageBlueChannel);
	equalizeHist(imageGreenChannel, imageGreenChannel);
	equalizeHist(imageRedChannel, imageRedChannel);

	merge(channels, *dst);
}

void MainWindow::updateScreen(int poc)
{
	// Determine whether or not is BackwardPlay;
	if (is_BackwardPlay)
		frameNumber = presentFrame_BackwardPlay - poc;
	else 
		frameNumber = poc;

	// Read a frame from YUV file and transfer to RGB format(For QImage class read);
	yuvBuffer->ReadOneFrame(viewAddress, frameNumber);

	//////////////////////////////////////////////////////////////////////////test;
	if (selectEffect != NONE)
	{
		yuvBuffer->getData_inBGR(&bgr);
		cv::Mat dst(bgr);
		switch (selectEffect)
		{
		case BINARYZATION:
			histogramEqualiztion(&bgr, &dst);
			dst.copyTo(bgr);
			break;
		default:
			break;
		}

		yuvBuffer->setDataFromImgBGR(&bgr);
	}
	
	//////////////////////////////////////////////////////////////////////////testover;
	
	yuvBuffer->getData_inRGB(&bgr);

	auto rgbformat = QImage::Format_RGB888;
	QImage mImage = QImage((uchar*)(bgr.data), width, high, rgbformat);
	ms.clear();
	ms.addPixmap(QPixmap::fromImage(mImage));
	//ui->graphicsView->update();
	ui->progressBar->setValue(frameNumber);
	
	//frameNumber = poc;
}

void MainWindow::wheelEvent(QWheelEvent * event)
{
	int numDegrees = event->delta() / 120.0f;
	double scaf = numDegrees > 0 ? ScrollStep : -ScrollStep;
	double lastscale = curScale;
	curScale = curScale + scaf;
	curScale = (double)CLIP3(curScale,0.25, 4);
	ui->graphicsView->scale(1 / lastscale, 1 / lastscale);
	ui->graphicsView->scale(curScale, curScale);
}

void MainWindow::on_SUBMENU_OPEN_triggered()
{
	QFileDialog dialog(this, QStringLiteral("yuv文件(只需选择第一个视点即可)"));
	dialog.setDirectory(".");
	dialog.setNameFilter(QStringLiteral("YUV file (*.yuv)"));//文件格式设置;
	//QString FliePath[6]; //QString YuvName;
	if (dialog.exec() != QDialog::Accepted && dialog.selectedFiles().isEmpty())
		return;
	FliePath[0] = dialog.selectedFiles().first();
	FliePath[5] = FliePath[4] = FliePath[3] = FliePath[2] = FliePath[1] = FliePath[0];
	FliePath[1].chop(5);
	FliePath[1].append("1.yuv");
	FliePath[2].chop(5);
	FliePath[2].append("2.yuv");
	FliePath[3].chop(5);
	FliePath[3].append("3.yuv");
	FliePath[4].chop(5);
	FliePath[4].append("4.yuv");
	FliePath[5].chop(5);
	FliePath[5].append("5.yuv");
	//假设yuv文件路径没有任何问题;
	//不做异常处理;
	if (yuvBuffer)
	{
		yuvBuffer = new CIYuv(768, 1024, 420);
	}
	else
	{
		delete yuvBuffer;
		yuvBuffer = NULL;
		yuvBuffer = new CIYuv(768, 1024, 420);
	}

	viewAddress = fopen(FliePath[0].toStdString().c_str(), "rb");
	frameNumber = 0;
	updateScreen(frameNumber);

	is_Open = true;
}

void MainWindow::on_SUBMENU_CAPTURE_triggered()
{
	if (is_Open == false)
		return;

	yuvBuffer->ReadOneFrame(viewAddress, frameNumber);
	/*if (bgr)
	{
		cvReleaseImage(&bgr);
		bgr = NULL;
	}
	bgr = cvCreateImage(cvSize(1024, 768), 8, 3);*/
	yuvBuffer->getData_inBGR(&bgr);

	char str[5];
	std::string outputName = "Capture_Image_View_";
	itoa(viewNumber, str, 10);
	outputName += str;
	outputName += "_Frame_";
	itoa(frameNumber + 1, str, 10);
	outputName += str;
	outputName += ".bmp";
	
	cv::imwrite(outputName, bgr);
}

void MainWindow::on_SUBMENU_ABOUT_triggered()
{
	//mAbout->show();
	
	myAbout->show();
}

void MainWindow::on_Button_StepForward_clicked()
{
	if (is_Open == false)
		return;

	frameNumber++;
	frameNumber = CLIP3(frameNumber, 0, 100);
	updateScreen(frameNumber);
}

void MainWindow::on_Button_StepBackward_clicked()
{
	if (is_Open == false)
		return;

	frameNumber--;
	frameNumber = CLIP3(frameNumber, 0, 100);
	updateScreen(frameNumber);
}

void MainWindow::on_Button_MultiStepForward_clicked()
{
	if (is_Open == false)
		return;

	frameNumber += 5;
	frameNumber = CLIP3(frameNumber, 0, 100);
	updateScreen(frameNumber);
}

void MainWindow::on_Button_MultiStepBackward_clicked()
{
	if (is_Open == false)
		return;

	frameNumber -= 5;
	frameNumber = CLIP3(frameNumber, 0, 100);
	updateScreen(frameNumber);
}

void MainWindow::on_Button_ResetToStart_clicked()
{
	if (is_Open == false)
		return;

	is_Play = false;
	is_BackwardPlay = false;
	mtime.stop();
	
	ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\222\255\346\224\276", Q_NULLPTR)); //播放;
	ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\345\233\236\346\224\276", Q_NULLPTR)); //回放;
	
	frameNumber = 0;
	updateScreen(frameNumber);
}

void MainWindow::on_Button_ResetToEnd_clicked()
{
	if (is_Open == false)
		return;

	is_Play = false;
	is_BackwardPlay = false;
	mtime.stop();

	ui->Button_Play->setText(QApplication::translate("MainWindow", "\346\222\255\346\224\276", Q_NULLPTR)); //播放;
	ui->Button_RePlay->setText(QApplication::translate("MainWindow", "\345\233\236\346\224\276", Q_NULLPTR)); //回放;

	frameNumber = 99;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_View1_clicked()
{
	if (is_Open == false)
		return;

	viewNumber = 1;
	viewAddress = fopen(FliePath[viewNumber].toStdString().c_str(), "rb");

	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_View2_clicked()
{
	if (is_Open == false)
		return;

	viewNumber = 2;
	viewAddress = fopen(FliePath[viewNumber].toStdString().c_str(), "rb");

	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_View3_clicked()
{
	if (is_Open == false)
		return;

	viewNumber = 3;
	viewAddress = fopen(FliePath[viewNumber].toStdString().c_str(), "rb");

	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_View4_clicked()
{
	if (is_Open == false)
		return;

	viewNumber = 4;
	viewAddress = fopen(FliePath[viewNumber].toStdString().c_str(), "rb");

	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_View5_clicked()
{
	if (is_Open == false)
		return;

	viewNumber = 5;
	viewAddress = fopen(FliePath[viewNumber].toStdString().c_str(), "rb");

	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_None_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = NONE;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_Binaryzation_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = BINARYZATION;
	updateScreen(frameNumber);
}
