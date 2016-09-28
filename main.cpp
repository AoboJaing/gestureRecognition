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
    processor.setFrameProcessor(&wristband);
    // Start the process
    processor.run();

    cv::waitKey(0);
    return 0;
}

