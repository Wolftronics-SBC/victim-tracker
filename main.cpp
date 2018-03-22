/** 
 * @file    main.cpp
 * @author  Jan Dufek
 * @date    10/25/2017
 * @version 1.0
 *  
 * This is an example on how to use VictimTracker class.
 * VictimTracker class is used to track a victim wearing a red or yellow life jacket in the input video feed.
 *
 */

#include "opencv2/opencv.hpp"
#include "VictimTracker.hpp"

using namespace cv;

// The following three variables are used as externs and must be defined as global

// Select object flag
bool select_object = false;

// Track object mode toggle
int object_selected = 0;

// Object selection
Rect selection;


int main(int argc, char** argv) {

    // Initialize VictimTracker class
    // User interface can be disabled by going in VictimTracker.hpp and commenting "#define USER_INTERFACE" line
    VictimTracker * victimTracker = new VictimTracker();

    // This is the main loop
    while (true) {

        // Call logic function on VictimTracker in each iteration of the main loop
        if (victimTracker->logic() == -1) {
            
            // If VictimTracker returns -1, that means that it wants to terminate
            
            // Delete VictimTracker
            delete victimTracker;
            
            // Break the main loop
            break;
            
        }

        // Ask VictimTracker for current center of the victim
        Point center = victimTracker->getCenter();
        
        // Print current center of the victim to the console
        // Note: do not print anything to the console while running on EMILY, otherwise it will slow things down
        cout << "Center: " << center.x << " " << center.y << ". ";

        // Ask VictimTracker for current size of the victim
        Size2f size = victimTracker->getSize();
        
        // Print current center of the victim to the console
        cout << "Size: " << size.height << " " << size.width << endl;

    }

    // Clean return
    return 0;
}