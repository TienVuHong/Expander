# Expander
## Step1: update your machine
sudo apt update

## Step2: install all mandatory libraries 
### Install cpp-jwt (JSON Web Token)
<sub> Reference: https://github.com/arun11299/cpp-jwt </sub>
```
cd cpp-jwt
mkdir build
cd build
cmake ..
make -j4
make install
cd ../../
```

### Install json
<sub> reference: https://github.com/nlohmann/json </sub>
```
cd json
mkdir build
cmake ..
make -j4
make install
cd ../../../
```

### Install libevent for http-server
```
sudo apt install libevent-dev
```
*** OR build from source 2.1.11 ***
Download here: https://libevent.org/
Download .tar.gz file
Open terminal in folder
```
./configure
make
make intall
    OR
mkdir build
cd build
cmake ..
make
make install
```

### Install sqlite3 for database
```
sudo apt install sqlite3
sudo apt install libsqlite3-dev
sqlite3 --version
```
*** OR build from source 3.36 ***
Download here: https://www.sqlite.org/download.html
Download file .zip
Open terminal in folder
```
./configure
make
make install
```


### Install curl for http-client
```
sudo apt install curl
sudo apt-get install libcurl4-openssl-dev
curl --version
```
*** OR build from source 7.79.1 ***
Download here: https://curl.se/download.html
Download file .zip
Open terminal in folder
```
./configure --with-openssl --prefix=...
make
make test
make install
```

## Step3: build Expander project
Inside source file, use this command
```
mkdir build
cd build
#CAUTION !!!!!!!!!!!!!!!!!!!
#cmake command for ubuntu PC
	cmake ..
#cmake command for Pi only
	cmake -D RASPBERRY_PI=ON ..
make -j4
```

## Step4: finally, run the program
```
./PiServer
```



## Build lib for cross compiler - ignore please
### zlib
https://zlib.net/
./configure
make
make install

### openssl 1.1.1
https://www.openssl.org/source/
./config
make
make test
make install
