#include<opencv2/opencv.hpp>
#include <stdlib.h>      //for using the function sleep
#include<iostream>
#include<vector>
#include "./pca9685.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <ctime>

// Constants for image processing
int imageWidth = 64;
int imageHeight = 48;

// Constants for servo controls
int pinBase =300;
int i2cAddress =0x40;
float freq=50;
int panPin = pinBase + 1;
int tiltPin = pinBase + 0;
int triggerPin = pinBase + 2;
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
    std::cout << "pointing to " << degrees << std::endl;
    pwmWrite(tiltPin, pwmWidth);
}

void pointToDegrees(int degrees)
{
    int pwmWidth = degreesToPwmWidth(degrees);
    pwmWrite(triggerPin, pwmWidth);
}



int calculatePanDegreeChange(int x) 
{
    int fullRightPixels = imageWidth;
    int fullLeftPixels = 0;

    int fullRightDegrees = -30;
    int fullLeftDegrees = 30;

    int degreeRange = fullRightDegrees - fullLeftDegrees;
    int pixelRange = fullRightPixels - fullLeftPixels;

    float percentageAlongPixelRange = (float)x / (float)pixelRange;
    int panDegreeChange = fullLeftDegrees + (percentageAlongPixelRange * degreeRange);
    return panDegreeChange;
}

int calculateTiltDegreeChange(int y) 
{
    int fullUpPixels = 0;
    int fullDownPixels = imageHeight;

    int fullUpDegrees = 10;
    int fullDownDegrees = -10;

    int degreeRange = fullDownDegrees - fullUpDegrees;
    int pixelRange = fullUpPixels - fullDownPixels;

    float percentageAlongPixelRange = (float)y / (float)pixelRange;
    std::cout << "percentageAlongPixelRange is " << percentageAlongPixelRange << std::endl;
    int tiltDegreeChange = fullDownDegrees + (percentageAlongPixelRange * degreeRange);
    return tiltDegreeChange;
}

void shootStart()
{ 
    pointToDegrees(70);

}

void shootFinish()
{ 
    pointToDegrees(150);  
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
    bool haveShotYet = false;

    // Set up openCV
    std::cout << "Hello" << std::endl;
    cv::Mat frame;
    cv::Mat frameSmaller;
    cv::Mat back;
    cv::Mat fore;
    cv::VideoCapture cap(0);
    cv::BackgroundSubtractorMOG2 bg(3, false);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;
 

    std::vector<std::vector<cv::Point> > contours;
    cv::Moments moments;
 
    cv::namedWindow("FrameSmaller");
    cv::namedWindow("Fore");
 
    for(;;)
    {
    
        cap >> frame;
        cv::resize(frame, frameSmaller, cv::Size(imageWidth, imageHeight));
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
        if ((timeNow - lastMoveTime >= 1) && !haveShotYet){
             shootStart();
             haveShotYet = true;
	}
        if ((timeNow - lastMoveTime >= 2) && (x > 0))
	{
                shootFinish();
                haveShotYet = false;

        	// Calculate where to pan to
	        int panDegreeChange = calculatePanDegreeChange(x);
		std::cout << "would pan by " << panDegreeChange << " degrees" << std::endl;
                currentPan += panDegreeChange;
	        if (currentPan < 0) currentPan = 0;
		if (currentPan > 180) currentPan = 180;
		panToDegrees(currentPan);
                
                
                // Calculate where to tilt to
                int tiltDegreeChange = calculateTiltDegreeChange(y);
                std::cout << " would tilt by " << tiltDegreeChange << " degrees" << std::endl;
                currentTilt += tiltDegreeChange;
	        if (currentTilt < 50) currentTilt = 50;
		if (currentTilt > 130) currentTilt = 130;
		tiltToDegrees(currentTilt);
                
                //make it shoot
                //int firing = 1
                //int firing = 0
                                

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
