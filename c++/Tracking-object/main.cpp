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
#define THRESH_LEVEL 35

int main()
{
    bool show_difference = false;
    bool show_threshold = false;

    cv::VideoCapture camera;
    camera.open(CAM_INDEX);
    if(!camera.isOpened())
    {
        std::cout << "Could not open camera on port " << CAM_INDEX << "!"
                  << "Closing app!" << std::endl;
        return -1;
    }

    camera.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

    cv::Mat frame(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC3);
    cv::Mat frame_gray(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC1);
    cv::Mat frame_gray_old(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC1);
    cv::Mat difference_frame(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC1);
    cv::Mat threshold_frame(FRAME_WIDTH, FRAME_HEIGHT, CV_8UC1);

    cv::namedWindow("Camera", cv::WINDOW_AUTOSIZE);
    cv::moveWindow("Camera", 0, 0);

    //Get first frame
    camera >> frame;
    cv::cvtColor(frame, frame_gray_old, CV_RGB2GRAY);

    //Main loop
    while(true)
    {
        //Get next frame
        camera >> frame;
        cv::cvtColor(frame, frame_gray, CV_RGB2GRAY);

        cv::absdiff(frame_gray, frame_gray_old, difference_frame);
        cv::threshold(difference_frame, threshold_frame, THRESH_LEVEL, 255, cv::THRESH_BINARY);

        //Display image
        cv::imshow("Camera", frame);
        //cv::imshow("Grayscale", frame_gray);

        if(show_difference)
            cv::imshow("Grayscale", difference_frame);

        if(show_threshold)
            cv::imshow("Threshold", threshold_frame);

        //Save frame for next step
        frame_gray_old = frame_gray;

        //Check user input
        char user_input = cv::waitKey(FRAME_TIME);
        if(user_input == 27)
            break;

        switch(user_input)
        {
        case 'd':
            show_difference = !show_difference;
            if(!show_difference)
            {
                std::cout << "Difference image display disabled" << std::endl;
                cv::destroyWindow("Grayscale");
            }
            else
                std::cout << "Difference image display enabled" << std::endl;
            break;

        case 't':
            show_threshold = !show_threshold;
            if(!show_threshold)
            {
                std::cout << "Threshold image display disabled" << std::endl;
                cv::destroyWindow("Threshold");
            }
            else
                std::cout << "Threshold image display enabled" << std::endl;
            break;

        case 'p':
            std::cout << "PAUSED" << "  Press p to resume" << std::endl;
            while(true)
                if(cv::waitKey(FRAME_TIME) == 'p')
                {
                    std::cout << "RESUMED" << std::endl;
                    break;
                }
            break;
        }
    }

    //Clean up
    camera.release();
    cv::destroyAllWindows();
    return 0;
}
