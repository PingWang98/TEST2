/*
 * experiment.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: quan
 */
/*
 /*
 * experiment.c
 *
 *  Created on: Sep 3, 2013
 *      Author: quan modified from Andy's code
 *
 *  The experiment.c/.h library is designed to be an extremely high level library.
 *	The idea here is to have all of the elements of an experiment laid out, such that
 *	a user need only to call a few high level functions to run an experiment.
 */

//Standard C headers
//#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>
#include <math.h>
#include <assert.h>
//#include <sys/time.h>

//subtitutions for UNIX in Windows
#include "../3rdPartyLibs/getopt.h" //replacement for unistd.h

//OpenCV Headers
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <opencv/cxcore.h>

//Timer Lib
#include "../3rdPartyLibs/tictoc.h"

//Andy's Personal Headers
#include "Talk2BaslerCamera.h"
#include "Talk2Stage.h"
#include "NeuronAnalysis.h"
#include "AndysOpenCVLib.h"
#include "AndysComputations.h"
#include "WriteOutNeuron.h"


#include "experiment.h"

/*
 * Creates a new experiment object and sets values to zero.
 */
Experiment* CreateExperimentStruct() {

	/** Create Experiment Object **/
	Experiment* exp;
	exp = (Experiment*) malloc(sizeof(Experiment));

	/*************************************/
	/**  Set Everything to zero or NULL **/
	/*************************************/
	
	/** Simulation? True/False **/

	/** GuiWindowNames **/
	exp->WinDisp = "Display";
	exp->WinCon1 = "Control";
	
	

	/** Error information **/
	exp->e = 0;

	/** CommandLine Input **/
	exp->argv = NULL;
	exp->argc = 0;
	exp->outfname = "w1";
	exp->infname = NULL;
	exp->dirname = "F:\\FntRecording\\w1\\";

	/** Camera Input**/
	exp->MyCamera = NULL;

	/** User-configurable Neuron-related Parameters **/
	exp->Params = NULL;

	/** Information about Our Neuron **/
	exp->Neuron = NULL;

	/** internal IplImage **/
	exp->SubSampled = NULL; // Image used to subsample stuff
	exp->lastFrameSeenOutside = 0;
	exp->CurrentSelectedImg = NULL; //The current image selected for display

	/** Write Data To File **/
	exp->DataWriter = NULL;

	/** Write Video To File **/
	exp->Vid = NULL; //Video Writer


	/** Timing  Information **/
	exp->now = 0;
	exp->last = 0;

	/** Frame Rate Information **/
	exp->nframes = 0;
	exp->prevFrames = 0;
	exp->prevTime = 0;

	/** Stage Control **/
	exp->stageIsPresent = 1;
	exp->stage = NULL;
	exp->stageVel = cvPoint(0, 0);
	exp->stageCenter = cvPoint(0, 0);
	exp->stageFeedbackTargetOffset = cvPoint(0, 0);
	exp->stageIsTurningOff = 0;

	/** Information about the Previous frame's Worm **/


	/** Error Handling **/
	exp->e = 0;

	return exp;

}

/*
 * Load the command line arguments into the experiment object
 */
void LoadCommandLineArguments(Experiment* exp, int argc, char** argv) {
	exp->argc = argc;
	exp->argv = argv;
}

void displayHelp() {
	printf(
			"\n\n this software analyzes each frame, finds the neuron and track the neuron in real time\n");
	printf(
			"by Quan Wen, qwen@fas.harvard.edu, modified from Andy Leifer's code");
	printf("\nUsage:\n\n");
	printf(
			"If run with no arguments,  it will identify the neuron, but you need to track the neuron by hand.\n\n");
	printf("Optional arguments:\n");

	printf("\t-t\n\t\tUse USB stage tracker.\n\n");
	printf(
			"\t-x\n\tx 100\tSpecifies the x offset from center for the Neuron's location in the stage feedback trap. +x is to the right of screen.\n\n");
	printf(
			"\t-y\n\ty -100\tSpecifies the y offset from center for the Neuron's location in the stage feedback trap. +y is towards bottom of screen.\n\n");
	printf("\t-?\n\t\tDisplay this help.\n\n");
	printf("\nSee shortcutkeys.txt for a list of keyboard shortcuts.\n");
}

/*
 * Handle CommandLine Arguments
 * Parses commandline arguments.
 * Decides if user wants to record video or recorddata
 */

