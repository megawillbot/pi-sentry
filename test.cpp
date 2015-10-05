#include<opencv2/opencv.hpp>
#include <stdlib.h>     //for using the function sleep
#include<iostream>
#include<vector>
#include "./pca9685.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <ctime>

// Constants for servo controls
int pinBase =300;
int i2cAddress =0x40;
float freq=50;
int panPin = pinBase + 1;
int tiltPin = pinBase + 0;

int degreesToPwmWidth(int degrees)
{
    int lowest = 150;
    int highest = 530;
    int range = highest - lowest;

    float percentageThroughRange = (float)degrees / (float)180;

    int result = (lowest + percentageThroughRange * range);
    // std::cout << "pwmWidth of " << degrees << " is " << result << std::endl;
    return result;
}

void panToDegrees(int degrees)
{
    int pwmWidth = degreesToPwmWidth(degrees);
    pwmWrite(panPin, pwmWidth);
}

void tiltToDegrees(int degrees)
{
    int pwmWidth = degreesToPwmWidth(degrees);
    pwmWrite(tiltPin, pwmWidth);
}

int calculatePanDegreeChange(int x) 
{
    int fullRightPixels = 64;
    int fullLeftPixels = 0;

    int fullRightDegrees = -45;
    int fullLeftDegrees = 45;

    int degreeRange = fullRightDegrees - fullLeftDegrees;
    int pixelRange = fullRightPixels - fullLeftPixels;

    float percentageAlongPixelRange = (float)x / (float)pixelRange;
    int panDegreeChange = fullLeftDegrees + (percentageAlongPixelRange * degreeRange);
    return panDegreeChange;
}

int main(int argc, char *argv[])
{
    // Setup servo controller
    pca9685Setup(pinBase, i2cAddress, freq);

    // Move servos to initial position
    int currentPan = 90;
    int currentTilt = 90;
    panToDegrees(currentPan);
    tiltToDegrees(currentTilt);
    time_t lastMoveTime;
    time(&lastMoveTime);

    // Set up openCV
    std::cout << "Hello" << std::endl;
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
	int width = frameSmaller.size().width;

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
        
        // See if it's time to move yet
        time_t  timeNow;
	time(&timeNow);
        if ((timeNow - lastMoveTime >= 2) && (x > 0))
	{
        	// Calculate where to pan to
	        int panDegreeChange = calculatePanDegreeChange(x);
		std::cout << "would pan by " << panDegreeChange << " degrees" << std::endl;
                currentPan += panDegreeChange;
	        panToDegrees(currentPan);

		// Remember the time we last moved
		time_t  timeNow;
		time(&lastMoveTime);
		std::cout << "time now is " << timeNow << std::endl;
	}

        cv::imshow("FrameSmaller",frameSmaller);
        cv::imshow("Fore",fore);
        if(cv::waitKey(30) >= 0) break;
    }
    return 0;
}
