/* 
 * File:   UserInterface.cpp
 * Author: Jan Dufek
 */

#include "UserInterface.hpp"

// Program settings
Settings * UserInterface::settings;

// Original point of click
Point UserInterface::origin;

Size UserInterface::video_size;

UserInterface::UserInterface(Settings& s, Size sz) {

    UserInterface::settings = &s;

    UserInterface::video_size = sz;

    // Show main window including slide bars
    create_main_window();

    // Histogram window
    namedWindow("Histogram", 0);

    // Set mouse handler on main window to choose object of interest
    setMouseCallback(UserInterface::settings->MAIN_WINDOW, onMouse, 0);

    // Set main window to full screen
    setWindowProperty(settings->MAIN_WINDOW, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
}

UserInterface::UserInterface(const UserInterface& orig) {
}

UserInterface::~UserInterface() {
}

/**
 * Trackbar handler. Called when track bar is clicked on.
 * 
 */
void UserInterface::on_trackbar(int, void*) {

    // Gaussian kernel size must be positive and odd. Or, it can be zero’s and
    // then it is computed from sigma.
    if (UserInterface::settings->blur_kernel_size % 2 == 0) {
        UserInterface::settings->blur_kernel_size++;
    }

    // Erode size cannot be 0
    if (UserInterface::settings->erode_size == 0) {
        UserInterface::settings->erode_size++;
    }

    // Dilate size cannot be 0
    if (UserInterface::settings->dilate_size == 0) {
        UserInterface::settings->dilate_size++;
    }
}

/**
 * Shows the main window and creates track bars.
 * 
 */
void UserInterface::create_main_window() {

    // Show new window
    namedWindow(UserInterface::settings->MAIN_WINDOW, CV_GUI_NORMAL);

    // Saturation trackbars
    createTrackbar("S Min", UserInterface::settings->MAIN_WINDOW, &UserInterface::settings->saturation_min, 255, on_trackbar);
    createTrackbar("S Max", UserInterface::settings->MAIN_WINDOW, &UserInterface::settings->saturation_max, 255, on_trackbar);

    // Value trackbars
    createTrackbar("V Min", UserInterface::settings->MAIN_WINDOW, &UserInterface::settings->value_min, 255, on_trackbar);
    createTrackbar("V Max", UserInterface::settings->MAIN_WINDOW, &UserInterface::settings->value_max, 255, on_trackbar);

    // Gaussian blur trackbar
    createTrackbar("Blur", UserInterface::settings->MAIN_WINDOW, &UserInterface::settings->blur_kernel_size, min(UserInterface::video_size.height, UserInterface::video_size.width), on_trackbar);  

}

/**
 * Select object of interest in image.
 * 
 */
void UserInterface::onMouse(int event, int x, int y, int flags, void*) {

    // Select object mode
    if (select_object) {

        // Get selected rectangle
        selection.x = MIN(x, UserInterface::origin.x);
        selection.y = MIN(y, UserInterface::origin.y);
        selection.width = abs(x - UserInterface::origin.x);
        selection.height = abs(y - UserInterface::origin.y);

        // Get intersection with the original image
        selection &= Rect(0, 0, UserInterface::video_size.width, UserInterface::video_size.height); // TODO check if works
    }

    // Check mouse button
    switch (event) {

            // Right drag is reserved for moving over the image
            // Left click context menu is disabled
            // Scrool is reserved for zoom

            // Right drag to select EMILY
        case EVENT_RBUTTONDOWN:

            // Save current point as click origin
            UserInterface::origin = Point(x, y);

            // Initialize rectangle
            selection = Rect(x, y, 0, 0);

            // Start selection
            select_object = true;

            break;

        case EVENT_RBUTTONUP:

            // End selection
            select_object = false;

            // If the selection has been made, start tracking
            if (selection.width > 0 && selection.height > 0) {
                object_selected = -1;
            }

            break;
            
    }
}

/**
 * Draws position of the object as crosshairs with the center in the object's
 * centroid.
 * 
 * @param x x coordinate
 * @param y y coordinate
 * @param radius radius of crosshairs
 * @param frame frame to which draw into
 */
void UserInterface::draw_position(int x, int y, double radius, Mat &frame) {

    // Lines
    if (y - radius > 0) {
        line(frame, Point(x, y), Point(x, y - radius), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    } else {
        line(frame, Point(x, y), Point(x, 0), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    }

    if (y + radius < UserInterface::video_size.height) {
        line(frame, Point(x, y), Point(x, y + radius), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    } else {
        line(frame, Point(x, y), Point(x, UserInterface::video_size.height), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    }

    if (x - radius > 0) {
        line(frame, Point(x, y), Point(x - radius, y), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    } else {
        line(frame, Point(x, y), Point(0, y), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    }

    if (x + radius < UserInterface::video_size.width) {
        line(frame, Point(x, y), Point(x + radius, y), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    } else {
        line(frame, Point(x, y), Point(UserInterface::video_size.width, y), UserInterface::settings->LOCATION_COLOR, UserInterface::settings->LOCATION_THICKNESS);
    }

    // Text coordinates
    putText(frame, "[" + int_to_string(x) + "," + int_to_string(y) + "]", Point(x, y + radius + 20), 1, 1, UserInterface::settings->LOCATION_COLOR, 1, 8);
}

void UserInterface::print_status(Mat& frame, int status) {

    String stringStatus;

    switch (status) {
        case 0:
            stringStatus = "Initialization";
            break;
        case 1:
            stringStatus = "Select EMILY and target.";
            break;
        case 2:
            stringStatus = "Target set. Getting orientation.";
            break;
        case 3:
            stringStatus = "Target set. Going to target.";
            break;
    }

    // Print status
    putText(frame, stringStatus, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255), 2);
}

/**
 * Convert integer to string.
 * 
 * @param number integer to be converted to string
 * 
 */
string UserInterface::int_to_string(int number) {
    stringstream stringStream;
    stringStream << number;
    return stringStream.str();
}

void UserInterface::show_main(Mat& mat) {
    imshow(UserInterface::settings->MAIN_WINDOW, mat);
}

void UserInterface::show_histogram(Mat& mat) {
    imshow(UserInterface::settings->HISTOGRAM_WINDOW, mat);
}