int HandleCommandLineArguments(Experiment* exp) {
	int dflag = 0;
	opterr = 0;

	int c;
	while ((c = getopt(exp->argc, exp->argv, "si:d:o:p:gtx:y:?")) != -1) {
		switch (c) {
		case 't': /** Use the stage tracking software **/
			exp->stageIsPresent = 1;
			break;

		case 'x': /** adjust the target for stage feedback loop by these certain number of pixels **/
			if (optarg != NULL) {
				exp->stageFeedbackTargetOffset.x = atoi(optarg);
			}
			printf(
					"Adjusting target for stage feedback loop by x= %d pixels.\n",
					exp->stageFeedbackTargetOffset.x);
			break;

		case 'y': /** adjust the target for stage feedback loop by these certain number of pixels **/
			if (optarg != NULL) {
				exp->stageFeedbackTargetOffset.y = atoi(optarg);
			}
			printf(
					"Adjusting target for stage feedback loop by y= %d pixels.\n",
					exp->stageFeedbackTargetOffset.y);
			break;

		case '?':
			displayHelp();
			return -1;
			break;

		default:
			displayHelp();
			return -1;
		} // end of switch

	} // end of while loop
	return 1;
}

/** GUI **/

/* Assigns Default window names to the experiment object
 *
 */
void AssignWindowNames(Experiment* exp) {

	char* disp1 = (char*) malloc(strlen("Display"));
	char* control1 = (char*) malloc(strlen("Controls"));
	//disp1 = "Display";
	//control1 = "Controls";
	char dis[] = "Display";
	char cont[] = "Controls";
	disp1 = dis;
	control1 = cont;
	exp->WinDisp = disp1;
	exp->WinCon1 = control1;

}

/*
 * Release the memopry associated with window names
 * and set their pointers to Null
 */
void ReleaseWindowNames(Experiment* exp) {
	/*if (exp->WinDisp != NULL)
		free(exp->WinDisp);
	if (exp->WinCon1 != NULL)
		free(exp->WinCon1);
*/
	exp->WinDisp = NULL;
	exp->WinCon1 = NULL;

}

/*
 * SetupGui
 *
 */

void SetupGUI(Experiment* exp) {

	printf("Beginning to setup GUI\n");

	cvNamedWindow(exp->WinDisp); // <-- This goes into the thread.
	cvNamedWindow(exp->WinCon1);
	cvResizeWindow(exp->WinCon1, 500, 400);

	/** SelectDisplay **/
	cvCreateTrackbar("SelectDisplay", exp->WinCon1, &(exp->Params->Display), 2,
			NULL);
	printf("Pong\n");

	/** On Off **/
	cvCreateTrackbar("On", exp->WinCon1, &(exp->Params->OnOff), 1, NULL);

	/** Segmentation Parameters**/
	cvCreateTrackbar("Threshold", exp->WinCon1, &(exp->Params->BinThresh), 255,
			NULL);
			
	cvCreateTrackbar("Diameter", exp->WinCon1, &(exp->Params->MaskDiameter),NSIZEY,NULL);

	if (exp->stageIsPresent) {
		cvCreateTrackbar("StageSpeedFactor", exp->WinCon1,
				&(exp->Params->stageSpeedFactor), 100, NULL);
	}

	if (exp->stageIsPresent) {
		cvCreateTrackbar("MaxStageSpeed", exp->WinCon1, &(exp->Params->maxstagespeed),100,NULL);
	}

	printf("Created trackbars and windows\n");
	return;

}

/*
 * Update's trackbar positions for variables that can be changed by the software
 *
 */
void UpdateGUI(Experiment* exp) {

	/** Threshold **/
	cvSetTrackbarPos("Threshold", exp->WinCon1, (exp->Params->BinThresh));
	
	cvSetTrackbarPos("Diameter", exp->WinCon1, (exp->Params->MaskDiameter));

	cvSetTrackbarPos("On",exp->WinCon1,(exp->Params->OnOff));

	cvSetTrackbarPos("SelectDisplay",exp->WinCon1,(exp->Params->Display));

	/**Stage Speed **/
	if (exp->stageIsPresent){
		cvSetTrackbarPos("StageSpeedFactor", exp->WinCon1,
				(exp->Params->stageSpeedFactor));

		cvSetTrackbarPos("MaxStageSpeed", exp->WinCon1,
						(exp->Params->maxstagespeed));


	}

	return;

}

