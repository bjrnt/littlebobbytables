#define BOOST_THREAD_USE_LIB
#include "App.h"
#include <boost/thread.hpp>
#include "SDL.h"
#include "SDL_thread.h"

using ::std::string;
using ::std::cout;
using ::std::cerr;
using ::std::endl;

using namespace tetio;

std::pair<int,int> * resolution;
int prevXGazePos = 0;
int prevYGazePos = 0;

App::App() :
	trackerId_(""),
	trackerFound_(false),
	debug_(false)
{
}

// Starts the eyetracking and then destroys it.
//
// resolution : pointer to the resolution vector
// runner  : pointer to the thread where the eyetracker resides
// tracker : pointer to the EyeTracker-pointer
//
// Original author: Tobii
// Modified by: Christoffer & Andreas
// Version: 18-02-2013
int App::run(std::pair<int,int> * res, MainLoopRunner* runner, EyeTracker::pointer_t* tracker)//int argc, char *argv[])
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
void App::checkForAvailableEyeTracker(EyeTrackerBrowser::event_type_t type, EyeTrackerInfo::pointer_t info)
{
	if (type == EyeTrackerBrowser::TRACKER_FOUND) {
		trackerFound_ = true;
		//Set trackerid to this eyetracker
		trackerId_ = info->getProductId();
	}
}

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
//
// Author: Christoffer & Andreas
// Version: 18-02-2013
void App::onGazeDataReceived(tetio::GazeDataItem::pointer_t data)
{
	//Only accept valid data (see table in SDK manual for more info)
	if(data->leftValidity<2 && data->rightValidity<2){
		//Preserve a bit of the old gaze pos (will handle a bit of the noise)
		int gazePosX = (int)(0.2*(resolution->first*(data->leftGazePoint2d.x + data->rightGazePoint2d.x)/2));
		int gazePosY = (int)(0.2*(resolution->second*(data->leftGazePoint2d.y + data->rightGazePoint2d.y)/2));
        gazePosX += (int)(0.8*prevXGazePos);
        gazePosY += (int)(0.8*prevYGazePos);

        //Check if current gaze is within the boundaries of the screen
        if(!(gazePosX < 0 || gazePosY < 0)){
            /*SDL_Event ev;
            ev.type = SDL_MOUSEMOTION; //SDL_MOUSEBUTTONDOWN; //SDL_USEREVENT;
            ev.motion.x = gazePosX;
            ev.motion.y = gazePosY;
            ev.motion.state = SDL_MOUSEBUTTONDOWN;
            ev.motion.xrel = 0;
            ev.motion.yrel = 0;
            ev.button.button = SDL_BUTTON_LEFT;
            ev.button.type = SDL_MOUSEBUTTONDOWN;
            ev.button.state = SDL_PRESSED;
            ev.button.which = 0;
            ev.button.x = gazePosX;
            ev.button.y = gazePosY;
            */
            if(debug_)cout << "Current Pos: " << gazePosX << " " << gazePosY << endl;
            //Send mousebutton down event
            /*while (-1 == SDL_PushEvent(&ev))
            {}
            */
            SDL_WarpMouse(gazePosX,gazePosY);
            prevXGazePos = gazePosX;
            prevYGazePos = gazePosY;
            /*ev.type = SDL_MOUSEBUTTONUP;
            ev.button.type=SDL_MOUSEBUTTONUP;

            //Send mousebutton up event
            while (-1 == SDL_PushEvent(&ev))
            {}
            */
        }
        if(debug_)cout << data->timestamp << "\t" << data->leftGazePoint2d.x << " " << data->leftGazePoint2d.y << "\t" << data->rightGazePoint2d.x << " " << data->rightGazePoint2d.y << "\t" << endl;
	}
}

// This function begins to get tracking data from the eyetracker and starts the thread it uses.
//
// tracker : Pointer to EyeTracker
// runner  : Thread that EyeTracker uses
// Original author: Tobii
// Modified by: Andreas & Christoffer
// Version: 15-02-2013
void App::startEyeTracker(EyeTracker::pointer_t* tracker, MainLoopRunner* runner)
{
	//Copied from list
	trackerFound_ = false;
	runner->start(); //Start the thread
	EyeTrackerBrowser::pointer_t browser(EyeTrackerBrowserFactory::createBrowser(runner->getMainLoop()));
	browser->addEventListener(boost::bind(&App::checkForAvailableEyeTracker, this, _1, _2));
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

				(*(tracker))->addGazeDataReceivedListener(boost::bind(&App::onGazeDataReceived, this, _1));

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
			cout << " " << e.what() << " " << e.getErrorCode() << endl;
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
// Version : 01-03-2013
void App::exitEyeTracker(EyeTracker::pointer_t* tracker, MainLoopRunner* runner)
{
    //If we didn't have any Eyetracker conncected, do not try to stop it
	if(*tracker != NULL)
    {
        (*tracker)->stopTracking();
    }
    if(debug_)cerr << "Eyetracker was shut down" << endl;
	runner->stop();
	if(debug_)cerr << "RunnerThread was shut down" << endl;
}
