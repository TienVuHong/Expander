# Expander
## Build and run
1. Download "[compiler](https://drive.google.com/file/d/1xQmi6l3SPxJl9oD-uOrYI0WTW93Z1Sec/view?usp=sharing)" folder which includes all the necessary libraries, extract and place in the same directory with this source folder 
2. Go into source folder and use these command
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
3. Execute the program
```
./PiServer
```

## Setup the environment
If you want to setup the environment, please refer to the further [document](https://github.com/tien7397/Expander/blob/main/SetupEnvironment.md)