/*** Start Video Camera ***/

/*
 * Initialize camera library
 * Allocate Camera Data
 * Select Camera and Show Properties dialog box
 * Start Grabbing Frames as quickly as possible
 * *
 * OR open up the video file for reading.
 */
void RollCameraInput(Experiment* exp) {

	/** Use Basler USB Camera **/
	int ret = 0;
	/** Turn on Camera **/
	T2Cam_InitializeLib();
	T2Cam_AllocateCamDataStructMemory(&(exp->MyCamera));
	ret = T2Cam_CreateSpecificCamera(*(exp->MyCamera));
	/** Start Grabbing Frames and Update the Internal Frame Number iFrameNumber **/
	if (ret == 0)
	{
		ret = T2Cam_GrabFramesAsFastAsYouCan(*(exp->MyCamera));
		if (ret != 0)
		{
			T2Cam_TurnOff(*(exp->MyCamera));
			T2Cam_CloseLib();
			exp->e = EXP_ERROR;
		}
	}
	else
	{
		T2Cam_TurnOff(*(exp->MyCamera));
		T2Cam_CloseLib();
		exp->e = EXP_ERROR;
	}
}

/*
 * This function allocates images and frames
 * And a Neuron Object
 *
 * And a Parameter Object
 * For internal manipulation
 *
 *
 */
void InitializeExperiment(Experiment* exp) {

	/*** Create IplImage **/
	IplImage* SubSampled = cvCreateImage(cvSize(NSIZEX / 2, NSIZEY / 2),
		IPL_DEPTH_8U, 1);
	exp->CurrentSelectedImg = cvCreateImage(cvSize(NSIZEX, NSIZEY),IPL_DEPTH_8U, 1);
	exp->SubSampled = SubSampled;
	/*** Create Frames **/
	Frame* fromCCD = CreateFrame(cvSize(NSIZEX, NSIZEY));

	exp->fromCCD = fromCCD;

	/** Create Neuron Data Struct and Neuron Parameter Struct **/
	NeuronAnalysisData* Neuron = CreateNeuronAnalysisDataStruct();
	NeuronAnalysisParam* Params = CreateNeuronAnalysisParam();
	InitializeEmptyNeuronImages(Neuron, cvSize(NSIZEX, NSIZEY));
	InitializeNeuronMemStorage(Neuron);
	exp->Neuron = Neuron;
	exp->Params = Params;

}

/*
 * Free up all of the different allocated memory for the
 * experiment.
 *
 */
void ReleaseExperiment(Experiment* exp) {
	/** Free up Frames **/
	if (exp->fromCCD != NULL)
		DestroyFrame(&(exp->fromCCD));

	/** Free up Neuron Objects **/
	if (exp->Neuron != NULL) {
		DestroyNeuronAnalysisDataStruct((exp->Neuron));
		exp->Neuron = NULL;
	}

	if (exp->Params != NULL) {
		DestroyNeuronAnalysisParam((exp->Params));
		exp->Params = NULL;
	}

	/** Free up internal iplImages **/
	if (exp->CurrentSelectedImg !=NULL)
		cvReleaseImage(&(exp->CurrentSelectedImg));

	if (exp->fromCCD !=NULL)
		DestroyFrame(&(exp->fromCCD));

	if (exp->SubSampled != NULL)
		cvReleaseImage(&(exp->SubSampled));
	/** Release Window Names **/
	//ReleaseWindowNames(exp);

}

/* Destroy the experiment object.
 * To be run after ReleaseExperiment()
 */
void DestroyExperiment(Experiment** exp) {
	free(*exp);
	*exp = NULL;
}

/*********************************************
 *
 * Image Acquisition
 *
 */

/** Grab a Frame from either camera or video source
 *
 */
int GrabFrame(Experiment* exp) {

	// Retrieve an image from the Grab Result of the camera before further step.
	int ret = 0;
	ret=T2Cam_RetrieveAnImage(*(exp->MyCamera));
	if (ret == EXP_ERROR)
		return EXP_ERROR;

	/** Acquire from Camera **/
	if (exp->MyCamera->iFrameNumber > exp->lastFrameSeenOutside)
		exp->lastFrameSeenOutside = exp->MyCamera->iFrameNumber;
	else
		return EXP_ERROR;
	/*** Create a local copy of the image***/
	LoadFrameWithBin(exp->MyCamera->iImageData, exp->fromCCD);

	exp->Neuron->frameNum++;
	return EXP_SUCCESS;
}

