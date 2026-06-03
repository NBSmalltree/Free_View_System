# Multi_View_System (Free_View_System)

`Multi_View_System` is a professional desktop application designed for playing, processing, and analyzing multi-view YUV video sequences. Developed using C++ with the Qt framework and OpenCV library, it provides a seamless experience for navigating through different virtual viewpoints while applying real-time image processing effects.

## 🚀 Key Features (English)

### 1. Advanced YUV Processing
*   **Comprehensive CIYuv Class**: A robust custom class designed to handle various YUV sampling formats (e.g., 4:2:0, 4:4:4).
*   **Format Conversion**: High-performance conversion between YUV and BGR/RGB color spaces.
*   **Up-sampling Engine**: Features a configurable up-sampling filter with adjustable precision for high-quality view synthesis.

### 2. Flexible Viewpoint & Playback Control
*   **Dynamic View Switching**: Synchronously loads multiple views and allows real-time switching between viewpoints during playback.
*   **Precision Frame Navigation**:
    *   Continuous forward/backward playback.
    *   Single-frame step (Forward/Backward).
    *   Multi-frame skip for rapid navigation.
*   **Interactive Visualization**: Supports intuitive zooming in and out of the video stream via the mouse wheel.

### 3. Real-time Video Effects
Integrated various computer vision algorithms for instant processing:
*   **Base Processing**: Grayscale conversion.
*   **Morphological Operations**: Dilation and Erosion.
*   **Image Enhancement**: Histogram Equalization for better contrast.
*   **Analysis Tools**: Canny-based Edge Detection and Watershed segmentation.

### 4. Workflow Enhancements
*   **Screen Capture**: One-click capture of current frames to high-quality images.
*   **Automatic Sequence Discovery**: Smart file naming recognition allows the software to automatically load all associated views by opening just one file.

## 📂 Technical Stack
*   **Language**: C++ (MSVC 2015)
*   **GUI Framework**: Qt 5.6.3
*   **Vision Library**: OpenCV 2.4.13
*   **Platform**: Windows (Visual Studio 2015)

---

# 多视点视频播放系统 (中文)

利用多视点的YUV视频序列，基于OpenCV和Qt开发出一套能自由切换视点的播放系统，构建了一个较为完善的YUV类，实现了逐帧、连续播放及回退、截图、自由缩放等功能，并应用了膨胀、腐蚀、直方图均衡、二值化、边缘检测、分水岭分割等视频特效算法。

### 核心功能 :
  - **输入控制 :** 
    - 多视点YUV视频序列 (用户只需输入同一文件夹下的一个视频，其他相同命名格式的文件由软件自动读取)。
  - **输出展示 :**
    - 支持特定视点切换及多种实时特效叠加的视频流。
  - **播放控制 :**
	- YUV视频播放与回放。
	- 视点实时切换。
	- 播放操作：单帧步进/步退、多帧步进/步退。
  - **交互与特效 :**
	- 截屏预览。
	- 视频特效：自由缩放、灰度化、膨胀、腐蚀、直方图均衡、边缘检测、分水岭分割。

---

## 🖼 Screenshots (演示预览)

### Main Interface (主界面)
文件打开后加载的初始化界面
![](/doc/images/001.png)

文件播放并切换视点后界面
![](/doc/images/002.png)

### Viewpoint Switching (视点切换)
此处为后期处理图，展示视点切换效果
![](/doc/images/011.png)

### Video Effects (视频特效)

#### Zoom In/Out (放大与缩小)
![](/doc/images/003.png)
![](/doc/images/004.png)

#### Gray / Dilate / Erode (灰度化/膨胀/腐蚀)
![](/doc/images/005.png)
![](/doc/images/006.png)
![](/doc/images/007.png)

#### Histogram / Edge Detect / Watershed (直方图/边缘检测/分水岭)
![](/doc/images/008.png)
![](/doc/images/009.png)
![](/doc/images/010.png)

---

## 🛠 Development Environment (开发环境)

*   [**Visual studio 2015 com**](https://www.visualstudio.com/);
*   [**QT5.6.3**](https://www.qt.io/qt5-6/)
*   [**Opencv2.4.13**](https://opencv.org/releases.html)

## 🤝 Contact
本软件仍处于开发状态，有好的想法欢迎一起完善，联系我(tomruibin@163.com) 
