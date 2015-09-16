
#include<opencv2/opencv.hpp>
#include<iostream>
#include<vector>
 
int main(int argc, char *argv[])
{
    std::cout << "Hello";
    cv::Mat frame;
    cv::Mat back;
    cv::Mat fore;
    cv::VideoCapture cap(0);
    cv::BackgroundSubtractorMOG2 bg;
    //bg.nmixtures = 3;
    //bg.bShadowDetection = false;
 
    std::vector<std::vector<cv::Point> > contours;
 
    cv::namedWindow("Frame");
    cv::namedWindow("Background");
 
    for(;;)
    {
    std::cout << "inside for";
        cap >> frame;
        bg.operator ()(frame,fore);
        bg.getBackgroundImage(back);
    std::cout << "got background image";
        cv::erode(fore,fore,cv::Mat());
        cv::dilate(fore,fore,cv::Mat());
    std::cout << "done erode and dilate";
        cv::findContours(fore,contours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_NONE);
    std::cout << ", found contours";
        cv::drawContours(frame,contours,-1,cv::Scalar(0,0,255),2);
    std::cout << ", drawn contours";
//        frame.save("frame.jpg");

        cv::imshow("Frame",frame);
        cv::imshow("Background",back);
        if(cv::waitKey(30) >= 0) break;
    }
    return 0;
}
