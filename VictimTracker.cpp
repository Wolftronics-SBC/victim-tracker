/* 
 * File:   VictimTracker.cpp
 * Author: Jan Dufek
 * 
 * Created on October 25, 2017, 9:32 PM
 */

#include "VictimTracker.hpp"

VictimTracker::VictimTracker() {

    ////////////////////////////////////////////////////////////////////////////
    // Output video initialization
    //////////////////////////////////////////////////////////////////////////// 

    // Get FPS of the input video
    double input_video_fps = get_input_video_fps();

    // Inogeni for some reason cannot correctly estimate the FPS.
    // Therefore we use FPS equal to 7 which is frequency of this algorithm.
#ifdef INOGENI

    input_video_fps = 7;

#endif

    // Get the size of input video
    get_input_video_size();

    // Output video name. It is in format year_month_day_hour_minute_second.avi.
    time_t raw_time;
    time(&raw_time);
    struct tm * local_time;
    local_time = localtime(&raw_time);
    char output_file_name[40];
    strftime(output_file_name, 40, "output/%Y_%m_%d_%H_%M_%S", local_time);
    string output_file_name_string(output_file_name);

    OutputVideo * output_video = new OutputVideo(input_video_fps, resized_video_size, output_file_name_string);
    video_writer = output_video->get_video_writer();

    ////////////////////////////////////////////////////////////////////////////
    // Log
    ////////////////////////////////////////////////////////////////////////////

    logger = new Logger(output_file_name_string);

    ////////////////////////////////////////////////////////////////////////////
    // GUI
    ////////////////////////////////////////////////////////////////////////////

#ifdef USER_INTERFACE
    user_interface = new UserInterface(* settings, resized_video_size);
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Histogram
    ////////////////////////////////////////////////////////////////////////////

    // Histogram ranges
    histogram_ranges[0] = 0;
    histogram_ranges[1] = 180;

    // Histogram for red
    histogram = (Mat_<float>(16, 1) << 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255);

    // Histogram for yellow
    //histogram = (Mat_<float>(16, 1) << 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    // Normalize histogram
    normalize(histogram, histogram, 0, 255, NORM_MINMAX);

    // Initialize object of interest to be in the top left corner
    // It does not matter that the object is not there. The algorithm will find it.
    object_of_interest = Rect(0, 0, 20, 20);

    // Begin tracking object
    object_selected = 1;

}

VictimTracker::VictimTracker(const VictimTracker& orig) {
}

VictimTracker::~VictimTracker() {

    // Close logs
    delete VictimTracker::logger;

    // Announce that the processing was finished
    cout << "Processing finished!" << endl;

}

/**
 * Get frame per seconds of the input video feed.
 * 
 * @return Frame per seconds
 */
double VictimTracker::get_input_video_fps() {
    double input_video_fps = video_capture.get(CV_CAP_PROP_FPS);

    // If the input is video stream, we have to calculate FPS manually
    if (input_video_fps == 0) {

        // Number of sample frames to capture
        int num_sample_frames = 50;

        // Start and end times
        time_t start, end;

        // Sample video frame
        Mat sample_frame;

        // Start timer
        time(&start);

        // Load sample frames
        for (int i = 0; i < num_sample_frames; i++) {
            video_capture >> sample_frame;
        }

        // End timer
        time(&end);

        // Compute elapsed time
        double time_difference = difftime(end, start);

        // Calculate frames per second
        input_video_fps = num_sample_frames / time_difference;

        // The maximum frame rate from MPEG 4 is 65.535
        if (input_video_fps > 65.535) {
            input_video_fps = 65.535;
        }

    }

    return input_video_fps;
}

/**
 * Get the resolution of the input video feed.
 */
void VictimTracker::get_input_video_size() {

    Size input_video_size(video_capture.get(CV_CAP_PROP_FRAME_WIDTH), video_capture.get(CV_CAP_PROP_FRAME_HEIGHT));

    // If the input video exceeds processing video size limits, we will have to resize it
    if (input_video_size.height > settings->PROCESSING_VIDEO_HEIGHT_LIMIT) {

        // Compute scale ratio
        double ratio = (double) settings->PROCESSING_VIDEO_HEIGHT_LIMIT / input_video_size.height;

        // Compute new width
        int new_video_width = input_video_size.width * ratio;

        // Set new size
        resized_video_size.height = settings->PROCESSING_VIDEO_HEIGHT_LIMIT;
        resized_video_size.width = new_video_width;

        // Set parameter for maximum blob area
        settings->MAX_BLOB_AREA = resized_video_size.height * resized_video_size.width;

        // Indicate that resizing is necessary
        resize_video = true;

    } else {

        // Set video size to original size
        resized_video_size.height = input_video_size.height;
        resized_video_size.width = input_video_size.width;

        // Set parameter for maximum blob area
        settings->MAX_BLOB_AREA = resized_video_size.height * resized_video_size.width;

        // Indicate that resizing is not necessary
        resize_video = false;

    }
}