/*
 * Is a frame ready from the camera?
 *
 */
int isFrameReady(Experiment* exp) {

	return (exp->MyCamera->iFrameNumber > exp->lastFrameSeenOutside);

}

/*********************** RECORDING *******************/

/*
 * Sets up data recording and video recording
 * Will record video if exp->RECORDVID is 1
 * and record data if exp->RECORDDATA is 1
 *
 */
int SetupRecording(Experiment* exp) {

	printf("About to setup recording\n");
	char* DataFileName=NULL;
	
		/** Setup Writing and Write Out Comments **/
		exp->DataWriter = SetUpWriteToDisk(exp->dirname,exp->outfname, exp->Neuron->MemStorage);

		/** We should Quit Now if any of the data Writing is not working **/
		if (exp->DataWriter->error < 0 ) return -1;


		BeginToWriteOutFrames(exp->DataWriter);

		printf("Initialized data recording\n");
		DestroyFilename(&DataFileName);
	

	/** Set Up Video Recording **/
	char* MovieFileName;


	

		MovieFileName = CreateFileName(exp->dirname, exp->outfname, ".avi");


		exp->Vid = cvCreateVideoWriter(MovieFileName,
				CV_FOURCC('M','J','P','G'), 30, cvSize(NSIZEX / 2, NSIZEY / 2),
				0);

		if (exp->Vid ==NULL ) printf("\tERROR in SetupRecording! exp->Vid is NULL\nYou probably are missing the default codec.\n");
		
		DestroyFilename(&MovieFileName);
		printf("Initialized video recording\n");
	
	return 0;

}

/*
 * Finish writing video and  and data
 * and release
 *
 */
void FinishRecording(Experiment* exp) {
	/** Finish Writing Video to File and Release Writer **/
	if (exp->Vid != NULL)
		cvReleaseVideoWriter(&(exp->Vid));
	/** Finish Writing to Disk **/

		FinishWriteToDisk(&(exp->DataWriter));
}

/************************************************/
/*   Frame Rate Routines
 *
 */
/************************************************/

/*
 *This is the frame rate timer.
 */
void StartFrameRateTimer(Experiment* exp) {
	exp->prevTime = clock();
	exp->prevFrames = 0;

}

/*
 * If more than a second has elapsed
 * Calculate the frame rate and print i tout
 *
 */
void CalculateAndPrintFrameRate(Experiment* exp) {
	/*** Print out Frame Rate ***/
	if ((exp->Neuron->timestamp - exp->prevTime) > CLOCKS_PER_SEC) {
		printf("%d fps\n", exp->Neuron->frameNum - exp->prevFrames);
		exp->prevFrames = exp->Neuron->frameNum;
		exp->prevTime = exp->Neuron->timestamp;
	}
}

/************************************************/
/*   Action Chunks
 *
 */
/************************************************/

/*
 * Given an image in teh Neuron object, segment the Neuron
 *
 */
void DoSegmentation(Experiment* exp) {
	//_TICTOC_TIC_FUNC/
	/*** <segmentNeuron> ***/

	/*** Find Neuron Boundary ***/


	TICTOC::timer().tic("_FindNeuronCenter",exp->e);

	if (!(exp->e))

		exp->e = FindNeuronCenter(exp->Neuron, exp->Params);


	TICTOC::timer().toc("_FindNeuronCenter",exp->e);

//_TICTOC_TOC_FUNC
}

/*
 * Prepare the Selected Display
 *
 */
void PrepareSelectedDisplay(Experiment* exp) {
/** There are no errors and we are displaying a frame **/
switch (exp->Params->Display) {
case 0:
	//
	//exp->CurrentSelectedImg = exp->Neuron->ImgOrig;
	cvShowImage(exp->WinDisp,exp->Neuron->ImgOrig);


	break;
case 2:
	//exp->CurrentSelectedImg = exp->Neuron->ImgThresh;
	cvShowImage(exp->WinDisp,exp->Neuron->ImgThresh);
	break;
case 1:
	DisplayNeuronCenter(exp->Neuron, exp->WinDisp);
	break;

default:
	break;
}
//cvWaitKey(1); // Pause one millisecond for things to display onscreen.

}

/*
 *
 * Handle KeyStroke
 *
 * Returns 1 when the user is trying to exit
 *
 */
