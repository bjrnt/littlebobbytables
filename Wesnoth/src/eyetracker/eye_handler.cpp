#define BOOST_THREAD_USE_LIB
#include "eye_handler.h"
#include <boost/thread.hpp>
#include "SDL.h"
#include "SDL_thread.h"
#include "preferences.hpp"
#include "eyetracker/interaction_controller.hpp"

#include "time.h"
#include "math.h"

using ::std::string;
using ::std::cout;
using ::std::cerr;
using ::std::endl;

using namespace tetio;

#define BLINK_BOUNDARY_X 0.2
#define BLINK_BOUNDARY_Y 0.15
clock_t time_at_blink_;
bool eyesFound_    = false;
bool blinking_     = false;

std::pair<int,int> * resolution;
int prevXGazePos_  = 0;
int prevYGazePos_  = 0;

eye_handler::eye_handler() :
	trackerId_(""),
	trackerFound_(false),
	debug_(true)
{
}

// Starts the eyetracking.
//
// resolution : pointer to the resolution vector
// runner  : pointer to the thread where the eyetracker resides
// tracker : pointer to the EyeTracker-pointer
//
// Original author: Tobii
// Modified by: Christoffer & Andreas
// Version: 18-02-2013
int eye_handler::run(std::pair<int,int> * res, MainLoopRunner* runner, EyeTracker::pointer_t* tracker)
{
	resolution = res;
	startEyeTracker(tracker,runner);
	return 0;
}

// Checks for any available eyetrackers and gets if ProductId if located.
//
// Original author: Tobii
// Modified by: Christoffer & Andreas
// Version: 15-02-2013
void eye_handler::checkForAvailableEyeTracker(EyeTrackerBrowser::event_type_t type, EyeTrackerInfo::pointer_t info)
{
	if (type == EyeTrackerBrowser::TRACKER_FOUND) {
		trackerFound_ = true;
		//Set trackerid to this eyetracker
		trackerId_ = info->getProductId();
	}
}

// Original author: Tobii
void startEyeTrackerLookUp(EyeTrackerBrowser::pointer_t browser, std::string browsingMessage)
{
	browser->start();
	// wait for eye trackers to respond.
#ifdef __APPLE__
    // Slight different bonjuor behaviour on Mac vs Linux/Windows, ... On MAC this needs to be > 30 seconds
	if(debug_)cout << browsingMessage << endl;
	boost::this_thread::sleep(boost::posix_time::milliseconds(60000));
#else
	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
#endif
	browser->stop(); // NOTE this is a blocking operation.
}

// On receive of GazeData, do the following:
// Check if valid and within bounds.
// Also check if a Blink event occured, if so send out a SDL_UserEvent.
//
// Author: Christoffer & Andreas
// Version: 22-03-2013
void eye_handler::onGazeDataReceived(tetio::GazeDataItem::pointer_t data)
{
	//Only accept valid data (see table in SDK manual for more info)
	if(data->leftValidity<2 && data->rightValidity<2){
		//Preserve a bit of the old gaze pos (will handle a bit of the noise)
		//if(debug_) cerr << "Res: " << resolution->first << " " << resolution->second << endl;
		//if(debug_) cerr << "Res address: " << resolution << endl;
		int gazePosX = (int)(0.2*(resolution->first*(data->leftGazePoint2d.x + data->rightGazePoint2d.x)/2));
		int gazePosY = (int)(0.2*(resolution->second*(data->leftGazePoint2d.y + data->rightGazePoint2d.y)/2));
        gazePosX += (int)(0.8*prevXGazePos_);
        gazePosY += (int)(0.8*prevYGazePos_);
        eyesFound_ = true;

        //Check if current gaze is within the boundaries of the screen
        if(!(gazePosX < 0 || gazePosY < 0)){

            //Currently blinking
            if(blinking_){
                blinking_ = false;
                int current_blink_length = round(1000*((float)(clock() - time_at_blink_))/(CLOCKS_PER_SEC)); //Calculate nr of ms that blink lasted
                int blink_length_original = preferences::blink_length();

                if(debug_)cerr << "Diff-time: " << current_blink_length  << endl;
                if(debug_)cerr << "Blink-length: " << blink_length_original << endl;

                //Check that the blink was done inside of the blink boundary
                //(so that our gaze point does not change too much) and that it was long enough
                if((abs(prevXGazePos_-gazePosX) < BLINK_BOUNDARY_X*resolution->first)  &&
                    (abs(prevYGazePos_-gazePosY) < BLINK_BOUNDARY_Y*resolution->second) &&
                     current_blink_length >= blink_length_original){
                   eyetracker::interaction_controller::blink();
                   if(debug_)cerr << "Pushed out Blink event" << endl;
                }
            }

            if(debug_)cout << "Current Pos: " << gazePosX << " " << gazePosY << endl;
            SDL_WarpMouse(gazePosX,gazePosY);
            prevXGazePos_ = gazePosX;
            prevYGazePos_ = gazePosY;
        }
        if(debug_)cout << data->timestamp << "\t" << data->leftGazePoint2d.x << " " << data->leftGazePoint2d.y << "\t" << data->rightGazePoint2d.x << " " << data->rightGazePoint2d.y << "\t" << endl;
	}
	//Only detect blinking if blinking mode is enabled and once the user has been detected
	//(Validity = 4 == No eye present i.e. a potential blink)
	else if(preferences::interaction_blink() && eyesFound_ && !blinking_ && data->leftValidity==4 && data->rightValidity==4)
	{
	    eyesFound_ = false;    //Set that we have closed our eyes
	    blinking_  = true;
	    time_at_blink_ = clock(); //Save the time when we started blinking
	}
}