/**
 * Equalize histogram of the given frame.
 * 
 * @param HSV_frame
 */
void VictimTracker::equalize(Mat& HSV_frame) {
    vector<Mat> HSV_planes;
    split(HSV_frame, HSV_planes);
    equalizeHist(HSV_planes[2], HSV_planes[2]);
    merge(HSV_planes, HSV_frame);
}

/**
 * Create histogram for the area of interest.
 * 
 * @param object_of_interest
 * @param histogram_size
 * @param pointer_histogram_ranges
 * @param hue
 * @param saturation_value_threshold
 * @param histogram
 * @param histogram_image
 */
void VictimTracker::create_histogram(Rect& object_of_interest, int& histogram_size, const float*& pointer_histogram_ranges, Mat& hue, Mat& saturation_value_threshold, Mat& histogram, Mat& histogram_image) {

    // Region of interest
    Mat region_of_interest(hue, selection);

    // Region of interest mask
    Mat region_of_interest_mask(saturation_value_threshold, selection);

    // Calculate histogram of region of interest
    calcHist(&region_of_interest, 1, 0, region_of_interest_mask, histogram, 1, &histogram_size, &pointer_histogram_ranges);

    // Print histogram
    cout << histogram << endl;

    // Normalize histogram
    normalize(histogram, histogram, 0, 255, NORM_MINMAX);

    // Set object of interest to selection
    object_of_interest = selection;

    // Begin tracking object
    object_selected = 1;

    // Create histogram visualization
    histogram_image = Scalar::all(0);
    int bins_width = histogram_image.cols / histogram_size;
    Mat buffer(1, histogram_size, CV_8UC3);
    for (int i = 0; i < histogram_size; i++)
        buffer.at<Vec3b>(i) = Vec3b(saturate_cast<uchar> (i * 180. / histogram_size), 255, 255);
    cvtColor(buffer, buffer, COLOR_HSV2BGR);
    for (int i = 0; i < histogram_size; i++) {
        int val = saturate_cast<int> (histogram.at<float> (i) * histogram_image.rows / 255);
        rectangle(histogram_image, Point(i*bins_width, histogram_image.rows), Point((i + 1) * bins_width, histogram_image.rows - val), Scalar(buffer.at<Vec3b>(i)), -1, 8);
    }
}

/**
 * Create one log entry with current system status.
 * 
 * @param logger
 * @param current_commands
 */
void VictimTracker::create_log_entry(Logger* logger) {

    // Get current time
    time_t raw_time;
    time(&raw_time);
    struct tm * local_time;
    local_time = localtime(&raw_time);
    char current_time[40];
    strftime(current_time, 40, "%Y%m%d%H%M%S", local_time);

    // Log time
    logger->log_general(current_time);
    logger->log_general(" ");

    // Log EMILY location
    logger->log_general(victim_location.x);
    logger->log_general(" ");
    logger->log_general(victim_location.y);
    logger->log_general(" ");

    // Log EMILY size
    logger->log_general(victim_size.height);
    logger->log_general(" ");
    logger->log_general(victim_size.width);
    logger->log_general(" ");

    // Log status
    logger->log_general(status);
    logger->log_general(" ");

    logger->log_general("\n");

}

/**
 * Update victim location history.
 * 
 * @param target_set
 */
void VictimTracker::update_history() {
    // Save current location to history
    emily_location_history[emily_location_history_pointer] = victim_location;

    // Update circular array pointer
    emily_location_history_pointer = (emily_location_history_pointer + 1) % settings->EMILY_LOCATION_HISTORY_SIZE;
}

/**
 * Show current object of interest selection in the GUI.
 */
void VictimTracker::show_selection() {
    if (select_object && selection.width > 0 && selection.height > 0) {
        Mat roi(original_frame, selection);
        bitwise_not(roi, roi);
    }
}

/**
 * Call in each iteration to track the victim.
 * 
 * @return 
 */