int HandleKeyStroke(int c, Experiment* exp) {
switch (c) {
case 27:
	printf("User has pressed escape!\n");
	printf("Setting Stage Tracking Variables to off");
	exp->Params->stageTrackingOn = 0;
	exp->stageIsTurningOff = 1;
	return 1;
	break;
	/** Threshold **/
case ']':
	Increment(&(exp->Params->BinThresh), 200);
	break;
case '[':
	Decrement(&(exp->Params->BinThresh), 0);
	break;
case 'r': /** record **/
	Toggle(&(exp->Params->Record));
	if (exp->Params->Record == 0) {
		printf("Record off!\n");
	}
	else {
		printf("Record on!\n");
	}
	break;
	/** Tracker **/
case '\t':
	Toggle(&(exp->Params->stageTrackingOn));
	if (exp->Params->stageTrackingOn == 0) {
		/** If we are turning the stage off, let the rest of the code know **/
		printf("Turning tracking off!\n");
		exp->stageIsTurningOff = 1;
	} else {
		printf("Turning tracking on!\n");
	}
	break;

case 'o':
	Toggle(&(exp->Params->OnOff));
	if (exp->Params->OnOff==0){
		printf("Turning Analysis off!\n");
	}else{
		printf("Turning Analysis on!\n");
	}
	break;

case '+':
	Increment(&(exp->Params->stageSpeedFactor), 50);
	printf("stageSpeedFactor=%d\n", exp->Params->stageSpeedFactor);
	break;
case '-':
	Decrement(&(exp->Params->stageSpeedFactor), 0);
	printf("stageSpeedFactor=%d\n", exp->Params->stageSpeedFactor);
	break;

case 127: /** Delete key **/
case 8: /** Backspace key **/
	exp->Params->stageTrackingOn = 0;
	exp->stageIsTurningOff = 1;
	printf("Instructing stage to turn off..");
	break;

default:
	return 0;
	break;
}
return 0;
}

/*
 * Write video and data to Disk
 *
 */
void DoWriteToDisk(Experiment* exp) {




	/** Record VideoFrame to Disk**/
	if (exp->Params->Record) {
		TICTOC::timer().tic("cvResize");
		cvResize(exp->Neuron->ImgOrig, exp->SubSampled, CV_INTER_LINEAR);
		TICTOC::timer().toc("cvResize");

		TICTOC::timer().tic("cvWriteFrame");
		cvWriteFrame(exp->Vid, exp->SubSampled);
		if (exp->Vid==NULL ) printf("\tERROR in DoWriteToDisk!\n\texp->Vid is NULL\n");


		TICTOC::timer().toc("cvWriteFrame");

	}
	/** Record data frame to diskl **/

	if (exp->Params->Record) {
		TICTOC::timer().tic("AppendNeuronFrameToDisk");
		AppendNeuronFrameToDisk(exp->Neuron, exp->Params, exp->DataWriter);
		TICTOC	::timer().toc("AppendNeuronFrameToDisk");
	
	}
}

/**************************************************
 * Stage Tracking and FEedback System
 *
 * This should really probably go in a special library called Stage Tracking
 * that depends on both OpenCV AND Talk2STage.c, but its a huge pain to modify the makefile
 * to create a new library that has only one function in it.
 *
 * Alternatively this could conceivably go in Talk2Stage.c, but then I find it weird
 * that Talk2Stage.c should depend on OpenCV, because ultimatley it should be more general.
 *
 * It doesn't really belong in experiment.c either because it is not a method of experiment.c
 * But for now that is where it will sit.
 *
 */

CvPoint AdjustStageToKeepObjectAtTarget(HANDLE stage, CvPoint *obj,
	CvPoint target, int speedfactor, int maxspeed) {


if (obj == NULL) {
	printf("Error! obj is NULL in AdjustStageToKeepObjectAtTarget()\n");
	return cvPoint(0, 0);
}

if (obj == NULL) {
	printf("Error! target is NULL in AdjustStageToKeepObjectAtTarget()\n");
	return cvPoint(0, 0);
}
//	printf("About to adjust stage.\n");

CvPoint diff;
CvPoint vel;

/** (stage-obj)*speed **/
printf("obj= (%d, %d), target =(%d, %d)\n",obj->x, obj->y, target.x, target.y);

diff.x = target.x - obj->x;
diff.y = target.y - obj->y;

//printf("About to Multiply!\n");

vel.x = (int)(maxspeed*100.0*tanh(0.001*diff.x*speedfactor));
vel.y = (int)(maxspeed*100.0*tanh(0.001*diff.y*speedfactor));

spinStage(stage, vel.x, vel.y);

printf("SpinStage: vel.x=%d, vel.y=%d\n",vel.x,vel.y);

return vel;

}


