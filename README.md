# Low Latency Multithreaded Audio Player

## Description

This command-line app was created as an exercise in thread management using the C++ 11 threading model. It uses a total of 24 asynchronous processes to play back audio from a bandwidth-limited source. It is not meant to be an optimal approach to writing mulithreaded applications (atomics are not used), but it is meant to serve as a test of coordinating many concurrent threads in an application where order of execution matters.

The app loads raw wave data using an intentionally slow file-loading library to mimick a bandwidth-limited source. The data is loaded in 512k batches and then processed in 2k chunks by 20 concurrent threads using the Microsoft waveOut API.

## Installation

This is a Visual Studio 2019 project. To compile on your own machine, ensure this version of Visual Studio is installed, then download the source code and launch the project from the solution file. 

## Usage

This project should compile and run as is. Simply launch it using VS2019 and press run. The application will start and you should hear audio playback begin. The app will terminate when all waveout files in the DATA directory are processed and played.