int VictimTracker::logic() {

    // If not paused       
    if (!paused) {

        // Read one frame
        video_capture >> original_frame;

        // End if frame is empty for long time
        if (original_frame.empty()) {

            empty_frame_counter++;

            if (empty_frame_counter < 1000) {
                return 0;
            } else {
                return -1;
            }

        } else {

            empty_frame_counter = 0;

        }

    }

    ////////////////////////////////////////////////////////////////////////
    // Preprocessing
    ////////////////////////////////////////////////////////////////////////

    if (resize_video) {

        // Resize the input
        resize(original_frame, original_frame, resized_video_size, 0, 0, INTER_LANCZOS4);

    }

    // Apply Gaussian blur filter
    GaussianBlur(original_frame, blured_frame, Size(settings->blur_kernel_size, settings->blur_kernel_size), 0, 0);

    // Convert to HSV color space
    Mat HSV_frame;
    cvtColor(blured_frame, HSV_frame, COLOR_BGR2HSV);

    // Equalize on value (V)
    equalize(HSV_frame);

    ////////////////////////////////////////////////////////////////////////
    // Thresholding
    ////////////////////////////////////////////////////////////////////////  

    if (!paused) {

        if (object_selected) {

            // Threshold on saturation and value, but not on hue
            inRange(HSV_frame, Scalar(0, settings->saturation_min, settings->value_min), Scalar(180, settings->saturation_max, settings->value_max), saturation_value_threshold);

            // Mix channels
            int chanels[] = {0, 0};
            hue.create(HSV_frame.size(), HSV_frame.depth());
            mixChannels(&HSV_frame, 1, &hue, 1, chanels, 1);

            // Uncomment this only if histogram should be created automatically
            //                // Object does not have histogram yet, so create it
            //                if (object_selected < 0) {
            //
            //                    // Create histogram of region of interest
            //                    create_histogram(object_of_interest, histogram_size, pointer_histogram_ranges, hue, saturation_value_threshold, histogram, histogram_image);
            //
            //                }

            // Calculate back projection
            calcBackProject(&hue, 1, 0, histogram, back_projection, &pointer_histogram_ranges);

            // Apply back projection on saturation value threshold
            back_projection &= saturation_value_threshold;

            // CamShift algorithm
            RotatedRect tracking_box = CamShift(back_projection, object_of_interest, TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));

            // Object of interest are is too small, so inflate the tracking box
            if (object_of_interest.area() <= 1) {
                int cols = back_projection.cols;
                int rows = back_projection.rows;
                int new_rectangle_size = (MIN(cols, rows) + 5) / 6;
                object_of_interest = Rect(object_of_interest.x - new_rectangle_size, object_of_interest.y - new_rectangle_size, object_of_interest.x + new_rectangle_size, object_of_interest.y + new_rectangle_size) & Rect(0, 0, cols, rows);
            }

            // We are in back projection mode
            if (back_projection_mode) {
                cvtColor(back_projection, original_frame, COLOR_GRAY2BGR);
            }

            // Draw bounding ellipse
            if (tracking_box.size.height > 0 && tracking_box.size.width > 0) {
                ellipse(original_frame, tracking_box, settings->LOCATION_COLOR, settings->LOCATION_THICKNESS, LINE_AA);

#ifdef USER_INTERFACE
                // Draw cross hairs
                user_interface->draw_position(tracking_box.center.x, tracking_box.center.y, min(tracking_box.size.width, tracking_box.size.height) / 2, original_frame);
#endif

                // Save EMILY location
                victim_location = Point(tracking_box.center.x, tracking_box.center.y);

                // Save EMILY size
                victim_size = tracking_box.size;

            }

        }
    } else if (object_selected < 0) {

        // Un pause if the selection has been made
        paused = false;
    }

    // Show the selection
    show_selection();

    // Show the histogram
    //user_interface->show_histogram(histogram_image);

    char character = (char) waitKey(10);
    
    if (character == 27)
        
        return -1;
    
    switch (character) {
        case 'b':

            // Toggle back projection mode
            back_projection_mode = !back_projection_mode;

            break;
        case 'c':

            // Stop tracking
            object_selected = 0;
            //histogram_image = Scalar::all(0);

            break;
        case 'p':

            // Toggle pause
            paused = !paused;

            break;
    }

    ////////////////////////////////////////////////////////////////////////
    // Compute heading
    ////////////////////////////////////////////////////////////////////////

    update_history();

    ////////////////////////////////////////////////////////////////////////
    // Show results
    ////////////////////////////////////////////////////////////////////////

    // Main Window
    ////////////////////////////////////////////////////////////////////////

#ifdef USER_INTERFACE
    // Get status as a string message
    user_interface->print_status(original_frame, status);

    Mat output;
    original_frame.copyTo(output);

    // Show output frame in the main window
    user_interface->show_main(output);
#endif

    ////////////////////////////////////////////////////////////////////////
    // Video output
    ////////////////////////////////////////////////////////////////////////

    // Write the frame to the output video
    video_writer << original_frame;
    //video_writer << threshold_color;

    ////////////////////////////////////////////////////////////////////////
    // Log output
    ////////////////////////////////////////////////////////////////////////

    // Debugging
    //cout << "Throttle: " << current_commands->get_throttle() << " Rudder: " << current_commands->get_rudder() << endl;

    // Log the data
    create_log_entry(logger);

    return 0;

}

/**
 * Get centroid of the victim.
 * 
 * @return 
 */
Point VictimTracker::getCenter() {

    return victim_location;

}

/**
 * Get size of the victim.
 * 
 * @return 
 */
Size2f VictimTracker::getSize() {

    return victim_size;

}