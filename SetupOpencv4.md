# Install Opencv4
## Release Memory
```
sudo apt clean
sudo apt autoremove
```

## Install necessary libraries
```
sudo apt -y install build-essential cmake pkg-config
sudo apt -y install libjpeg-dev libtiff5-dev libjasper-dev libpng12-dev
sudo apt -y install libavcodec-dev libavformat-dev libswscale-dev libv4l-dev
sudo apt -y install libxvidcore-dev libx264-dev
sudo apt -y install libgtk2.0-dev libgtk-3-dev
sudo apt -y install libatlas-base-dev gfortran
sudo apt -y install libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev
sudo apt -y install libatlas-base-dev
sudo apt -y install libmp3lame-dev libtheora-dev
sudo apt -y install libvorbis-dev libxvidcore-dev libx264-dev
sudo apt -y install libopencore-amrnb-dev libopencore-amrwb-dev
sudo apt -y install libavresample-dev
sudo apt -y install x264 v4l-utils libhdf5-dev
sudo apt -y install libprotobuf-dev protobuf-compiler
sudo apt -y install libgoogle-glog-dev libgflags-dev
sudo apt -y install libgphoto2-dev libeigen3-dev libhdf5-dev doxygen
sudo apt -y install build-essential cmake git libgtk2.0-dev libgtk-3-dev pkg-config libavcodec-dev libavformat-dev libswscale-dev libv4l-dev 
sudo apt -y install libjpeg8-dev libjasper-dev libpng12-dev libtiff5-dev
```
## Edit swap file
<sub> If there no swap file - ignore </sub>
```
sudo /etc/init.d/dphys-swapfile stop
sudo nano /etc/dphys-swapfile
#Change CONF_SWAPSIZE=1024
sudo /etc/init.d/dphys-swapfile start
```

## Compile and Install
1. Reference: https://opencv.org/releases/
2. Download: Chosing a version and click on "source" button
3. Extract file
4. Open terminal in folder
```
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=RELEASE \
-D CMAKE_INSTALL_PREFIX=/usr/local/ \
-D WITH_GSTREAMER=ON \
-D WITH_V4L=ON \
-D WITH_OPENGL=ON \
-D  BUILD_opencv_python3=ON \
-D  BUILD_opencv_python2=ON \
-D OPENCV_GENERATE_PKGCONFIG=ON \
-D BUILD_opencv_world=ON ..
```
**-D CMAKE_INSTALL_PREFIX=/usr/local \ is default path, can use -D CMAKE_INSTALL_PREFIX={CUSTOM_PATH} \ for install library in custom path**
```
make -j6
make install
sudo ldconfig
```

