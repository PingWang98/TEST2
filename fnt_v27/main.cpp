/*
 * Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
 * This file is part of MindControl.
 *
 * MindControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MindControl s distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MindControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * For the most up to date version of this software, see:
 * https://github.com/samuellab/mindcontrol
 *
 *
 *
 * NOTE: If you use any portion of this code in your research, kindly cite:
 * Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
 * 	"Optogenetic manipulation of neural activity with high spatial resolution in
 *	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
 */

/*
 * ~/workspace/OpticalMindControl/main.cpp
 * main.cpp
 *
 *  Created on: Jul 20, 2009
 *      Author: Andy
 */

/*
 * This is the main file for the MindControl software.
 *
 * This file starts two parallel threads. One thread is responsible for displaying
 * images, interacting with the user and manipulating the microscope stage.
 * The other thread reads in images of a moving Neuron and generates illumination patterns
 * corresponding to targets on that Neuron which are then transmitted to a digital
 * micromirror device.
 *
 *
 */

//Standard C headers
//#include <unistd.h>
#pragma comment  (lib,"User32.lib")
#pragma comment  (lib,"Gdi32.lib")
#include <stdio.h>
#include <ctime>
#include <time.h>
#include <conio.h>
#include <math.h>
//#include <sys/time.h>

//subtitution for headers in unix
//#include "3rdPartyLibs/getopt.h"


//Windows Header
#include <windows.h>

//C++ header
#include <iostream>
#include <limits>

using namespace std;

//OpenCV Headers
#include <opencv\highgui.h>
#include <opencv\cv.h>
#include <opencv\cxcore.h>

//Andy's Personal Headers
#include "MyLibs/AndysOpenCVLib.h"
#include "MyLibs/Talk2BaslerCamera.h"
#include "MyLibs/AndysComputations.h"
#include "MyLibs/NeuronAnalysis.h"
#include "MyLibs/WriteOutNeuron.h"
#include "MyLibs/experiment.h"


//3rd Party Libraries
#include "3rdPartyLibs/tictoc.h"

/** Global Variables (for multithreading) **/
UINT Thread1(LPVOID lpdwParam);
UINT Thread2(LPVOID lpdwParam);

//IplImage* CurrentImg;
bool DispThreadHasStarted;
bool TrackThreadHasStarted;
bool MainThreadHasStopped;
bool DispThreadHasStopped;
bool TrackThreadHasStopped;
bool UserWantsToStop;

int main(int argc, char** argv) {
	int DEBUG = 0;
	if (DEBUG) {
		cvNamedWindow("Debug");
		//	cvNamedWindow("Debug2");
	}

	/** Display output about the OpenCV setup currently installed **/
	DisplayOpenCVInstall();

	/** Create a new experiment object **/
	Experiment* exp = CreateExperimentStruct();

	/** Create memory and objects **/
	InitializeExperiment(exp);
	exp->e = 0; //set errors to zero.

	/** Deal with CommandLineArguments **/
	LoadCommandLineArguments(exp, argc, argv);
	if (HandleCommandLineArguments(exp) == -1)
		return -1;

	/** Start Camera Input **/
	RollCameraInput(exp);

	/** Quit now if we have some errors **/
	if (exp->e != 0)
		return -1;

	/** Setup Segmentation Gui **/
	//AssignWindowNames(exp);

	/** Start New Thread1 **/
	DWORD dwThreadId1;
	HANDLE hThread1 = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) Thread1,
			(void*) exp, 0, &dwThreadId1);
	if (hThread1 == NULL) {
		printf("Cannot create display thread.\n");
		return -1;
	}

	/** Start New Thread2 **/
	if (exp->stageIsPresent) {
		DWORD dwThreadId2;
		HANDLE hThread2 = CreateThread(NULL, 0,
				(LPTHREAD_START_ROUTINE) Thread2, (void*) exp, 0, &dwThreadId2);
		if (hThread2 == NULL) {
			printf("Cannot create tracking thread.\n");
			return -1;
		}
	}

	// wait for thread
	DispThreadHasStarted = FALSE;
	DispThreadHasStopped = FALSE;
	TrackThreadHasStarted = FALSE;
	TrackThreadHasStopped = FALSE;
	MainThreadHasStopped = FALSE;

	while ((!DispThreadHasStarted)||((exp->stageIsPresent) && (!TrackThreadHasStarted)))
		Sleep(10);

	/** SetUp Data Recording **/
	exp->e=SetupRecording(exp);


	/*Start the frame rate timer */
	StartFrameRateTimer(exp);

	/** Quit now if we have some errors **/
	if (exp->e != 0)
		return -1;

	/** Giant While Loop Where Everything Happens **/
	//TICTOC::timer().tic("WholeLoop");
	UserWantsToStop = 0;
	while (UserWantsToStop != 1) {
		TICTOC::timer().tic("OneLoop");

		/** Set error to zero **/
		exp->e = 0;
		TICTOC::timer().tic("GrabFrame()");
		/** Grab a frame **/

		int ret = 0;
		ret = GrabFrame(exp);
		TICTOC::timer().toc("GrabFrame()");
		if (ret == -1)
			continue;

		/** Calculate the frame rate and every second print the result **/
		CalculateAndPrintFrameRate(exp);



		/** Do we even bother doing analysis?**/
		if (exp->Params->OnOff == 0) {
			/**Don't perform any analysis**/;
			continue;
		}

		/** Load Image into Our Neuron Objects **/

		if (exp->e == 0)
			exp->e = LoadNeuronImg(exp->Neuron, exp->Params, exp->fromCCD->iplimg, exp->stageFeedbackTargetOffset);




		TICTOC::timer().tic("EntireSegmentation");
		/** Do Segmentation **/
		DoSegmentation(exp);
		TICTOC::timer().toc("EntireSegmentation");

		if (exp->e != 0) {
			printf("\nError in main loop. :(\n");
			if (exp->stageIsPresent) {
				printf("\tAuto-safety STAGE SHUTOFF!\n");
				ShutOffStage(exp);
			}
			/** Write Values to Disk **/
		}

		TICTOC::timer().tic("DoWriteToDisk()");
		DoWriteToDisk(exp);
		TICTOC::timer().toc("DoWriteToDisk()");

		if (UserWantsToStop)
			break;
		TICTOC::timer().toc("OneLoop");

		//if (exp->e == 0
		//		&& EverySoOften(exp->Neuron->frameNum, exp->Params->DispRate)) {
		//	TICTOC::timer().tic("DisplayOnScreen");
			/** Setup Display but don't actually send to screen **/
		//	PrepareSelectedDisplay(exp);
		//	TICTOC::timer().toc("DisplayOnScreen");
		//}

	}
	/** Shut down the main thread **/

	//TICTOC::timer().toc("WholeLoop");
	/** Tell the display thread that the main thread is shutting down**/
	MainThreadHasStopped = TRUE;

	TICTOC::timer().tic("FinishRecording()");
	FinishRecording(exp);
	TICTOC::timer().toc("FinishRecording()");
	/***** Turn off Camera & DLP ****/
	T2Cam_TurnOff(*(exp->MyCamera));
	T2Cam_CloseLib();

	printf("%s", TICTOC::timer().generateReportCstr());
	if (!DispThreadHasStopped) {
		printf("Waiting for DisplayThread to Stop...");

	}
	while (!DispThreadHasStopped) {
		printf(".");
		Sleep(500);
		cvWaitKey(10);
	}

	while ((exp->stageIsPresent)&&(!TrackThreadHasStopped)) {
		printf(".");
		Sleep(500);
		cvWaitKey(10);
	}

	if (exp->stageIsPresent)
		ShutOffStage(exp);
	ReleaseExperiment(exp);
	DestroyExperiment(&exp);

	printf("\nMain Thread: Good bye.\n");
	return 0;
}

