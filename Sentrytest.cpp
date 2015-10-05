#include<wiringPi.h>
#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
 
int main(int argc, char *argv[])
{
    std::cout << "Hello";
    cv::Mat frame;
    cv::Mat frameSmaller;
    cv::Mat back;
    cv::Mat fore;
    cv::VideoCapture cap(0);
    cv::BackgroundSubtractorMOG2 bg(2, false);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;
 

    std::vector<std::vector<cv::Point> > contours;
cv::Moments moments;
 
    cv::namedWindow("FrameSmaller");
    cv::namedWindow("Fore");
 
    for(;;)
    {
    	std::cout << "inside for";
        cap >> frame;
        cv:resize(frame, frameSmaller, cv::Size(), 0.1, 0.1, cv::INTER_NEAREST);
	// Print out the width
	std::cout << frameSmaller.Size.Width;

        bg.operator ()(frameSmaller,fore);
        bg.getBackgroundImage(back);
        cv::erode(fore,fore,cv::Mat());
        cv::dilate(fore,fore,cv::Mat());
        cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
        cv::drawContours(frameSmaller,contours,-1,cv::Scalar(0,255,0),0.1);

        // Try to find the "midpoint" of the foreground image - that's what we will shoot at
        moments = cv::moments(fore, false);
        int x = (int) (moments.m10 / moments.m00);
        int y = (int) (moments.m01 / moments.m00);
        
	//std::cout << x;
	if (x > 0 && y > 0) {
		// Draw the circle
     		int thickness = 1;
		int lineType = 8;
        	cv::ellipse( frameSmaller,
           	  cv::Point(x, y),
           	  cv::Size( 5, 5 ),
           	  0,
           	  0,
           	  360,
           	  cv::Scalar( 0, 0, 255 ),
           	  thickness,
           	  lineType );

		// Draw the crosshairs
	        cv::line(frameSmaller, cv::Point(x+7, y), cv::Point(x-7, y), cv::Scalar(0,0,255), 1.5);
        	cv::line(frameSmaller, cv::Point(x, y - 7), cv::Point(x, y+7), cv::Scalar(0,0,255), 1.5);
	}

        cv::imshow("FrameSmaller",frameSmaller);
        cv::imshow("Fore",fore);
//        cv::imshow("Background",back);
        if(cv::waitKey(30) >= 0) break;
    }
    return 0;
}
