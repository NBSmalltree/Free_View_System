# Multi_View_System
This program is about to play any virtual view according to the reference views' color and depth video. The UI interface is developed with Qt library.

  - **Input :** 
    - Multiview YUV Video Sequence(User only have to pick one, the software will automatically recognize others.)
  - **Output :**
    - Certain View with certain Effect
  - **Function :**
	- YUV Player
	- Play Control
	  - Single Frame Forward
	  - Single Frame Backward
	  - Multiframe Forward
	  - Multiframe Backward
	  - Play Forward
	  - Play Backward
	- Screen Capture
	- Video Effect
	  - Zoom in\out
	  - GRAY
	  - DILATE
	  - ERODE
	  - HISTOGRAM
	  - EDGE DETECT
	  - WATERSHED
	  
# 多视点视频播放系统

利用多视点的YUV视频序列，基于OpenCV和Qt开发出一套能自由切换视点的播放系统，构建了一个较为完善的YUV类，实现了逐帧、连续播放及回退、截图、自由缩放等功能，并应用了膨胀、腐蚀、直方图均衡、二值化、边缘检测、分水岭分割等视频特效算法。

  - **输入 :** 
    - 多视点YUV视频序列(用户只需输入同一文件夹下的一个视频，其他相同命名格式的文件由软件自动读取)
  - **输出 :**
    - 特定视点特定特效的视频流
  - **实现功能 :**
	- YUV视频播放
	- 播放控制
	  - 单帧步进
	  - 单帧步退
	  - 多帧步进
	  - 多帧步退
	  - 播放
	  - 回放
	- 截屏
	- 视频特效
	  - 放大缩小
	  - 灰度化
	  - 膨胀
	  - 腐蚀
	  - 直方图均衡
	  - 边缘检测
	  - 分水岭分割
	
## 主界面

文件打开后加载的初始化界面
![](/doc/images/001.png)

文件播放并切换视点后界面
![](/doc/images/002.png)

## 视频特效

### 放大、缩小
![](/doc/images/003.png)
![](/doc/images/004.png)

### 灰度化
![](/doc/images/005.png)

### 膨胀
![](/doc/images/006.png)

### 腐蚀
![](/doc/images/007.png)

### 直方图均衡
![](/doc/images/008.png)

### 边缘检测
![](/doc/images/009.png)

### 分水岭分割
![](/doc/images/010.png)

本软件仍处于开发状态，有好的想法欢迎一起完善，联系我(tomruibin@163.com)

**开发环境：**

[**Visual studio 2015 com**](https://www.visualstudio.com/);
[**QT5.6.3**](https://www.qt.io/qt5-6/)
[**Opencv2.4.13**](https://opencv.org/releases.html)
