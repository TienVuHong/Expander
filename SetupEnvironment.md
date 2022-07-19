# Expander setup environment
## Install on device
### Step1: update your machine
```
sudo apt update
```

### Step2: install all mandatory libraries 
#### 1. Install libevent for http-server
```
sudo apt install libevent-dev
```
#### 2. Install sqlite3 for database
```
sudo apt install sqlite3
sudo apt install libsqlite3-dev
sqlite3 --version
```
#### 3. Install curl for http-client
```
sudo apt install curl
sudo apt-get install libcurl4-openssl-dev
curl --version
```
#### 4. Install opencv
Refer to this [document](https://github.com/tien7397/Expander/blob/main/SetupOpencv4.md)
#### 5. Install cpp-jwt (JSON Web Token)
```
git clone https://github.com/arun11299/cpp-jwt
cd cpp-jwt
mkdir build
cd build
cmake ..
make -j4
sudo make install
```
#### 6. Install json
```
git clone https://github.com/nlohmann/json
cd json
mkdir build
cmake ..
make -j4
sudo make install
```
### Step3: build Expander project
Inside source folder, use this command
```
mkdir build
cd build
#CAUTION !!!!!!!!!!!!!!!!!!!
#cmake for Ubuntu PC
	cmake ..
#cmake for Raspberry Pi
	cmake -D RASPBERRY_PI=ON ..
make -j4
```
### Step4: finally, run the program
```
./PiServer
```

## Install into custom folder
***Default, libraries will be install in /usr/lib, if you want to install libraries for portable application, use Prefix {CUSTOM_PATH} instead***
```
Example: my libraries folder look like
compiler/libs_ubuntu/lib_event-2.1.12
compiler/libs_ubuntu/lib_curl-7.79.1
compiler/libs_ubuntu/lib_sqlite-3.36
compiler/libs_ubuntu/lib_cpp-jwt-1.2.0
compiler/libs_ubuntu/lib_json-3.9.1
compiler/libs_ubuntu/lib_openssl-1.1.1l
compiler/libs_ubuntu/lib_opencv-4.5.2
compiler/libs_pi/lib_event-2.1.12
compiler/libs_pi/lib_curl-7.79.1
compiler/libs_pi/lib_sqlite-3.36
compiler/libs_pi/lib_cpp-jwt-1.2.0
compiler/libs_pi/lib_json-3.9.1
compiler/libs_pi/lib_openssl-1.1.1
compiler/libs_pi/lib_opencv-4.5.2
compiler/libs_pi/lib_zlib-1.2.11
```
### 1. Install libevent
1. Reference here: https://libevent.org/
2. Download .tar.gz file
3. Extract file
4. Open terminal in folder
```
./configure --prefix={CUSTOM_PATH}
make
make intall
```
### 2. Install curl
1. Reference here: https://curl.se/download.html
2. Download file .zip
3. Extract file
3. Open terminal in folder
```
./configure --with-openssl --prefix={CUSTOM_PATH}
make
make test
make install
```
### 3. Install sqlite3
1. Reference here: https://www.sqlite.org/download.html
2. Download file .zip
3. Extract file
3. Open terminal in folder
```
./configure --prefix={CUSTOM_PATH}
make
make install
```
### 4. Install cpp-jwt (JSON Web Token)
```
git clone https://github.com/arun11299/cpp-jwt
cd cpp-jwt
mkdir build
cd build
cmake --install-prefix={CUSTOM_PATH} ..
make -j4
make install
```
### 5. Install json
```
git clone https://github.com/nlohmann/json
cd json
mkdir build
cmake --install-prefix={CUSTOM_PATH} ..
make -j4
make install
```
### 6. Install openssl
1. Reference: https://www.openssl.org/source/
2. Download openssl-1.1.1l.tar.gz 
3. Extract file
4. Open terminal in folder
```
./config --prefix={CUSTOM_PATH}
make
make test
make install
```
### 7. Install opencv
1. Refer to this [document](https://github.com/tien7397/Expander/blob/main/SetupOpencv4.md)
### 8. zlib - only when cross compiler for Raspberry Pi
1. Reference: https://zlib.net/
2. Download .zip file
3. Extract file
4. Open terminal in folder
```
./configure --prefix={CUSTOM_PATH}
make
make install
```