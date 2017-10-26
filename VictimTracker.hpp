/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   VictimTracker.hpp
 * Author: Jan
 *
 * Created on October 25, 2017, 9:32 PM
 */

#ifndef VICTIMTRACKER_HPP
#define VICTIMTRACKER_HPP

// Comment the following line if you do not want user interface
#define USER_INTERFACE

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include "Settings.hpp"
#include "OutputVideo.hpp"
#include "Logger.hpp"
#include "UserInterface.hpp"
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>

////////////////////////////////////////////////////////////////////////////////
// Name spaces
////////////////////////////////////////////////////////////////////////////////

using namespace cv;
using namespace std;

extern bool select_object;
extern int object_selected;
extern Rect selection;

class VictimTracker {
public:
    VictimTracker();
    VictimTracker(const VictimTracker& orig);
    virtual ~VictimTracker();

    // Main logic that is to be called on every iteration of the main loop
    int logic();

    // Get center of the victims
    Point getCenter();

    // Get size of the victims
    Size2f getSize();

private:

    ////////////////////////////////////////////////////////////////////////////////
    // Settings
    ////////////////////////////////////////////////////////////////////////////////

    Settings * settings = new Settings();

    ////////////////////////////////////////////////////////////////////////////////
    // Video Capture
    ////////////////////////////////////////////////////////////////////////////////

    VideoCapture video_capture = VideoCapture(settings->video_capture_source);

    ////////////////////////////////////////////////////////////////////////////////
    // Global variables
    ////////////////////////////////////////////////////////////////////////////////

    // Back projection mode toggle
    bool back_projection_mode = false;

    // New resized size of video used in processing
    Size resized_video_size;

    // Indicates that resizing is necessary
    bool resize_video = false;

    // Original frame
    Mat original_frame;

    // EMILY location
    Point emily_location;

    // EMILY size
    Size2f emily_size;

    // EMILY location history
    Point * emily_location_history = new Point[settings->EMILY_LOCATION_HISTORY_SIZE];

    // Timer to estimate EMILY heading
    int emily_location_history_pointer;

    // Status of the algorithm
    int status = 0;

    // Empty frame counter
    int empty_frame_counter = 0;

    VideoWriter video_writer;

    Logger * logger;

#ifdef USER_INTERFACE
    UserInterface * user_interface;
#endif

    // Frame with edits for blob detection
    Mat blured_frame;

    // Rectangle representing object of interest
    Rect object_of_interest;

    // Size of histogram of object of interest
    int histogram_size = 16;

    // Histogram ranges
    float histogram_ranges[2];
    const float * pointer_histogram_ranges = histogram_ranges;

    // HSV hue
    Mat hue;

    // Threshold on saturation and value only. Hue is not thresholded.
    Mat saturation_value_threshold;

    // Histogram of object of interest
    Mat histogram;

    // Visualization of histogram
    Mat histogram_image = Mat::zeros(200, 320, CV_8UC3);

    // Back projection of histogram
    Mat back_projection;

    // Paused mode
    bool paused = false;

    double get_input_video_fps();

    void get_input_video_size();

    void equalize(Mat&);

    void create_histogram(Rect&, int&, const float*&, Mat&, Mat&, Mat&, Mat&);

    void create_log_entry(Logger*);

    void update_history();

    void show_selection();

};

#endif /* VICTIMTRACKER_HPP */

