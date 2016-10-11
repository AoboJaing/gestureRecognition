#include <iostream>

#include "videoprocessor.h"
#include "wristbandTracker.h"

#include<windows.h>

int dataLimiting(int origin, int low, int high);
// get now handwrist location - past handwrist location = slope
template<class T>
T getSlope(T past, T now);

int isClick = 0, pastWristbandNumber = 0;

float handSlopeX, handSlopeY;
float handPastOffsetX, handPastOffsetY;
bool firstHandOffset;

int main()
{

    // Create instance
    VideoProcessor processor;
    // Open video file
    processor.setInput(0);

    WristbandTracker wristband;

    // Declare a window to display the video
//    processor.displayInput("Current Frame");
    processor.displayOutput("Output Frame");
    // Play the video at the original frame rate
    processor.setDelay(10);
    // Set the frame processor callback function
    //processor.setFrameProcessor(&wristband);
    // Start the process
    //processor.run();

	int aScreenWidth, aScreenHeight;
	aScreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	aScreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	std::cout << "all Screen , Width is " << aScreenWidth << "; Height is " << aScreenHeight << std::endl;

	while(1){
		cv::Mat img, res;
		processor.runOnce(img, res);
		wristband.process(img, res);
		cv::namedWindow("Show Recogition Result");
		cv::imshow("Show Recogition Result", res);
		//std::cout << "wristbandNumber is " << wristband.wristbandNumber << std::endl;
		std::cout << wristband.move_x << "\t" << wristband.move_y << "\t" << wristband.move_z << std::endl;

		//get cursor position
		POINT p;
        GetCursorPos(&p);
		std::cout << "cursor position is : x = " << p.x << ", y = " << p.y << std::endl;

		// if not recognition wristhand, continue this circle
		if(wristband.wristbandNumber == 0){
			//set have first hand offset is true.
			firstHandOffset = true;
			continue;
		}
		POINT pTemp;
		//if recognition wristhand, update cursor.
		if(wristband.wristbandNumber != 0){
			//if the time is first have hand offset value, update hand pasr offset value. 
			if(firstHandOffset == true){
				firstHandOffset = false;
				handPastOffsetX = wristband.move_x;
				handPastOffsetY = wristband.move_y;
			}
			// get hand x,y offset's slope.
			handSlopeX = getSlope<float>(handPastOffsetX, wristband.move_x);
			handSlopeY = getSlope<float>(handPastOffsetY, wristband.move_y);
			// Limiting cursor result coordinate value.
			pTemp.x = dataLimiting(p.x+handSlopeX, 0+1, aScreenWidth-1);
			pTemp.y = dataLimiting(p.y-handSlopeY, 0+1, aScreenHeight-1);
			//Set cursor coordinate.
			SetCursorPos(pTemp.x, pTemp.y);
			//udate hand past offset.
			handPastOffsetX = wristband.move_x;
			handPastOffsetY = wristband.move_y;

		}
		if(wristband.wristbandNumber == 2 && isClick == 0){
			if(wristband.wristbandNumber == pastWristbandNumber)
				;
			else{
				//mouse left button down
				isClick = 1;
				mouse_event(MOUSEEVENTF_LEFTDOWN,pTemp.x,pTemp.y,0,0);
			}
		}
		if(isClick == 1 && wristband.wristbandNumber == 1){
			//mouse left button up
			isClick = 0;
			mouse_event(MOUSEEVENTF_LEFTUP,pTemp.x,pTemp.y,0,0);
			//http://keleyi.com/a/bjac/2ua08g4c.htm
		}
		pastWristbandNumber = wristband.wristbandNumber;
	}

    cv::waitKey(0);
    return 0;
}

//Data Limiting.
int dataLimiting(int origin, int low, int high){
	int res = origin;
	if(res < low)
		res = low;
	else if(res > high)
		res = high;
	return res;
}

// get now handwrist location - past handwrist location = slope
template<class T>
T getSlope(T past, T now){
	return now - past;
}


