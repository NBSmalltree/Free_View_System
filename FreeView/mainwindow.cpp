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

void MainWindow::Gray(cv::Mat * src, cv::Mat * dst)
{
	cv::Mat grayImage;
	cv::cvtColor(*src, grayImage, cv::COLOR_BGR2GRAY);
	cv::cvtColor(grayImage, *dst, cv::COLOR_GRAY2BGR);
}

void MainWindow::Dilate(cv::Mat * src, cv::Mat * dst)
{
	//>getStructuringElement函数返回的是指定形状和尺寸的结构元素
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
	cv::dilate(*src, *dst, element);
}

void MainWindow::Erode(cv::Mat * src, cv::Mat * dst)
{
	//>getStructuringElement函数返回的是指定形状和尺寸的结构元素
	cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(7, 7));
	cv::erode(*src, *dst, element);
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

void MainWindow::edgeDetect(cv::Mat * src, cv::Mat * dst)
{
	cv::Mat edge, grayImage, outImage;
	cv::cvtColor(*src, grayImage, cv::COLOR_BGR2GRAY);

	//>先使用3*3内核来降噪
	cv::blur(grayImage, edge, cv::Size(3, 3));

	//>运行canny算子
	cv::Canny(edge, outImage, 30, 90);

	cv::cvtColor(outImage, *dst, cv::COLOR_GRAY2BGR);
}

//>描述：生成随机颜色函数;
cv::Vec3b RandomColor(int value)
{
	value = value % 255;  //>生成0~255的随机数  
	cv::RNG rng;
	int aa = rng.uniform(0, value);
	int bb = rng.uniform(0, value);
	int cc = rng.uniform(0, value);
	return cv::Vec3b(aa, bb, cc);
}

void MainWindow::waterShed(cv::Mat * src, cv::Mat * dst)
{
	//>灰度化，滤波，Canny边缘检测
	cv::Mat imageGray;
	cvtColor(*src, imageGray, cv::COLOR_BGR2GRAY);

	cv::GaussianBlur(imageGray, imageGray, cv::Size(5, 5), 2);

	cv::Canny(imageGray, imageGray, 60, 140);

	//>查找轮廓
	cv::vector<cv::vector<cv::Point>> contours;
	cv::vector<cv::Vec4i> hierarchy;
	cv::findContours(imageGray, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE, cv::Point());
	cv::Mat imageContours = cv::Mat::zeros(src->size(), CV_8UC1);  //>轮廓
	cv::Mat marks(src->size(), CV_32S);   //>OpenCV分水岭第二个矩阵参数
	marks = cv::Scalar::all(0);
	int index = 0;
	int compCount = 0;
	for (; index >= 0; index = hierarchy[index][0], compCount++) {
		//>对marks进行标记，对不同区域的轮廓进行编号，相当于设置注水点，有多少轮廓，就有多少注水点  
		cv::drawContours(marks, contours, index, cv::Scalar::all(compCount + 1), 1, 8, hierarchy);
		cv::drawContours(imageContours, contours, index, cv::Scalar(255), 1, 8, hierarchy);
	}

	cv::watershed(*src, marks);

	//>对每一个区域进行颜色填充
	cv::Mat PerspectiveImage = cv::Mat::zeros(src->size(), CV_8UC3);
	for (int i = 0; i < marks.rows; i++) {
		for (int j = 0; j < marks.cols; j++) {
			int index = marks.at<int>(i, j);
			if (marks.at<int>(i, j) == -1)
				PerspectiveImage.at<cv::Vec3b>(i, j) = cv::Vec3b(255, 255, 255);
			else
				PerspectiveImage.at<cv::Vec3b>(i, j) = RandomColor(index);
		}
	}

	//>分割并填充颜色的结果跟原始图像融合
	cv::Mat wshed;
	addWeighted(*src, 0.6, PerspectiveImage, 0.4, 0, *dst);
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
		case GRAY:
			Gray(&bgr, &dst);
			dst.copyTo(bgr);
			break; 
		case DILATE:
			Dilate(&bgr, &dst);
			dst.copyTo(bgr);
			break;
		case ERODE:
			Erode(&bgr, &dst);
			dst.copyTo(bgr);
			break;
		case HISTOGRAM:
			histogramEqualiztion(&bgr, &dst);
			dst.copyTo(bgr);
			break;
		case EDGE_DETECT:
			edgeDetect(&bgr, &dst);
			dst.copyTo(bgr);
			break;
		case WATERSHED:
			waterShed(&bgr, &dst);
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

void MainWindow::on_radioButton_Gray_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = GRAY;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_Dilate_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = DILATE;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_Erode_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = ERODE;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_Histogram_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = HISTOGRAM;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_EdgeDetect_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = EDGE_DETECT;
	updateScreen(frameNumber);
}

void MainWindow::on_radioButton_WaterShed_clicked()
{
	if (is_Open == false)
		return;

	selectEffect = WATERSHED;
	updateScreen(frameNumber);
}
