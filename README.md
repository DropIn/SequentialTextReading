Sequential Text Reading
=======================

Software to analyze video of scanned printed text, running OCR and turning it into speech.

Prerequisites
-------------
* CMake: http://www.cmake.org/
* Qt 4.7+ : http://qt-project.org/
* OpenCV 2.4.x: http://www.opencv.org/
* Tesseract OCR: https://code.google.com/p/tesseract-ocr/
* Flite text-to-speech engine: http://www.speech.cs.cmu.edu/flite/ 
* Boost (just the System component): http://www.boost.org/

Everything needs to be compiled for the system of your choice. We've successfully built for Windows (via MinGW) and Mac OS 10.7

Building
--------
In high-level, just run CMake and then build

	mkdir build ; cd build
	cmake ..
	make

On Windows we've used MinGW (MSYS build system), built Tessract, Flite and Boost from source, OpenCV and Qt have prebuilt binaries for MinGW.
We reccommend using Eclipse for building, by setting up CMake to create an Eclipse/MinGW project.

Building Tesseract on MinGW: http://www.sk-spell.sk.cx/compiling-leptonica-and-tesseract-ocr-with-mingwmsys
Building Flite on MinGW is straightforward: http://www.speech.cs.cmu.edu/flite/doc/flite_4.html#SEC4
When building boost, remember to only build System else the build takes forever. Run boostrap script and then use bjam to build System.

Running
-------
The Qt UI should be quite self explanatory. You load a video or connect to a camera (OpenCV does that), and wait for the text to be read out loud. 
The video should be of a close-up of scanning along a printed text line with the finger.
It supports only English` (left-to-right)
