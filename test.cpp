
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
    cv::BackgroundSubtractorMOG2 bg(10, false);
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;
 
    std::vector<std::vector<cv::Point> > contours;
 
    cv::namedWindow("FrameSmaller");
    cv::namedWindow("Fore");
 
    for(;;)
    {
    std::cout << "inside for";
        cap >> frame;
        cv:resize(frame, frameSmaller, cv::Size(), 0.4,0.4, cv::INTER_NEAREST);
        bg.operator ()(frameSmaller,fore);
        bg.getBackgroundImage(back);
        cv::erode(fore,fore,cv::Mat());
        cv::dilate(fore,fore,cv::Mat());
        cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
        cv::drawContours(frameSmaller,contours,-1,cv::Scalar(0,0,255),2);
        cv::imshow("FrameSmaller",frameSmaller);
        cv::imshow("Fore",fore);
//        cv::imshow("Background",back);
        if(cv::waitKey(30) >= 0) break;
    }
    return 0;
}
