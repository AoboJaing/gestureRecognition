#include <iostream>

#include "videoprocessor.h"
#include "wristbandTracker.h"

#include<windows.h>

int dataLimiting(int origin, int low, int high);

int isClick = 0, pastWristbandNumber = 0;

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
		if(wristband.wristbandNumber == 0)
			continue;

		POINT pTemp;
		//if recognition wristhand, update cursor.
		if(wristband.wristbandNumber != 0){
			// Limiting cursor result coordinate value.
			pTemp.x = dataLimiting(p.x+wristband.move_x, 0+1, aScreenWidth-1);
			pTemp.y = dataLimiting(p.y-wristband.move_y, 0+1, aScreenHeight-1);
			//Set cursor coordinate.
			SetCursorPos(pTemp.x, pTemp.y);
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