/**
 * Thread to display image. 
 */
UINT Thread1(LPVOID lpdwParam) {

	Experiment* exp = (Experiment*) lpdwParam;
	printf("DisplayThread: Hello!\n");
	MSG Msg;

	SetupGUI(exp);
	cvWaitKey(30);
//	SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);

	DispThreadHasStarted = TRUE;
	printf("DisplayThread has started! \n");
	cvWaitKey(30);

	int key;
	int k = 0;
	while (!MainThreadHasStopped) {

		//needed for display window
		if (PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))
			DispatchMessage(&Msg);

		TICTOC::timer().tic("DisplayThreadGuts");
		TICTOC::timer().tic("cvShowImage");

		if (exp->Params->OnOff) {
			TICTOC::timer().tic("DisplayOnScreen");

			if (exp->e==0) PrepareSelectedDisplay(exp);

			TICTOC::timer().toc("DisplayOnScreen");

		} else {

			cvShowImage(exp->WinDisp, exp->fromCCD->iplimg);
		}
		TICTOC::timer().toc("cvShowImage");

		if (MainThreadHasStopped == 1)
			continue;

		TICTOC::timer().toc("DisplayThreadGuts");
		UpdateGUI(exp);

		key = cvWaitKey(100);   

		if (MainThreadHasStopped == 1)
			continue;

		if (HandleKeyStroke(key, exp)) {
			printf("\n\nEscape key pressed!\n\n");

			/** Let the Other thread know that the user wants to stop **/
			UserWantsToStop = 1;

			/** Emergency Shut off the Stage **/
			printf("Emergency stage shut off.");
			if (exp->stageIsPresent)
				ShutOffStage(exp);

			/** Exit the display thread immediately **/
			DispThreadHasStopped = TRUE;
			printf("\nDisplayThread: Goodbye!\n");
			return 0;

		}

		UpdateGUI(exp);

	}

	printf("\nDisplayThread: Goodbye!\n");
	DispThreadHasStopped = TRUE;
	return 0;
}

UINT Thread2(LPVOID lpdwParam) {

	Experiment* exp = (Experiment*) lpdwParam;
	printf("TrackingThread: Hello!\n");

	printf("TrackingThread: invoking stage...\n ");
	InvokeStage(exp);
	
	TrackThreadHasStarted=TRUE;

	printf("TrackingThread: Starting loop\n");

	int k = 0;
	
	

	while (!MainThreadHasStopped) {

		if (EverySoOften(k, 1)) { //This determines how often the stage is updated

			/** Do the Stage Tracking **/
			TICTOC::timer().tic("HandleStageTracker()");
			HandleStageTracker(exp);
			TICTOC::timer().toc("HandleStageTracker()");
		}

		k++;
		cvWaitKey(30);

	}

	if (exp->stageIsPresent)
		ShutOffStage(exp);

	printf("\nTrackingThread: Goodbye!\n");
	TrackThreadHasStopped = TRUE;
	return 0;

}

