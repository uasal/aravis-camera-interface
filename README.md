# Camera Software for the AstroSDR

## Overview
This software is designed to run on the AstroSDR, interfacing with an HD gigabit ethernet camera. Software was written and tested using a PL-B771G GigE Camera. It is designed to handle camera discovery, writing feature parameters to the camera, and collect video stream for user-defined durations of time. The software will also handle insertion of images into the ASDR's memory buffer downlink process through the FPGA.

## Dependencies
The software primarily uses [Aravis-0.8](https://github.com/AravisProject/aravis) to handle the networking with the camera. The UDP protocol used by the camera (GigE Vision Streaming/Control Protocol) is a closed protocol, so the reconstruction mechanism of Aravis is highly useful.

At the moment, there are many other dependencies because the software must be statically compiled on the ASDR. It may be possible to remove some of the mentioned dependencies below if the size of the executable becomes a concern.

* aravis-0.8
* glib-2.0
* pthread
* gobject-2.0 
* gio-2.0 
* gthread-2.0 
* gmodule-2.0
* ffi 
* pcre 
* mount 
* blkid 
* uuid
* icui18n 
* icuuc 
* z 
* m
* lzma 
* swresample 
* xml2 
* icudata 
* dl 
* resolv

## Building the Package
The software uses CMake to build source and test files. To build everything, simply navigate to the root directory of the project and perform

```
mkdir build
cd build
cmake ..
```

## Usage
After building the project, the executable `hdcam` can be found in the `build/src/` folder. 

```
hdcam --help

```
will reveal a list of options that may be invoked with the software.

Example: to stream the camera for 100 seconds with default features, invoke
```
hdcam -d 100

```

## Future Tasks

## Known Issues