// This function starts getting tracking data from the eyetracker and starts the thread it uses.
//
// tracker : Pointer to EyeTracker
// runner  : Thread that EyeTracker uses
// Original author: Tobii
// Modified by: Andreas & Christoffer
// Version: 15-02-2013
void eye_handler::startEyeTracker(EyeTracker::pointer_t* tracker, MainLoopRunner* runner)
{
	//Copied from list
	trackerFound_ = false;
	runner->start(); //Start the thread
	EyeTrackerBrowser::pointer_t browser(EyeTrackerBrowserFactory::createBrowser(runner->getMainLoop()));
	browser->addEventListener(boost::bind(&eye_handler::checkForAvailableEyeTracker, this, _1, _2));
	startEyeTrackerLookUp(browser, "Browsing for eye trackers, please wait ...");

	//Did we manage to find an eyetracker?
	if(trackerFound_){
		uint32_t tetserverPort = 0;
		uint32_t syncPort = 0;

		if(debug_)cout << "Tracking with Eye Tracker: " << trackerId_ << endl;

		try
		{
			EyeTrackerFactory::pointer_t eyeTrackerFactory = EyeTrackerBrowserFactory::createEyeTrackerFactoryByIpAddressOrHostname(trackerId_, tetserverPort, syncPort);

			//Only create EyeTracker pointer if Factory was succesfully created
			if (eyeTrackerFactory) {
				if(debug_)cout << "Connecting ..." << endl;

				(*tracker) = eyeTrackerFactory->createEyeTracker(runner->getMainLoop());

				if(debug_)cout << "Created EyeTracker pointer" << endl;

				(*(tracker))->addGazeDataReceivedListener(boost::bind(&eye_handler::onGazeDataReceived, this, _1));

				if(debug_)cout << "Added GazeDataReceivedListener" << endl;

				(*(tracker))->startTracking();

				if(debug_)cout << "Started tracking" << endl;
			}
			else {
				cerr << "The specified eyetracker could not be found." << endl;
			}

		}
		catch (EyeTrackerException e)
		{
			cerr << " " << e.what() << " " << e.getErrorCode() << endl;
		}
	}
	else
	{
		cerr << "No eyetracker could be found." << endl;
	}
}

// This function shutsdown the eyetracker and closes the thread it uses.
//
// tracker : Pointer to EyeTracker
// runner  : Thread that EyeTracker uses
// Author  : Andreas & Christoffer
// Version : 05-03-2013
void eye_handler::exitEyeTracker(EyeTracker::pointer_t* tracker, MainLoopRunner* runner)
{
    //If we didn't have any Eyetracker conncected, do not try to stop it
	if(*tracker != NULL)
    {
        (*tracker)->stopTracking();
        if(debug_)cerr << "Eyetracker was shut down" << endl;
    }
	runner->stop();
	if(debug_)cerr << "RunnerThread was shut down" << endl;
}
