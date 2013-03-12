#ifndef __EYEHANDLER_H_
#define __EYEHANDLER_H_

#include <string>
#include <tobii/sdk/cpp/EyeTracker.hpp>
#include <tobii/sdk/cpp/EyeTrackerBrowser.hpp>
#include <tobii/sdk/cpp/EyeTrackerBrowserFactory.hpp>
#include <tobii/sdk/cpp/EyeTrackerInfo.hpp>
#include "MainLoopRunner.h"

#define BLINK_EVENT (SDL_USEREVENT+6) //Use the event next after the last defined USER_EVENT

namespace tetio = tobii::sdk::cpp;

class eye_handler
{
public:
	eye_handler();
	int run(std::pair<int,int> * res, MainLoopRunner* runner, tetio::EyeTracker::pointer_t* tracker);
    void exitEyeTracker(tetio::EyeTracker::pointer_t* tracker, MainLoopRunner* runner);
private:
	void startEyeTracker(tetio::EyeTracker::pointer_t* tracker, MainLoopRunner* runner);

	void checkForAvailableEyeTracker(tetio::EyeTrackerBrowser::event_type_t type, tetio::EyeTrackerInfo::pointer_t info);

	void onGazeDataReceived(tetio::GazeDataItem::pointer_t data);

	std::string trackerId_;
	bool trackerFound_;
	bool debug_;
};

#endif // __EYEHANDLER_H_
