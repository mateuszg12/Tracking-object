#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv_modules.hpp>

#define CAM_INDEX 1
#define FRAME_TIME 50
#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720

int main()
{
    cv::VideoCapture camera;
    camera.open(CAM_INDEX);
    if(!camera.isOpened())
    {
        std::cout << "Could not open camera on port " << CAM_INDEX << "!" << std::endl;
        return -1;
    }

    camera.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    cv::Mat frame(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC3);

    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
    cv::moveWindow("Camera", 0, 0);

    while(true)
    {
        camera >> frame;

        cv::imshow("Camera", frame);

        char user_input = cv::waitKey(FRAME_TIME);
        if(user_input == 27)
            break;
    }

    camera.release();
    return 0;
}