/*
 * tracking in dark
 */

CvPoint AdjustStageToKeepObjectAtTargetInDark(HANDLE stage, CvPoint* obj,
	CvPoint target, int speedfactor, int maxspeed, CvPoint prestageVelocity) {


	CvPoint diff;
	CvPoint vel;

	//printf("About to Multiply!\n");

	vel.x = prestageVelocity.x;
	vel.y = prestageVelocity.y;

	spinStage(stage, vel.x, vel.y);

	printf("SpinStage: vel.x=%d, vel.y=%d\n", vel.x, vel.y);

	return vel;

}







/*
 * Scan for the USB device.
 */
void InvokeStage(Experiment* exp) {
exp->stageCenter = cvPoint(NSIZEX / 2, NSIZEY / 2);

exp->stage = InitializeUsbStage();
if (exp->stage == NULL) {
	printf("ERROR! Invoking the stage failed.\nTurning tracking off.\n");
	exp->Params->stageTrackingOn = 0;

} else {
	printf("Telling stage to HALT.\n");
	haltStage(exp->stage);
}

}

void ShutOffStage(Experiment* exp) {
haltStage(exp->stage);
}

/*
 * Update the Stage Tracker.
 * If the Stage tracker is not initialized, don't do anything.
 * If the stage tracker is initialized then either do the tracking,
 * or if we are in the process of turning off tracking off, then tell
 * the stage to halt and update flags.
 */
int HandleStageTracker(Experiment* exp) {
if (exp->stageIsPresent == 1) { /** If the Stage is Present **/
	if (exp->stage == NULL)
		return 0;

	if (exp->Params->stageTrackingOn == 1) {
		if (exp->Params->OnOff == 0) { /** if the analysis system is off **/
			/** Turn the stage off **/
			exp->stageIsTurningOff = 1;
			exp->Params->stageTrackingOn = 0;
		} else {
			/** Move the stage to keep the Neuron centered in the field of view **/
			printf(".");
			//printf("stageFeedbackTargetoffset=(%d, %d)\n",exp->stageFeedbackTargetOffset.x,exp->stageFeedbackTargetOffset.y);
			CvPoint target = cvPoint(
					exp->stageCenter.x + exp->stageFeedbackTargetOffset.x,
					exp->stageCenter.y + exp->stageFeedbackTargetOffset.y);
			//printf("target=(%d, %d)\n",target.x,target.y);
			if (exp->Neuron->darktracking == 0;){
				exp->Neuron->stageVelocity = AdjustStageToKeepObjectAtTarget(
					exp->stage, exp->Neuron->centerOfNeuron, target,
					exp->Params->stageSpeedFactor,exp->Params->maxstagespeed);
				exp->Neuron->prestageVelocity = exp->Neuron->stageVelocity;
		    }else{
				exp->Neuron->stageVelocity = AdjustStageToKeepObjectAtTargetInDark(
					exp->stage, exp->Neuron->centerOfNeuron, target,
					exp->Params->stageSpeedFactor, exp->Params->maxstagespeed, exp->Neuron->prestageVelocity);
				exp->Neuron->prestageVelocity = exp->Neuron->stageVelocity;
			}
		}
	}
	if (exp->Params->stageTrackingOn == 0) {/** Tracking Should be off **/
		/** If we are in the process of turning tacking off **/
		if (exp->stageIsTurningOff == 1) {
			/** Tell the stage to Halt **/
			printf("Tracking Stopped!");
			printf("Telling stage to HALT.\n");
			haltStage(exp->stage);
			exp->stageIsTurningOff = 0;
		}
		/** The stage is already halted, so there is nothing to do. **/
	}

}
return 0;
}


/*
 * Update the Stage Tracker Position.
 * If the Stage tracker is not initialized, don't do anything.
*/
int RecordStageTracker(Experiment* exp) {


	findStagePosition(exp->stage, &(exp->Neuron->stagePosition.x), &(exp->Neuron->stagePosition.y));

	
	return 0;

}

