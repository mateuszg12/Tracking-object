#include <iostream>
#include <fstream>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#define CAM_INDEX 0
#define FRAME_TIME 50
#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720
#define THRESH_LEVEL 35
#define BLUR_SIZE 15

std::string int_to_string(int number);
bool init_file(std::fstream &file, std::string filename);
void write_new_line(std::fstream &file, int counter, int x, int y);
void close_file(std::fstream &file);

int main()
{
    bool show_difference = false;
    bool show_threshold = false;
    bool enable_tracking = false;
    bool enable_recording = false;
    bool object_detected = false;

    int x_pos = 0;
    int y_pos = 0;

    struct timespec start_tim, end_tim;
    int count_fps = 0;
    double seconds = 0;
    double fps = 0;

    std::fstream file;
    int counter = 1;

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

    std::string filename = "data.svc";
    if(!init_file(file, filename))
    {
        std::cout <<"Could not open or create file: " << filename
                  << "Closing app!" << std::endl;

        camera.release();
        return -2;
    }

    std::cout << "[ESC] --> close app" << std::endl;
    std::cout << "[q] ----> start tracking" << std::endl;
    std::cout << "[t] ----> show threshold" << std::endl;
    std::cout << "[d] ----> show difference" << std::endl;
    std::cout << "[r] ----> start recording to file" << std::endl;
    std::cout << "[p] ----> pause" << std::endl;
    std::cout << "[m] ----> display this menu" << std::endl;

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
        cv::blur(threshold_frame, threshold_frame, cv::Size(BLUR_SIZE, BLUR_SIZE));
        cv::threshold(threshold_frame, threshold_frame, THRESH_LEVEL, 255, cv::THRESH_BINARY);

        //Clear terminal
        //std::cout << "\033[2J\033[1;1H";

        //Detection
        if(enable_tracking)
        {
            cv::Mat tmp;
            threshold_frame.copyTo(tmp);

            std::vector<std::vector<cv::Point> > contours;
            std::vector<cv::Vec4i> hierarchy;

            cv::findContours(tmp, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );

            if(contours.size() > 0)
                object_detected=true;
            else
                object_detected = false;

            if(object_detected)
            {
                std::vector<std::vector<cv::Point>> largest_detection;
                largest_detection.push_back(contours.at(contours.size() - 1));

                cv::Rect vehicle_rectangle = cv::boundingRect(largest_detection.at(0));

                x_pos = vehicle_rectangle.x + vehicle_rectangle.width / 2;
                y_pos = vehicle_rectangle.y + vehicle_rectangle.height / 2;
            }

            std::cout << "X: " << x_pos << "   Y: " << y_pos << std::endl;

            cv::circle(frame, cv::Point(x_pos, y_pos), 20, cv::Scalar(0,255,0), 2);

            cv::line(frame, cv::Point(x_pos, y_pos), cv::Point(x_pos, y_pos-25), cv::Scalar(0,255,0), 2);
            cv::line(frame, cv::Point(x_pos, y_pos), cv::Point(x_pos, y_pos+25), cv::Scalar(0,255,0), 2);
            cv::line(frame, cv::Point(x_pos, y_pos), cv::Point(x_pos-25, y_pos), cv::Scalar(0,255,0), 2);
            cv::line(frame, cv::Point(x_pos, y_pos), cv::Point(x_pos+25, y_pos), cv::Scalar(0,255,0), 2);

            cv::putText(frame, "vehicle pos: " + int_to_string(x_pos) + ":" + int_to_string(y_pos), cv::Point(x_pos,y_pos), 1, 1, cv::Scalar(255,0,0), 2);
        }

        if(enable_recording)
        {
            write_new_line(file, counter, x_pos, y_pos);
            counter++;
        }

        //Display image
        cv::imshow("Camera", frame);
        //cv::imshow("Grayscale", frame_gray);

        if(show_difference)
            cv::imshow("Difference", difference_frame);

        if(show_threshold)
            cv::imshow("Threshold", threshold_frame);

        //Save frame for next step
        frame_gray.copyTo(frame_gray_old);

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
                cv::destroyWindow("Difference");
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

        case 'q':
            enable_tracking = !enable_tracking;
            if(!enable_tracking)
                std::cout << "Tracking disabled" << std::endl;
            else
                std::cout << "Tracking enabled" << std::endl;
            break;

        case 'r':
            enable_recording = !enable_recording;
            counter = 1;
            if(!enable_recording)
                std::cout << "Recording disabled" << std::endl;
            else
                std::cout << "Recording enabled" << std::endl;
            break;

        case 'm':
            //Clear terminal
            std::cout << "\033[2J\033[1;1H";
            //Display menu
            std::cout << "[ESC] --> close app" << std::endl;
            std::cout << "[q] ----> start tracking" << std::endl;
            std::cout << "[t] ----> show threshold" << std::endl;
            std::cout << "[d] ----> show difference" << std::endl;
            std::cout << "[r] ----> start recording to file" << std::endl;
            std::cout << "[p] ----> pause" << std::endl;
            std::cout << "[m] ----> display this menu" << std::endl;
            break;
        }

        if(count_fps > 1000)
        {
            count_fps = 0;
            clock_gettime(CLOCK_MONOTONIC, &end_tim);

            seconds = (end_tim.tv_sec - start_tim.tv_sec);
            fps  =  1 / (seconds / 1000);

            std::cout << "FPS: " << fps << ", frame time: " << seconds << std::endl;
        }
        else
            count_fps++;
    }

    //Clean up
    camera.release();
    close_file(file);
    cv::destroyAllWindows();
    return 0;
}

std::string int_to_string(int number)
{
    std::stringstream ss;
    ss << number;
    return ss.str();
}

bool init_file(std::fstream &file, std::string filename)
{
    file.open(filename, std::ios::out);

    if(!file.good())
        return 0;

    file << "Framerate: " << ", " << 1000 / FRAME_TIME << "; " << std::endl
         << "Date: "  << ", " << "21.02.2018" << ";" << std::endl
         << "Author: "  << ", " << "Mateusz Grudzień" << ";" << std::endl << std::endl;

    file << "Lp." << ", " << "X" << ", " << "Y" << ";" << std::endl;

    return 1;
}

void write_new_line(std::fstream &file, int counter, int x, int y)
{
    file << counter << ", " << x << ", " << y << ";" << std::endl;
}

void close_file(std::fstream &file)
{
    file.close();
}
