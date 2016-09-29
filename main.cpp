#include <iostream>

#include "videoprocessor.h"
#include "wristbandTracker.h"


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

	while(1){
		cv::Mat img, res;
		processor.runOnce(img, res);
		wristband.process(img, res);
		cv::namedWindow("Show Recogition Result");
		cv::imshow("Show Recogition Result", res);
		//std::cout << "wristbandNumber is " << wristband.wristbandNumber << std::endl;
		std::cout << wristband.move_x << "\t" << wristband.move_y << "\t" << wristband.move_z << std::endl;
	}

    cv::waitKey(0);
    return 0;
}

