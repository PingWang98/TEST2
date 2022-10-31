/*
 * experiment.h
 *
 *  Created on: Sep 3, 2013
 *      Author: quan
 */

#ifndef EXPERIMENT_H_
#define EXPERIMENT_H_




//#ifndef NEURONANALYSIS_H_
// #error "#include NeuronAnalysis.h" must appear in source files before "#include experiment.h"
//#endif

//#ifndef TALK2CAMERA_H_
// #error "#include Talk2Camera.h" must appear in source files before "#include experiment.h"
//#endif

//#ifndef TALK2STAGE_H_
// #error "#include Talk2Stage.h" must appear in source files before "#include experiment.h"
//#endif

#define EXP_ERROR -1
#define EXP_SUCCESS 0
#define NSIZEX 586
#define NSIZEY 512

typedef struct ExperimentStruct{

	/** GuiWindowNames **/
	const char* WinDisp;
	const char* WinCon1;

	/** CommandLine Input **/
	char** argv;
	int argc;
	char* dirname;
	char* outfname;
	char* infname;

	/** Camera Input **/
	CamData* MyCamera;

	/** MostRecently Observed CameraFrameNumber **/
	uint64_t lastFrameSeenOutside;

	/** User-configurable Neuron-related Parameters **/
	NeuronAnalysisParam* Params;

	/** Information about Our Neuron **/
	NeuronAnalysisData* Neuron;

	/** internal IplImage **/
	IplImage* CurrentSelectedImg;
	IplImage* SubSampled; // Image used to subsample stuff
	IplImage* HUDS;  //Image used to generate the Heads Up Display
	/** Internal Frame data types **/
	Frame* fromCCD;

	/** Write Data To File **/
	WriteOut* DataWriter;

	/** Write Video To File **/
	CvVideoWriter* Vid;  //Video Writer

	/** Timing  Information **/
	clock_t now;
	clock_t last;


	/** Frame Rate Information **/
	int nframes;
	int prevFrames;
	long prevTime;

	/** Stage Control **/
	int stageIsPresent;
	HANDLE stage; // Handle to USB stage object
	CvPoint stageVel; //Current velocity of stage
	CvPoint stageCenter; // Point indicating center of stage.
	CvPoint stageFeedbackTargetOffset; //Target of the stage feedback loop as a delta distance in pixels from the center of the image
	int stageIsTurningOff; //1 indicates stage is turning off. 0 indicates stage is on or off.

	/** Error Handling **/
	int e;

} Experiment;

/*
 * Creates a new experiment object and sets values to zero.
 */
Experiment* CreateExperimentStruct();


/*
 * Load the command line arguments into the experiment object
 */
void LoadCommandLineArguments(Experiment* exp, int argc, char** argv);

void displayhelp();

/*
 * Handle CommandLine Arguments
 * Parses commandline arguments.
 * Decides if user wants to record video or recorddata
 */
int HandleCommandLineArguments(Experiment* exp);



/* Assigns Default window names to the experiment object
 *
 */
void AssignWindowNames(Experiment* exp);

/*
 * Release the memopry associated with window names
 * and set their pointers to Null
 */
void ReleaseWindowNames(Experiment* exp);


/*
 * SetupGui
 *
 */
void SetupGUI(Experiment* exp);

/*
 * Update's trackbar positions for variables that can be changed by the software
 *
 */
void UpdateGUI(Experiment* exp);


/*
 * Initialize camera library
 * Allocate Camera Data
 * Select Camera and Show Properties dialog box
 * Start Grabbing Frames as quickly as possible
 *
 * OR open up the video file for reading.
 */

void RollCameraInput(Experiment* exp);

/** Grab a Frame from either camera or video source
 *
 */
int GrabFrame(Experiment* exp);



/*
 * This function allocates images and frames
 * And a Neuron Object
 *
 * And a Parameter Object
 * For internal manipulation
 *
 *
 */
void InitializeExperiment(Experiment* exp);


/*
 * Free up all of the different allocated memory for the
 * experiment.
 *
 */
void ReleaseExperiment(Experiment* exp);

void DestroyExperiment(Experiment** exp);

/*
 * Setsup data recording and video recording
 * Will record video if exp->RECORDVID is 1
 * and record data if exp->RECORDDATA is 1
 *
 */
int SetupRecording(Experiment* exp);


/*
 * Finish writing video and  and data
 * and release
 *
 */
void FinishRecording(Experiment* exp);



/*********************************************
 *
 * Image Acquisition
 *
 */

/*
 * Is a frame ready from the camera?
 */
int isFrameReady(Experiment* exp);


/************************************************/
/*   Frame Rate Routines
 *
 */
/************************************************/

/*
 *This is the frame rate timer.
 */
void StartFrameRateTimer(Experiment* exp);

/*
 * If more than a second has elapsed
 * Calculate the frame rate and print it out
 *
 */
void CalculateAndPrintFrameRate(Experiment* exp);

/************************************************/
/*   Action Chunks
 *
 */
/************************************************/

/*
 * Given an image in teh Neuron object, segment the Neuron
 *
 */

void DoSegmentation(Experiment* exp);

void PrepareSelectedDisplay(Experiment* exp);


/*
 *
 * Handle KeyStroke
 *
 * Returns 1 when the user is trying to exit
 *
 */
int HandleKeyStroke(int c, Experiment* exp);



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

/*
 * Scan for the USB device.
 */

 /*
  * Write video and data to Disk
  *
  */
void DoWriteToDisk(Experiment* exp);

/*
 * Writes a recent frame number to file
 */
int WriteRecentFrameNumberToFile(Experiment* exp);

void InvokeStage(Experiment* exp);

/*
 * Update the Stage Tracker.
 * If the Stage tracker is not initialized, don't do anything.
 * If the stage tracker is initialized then either do the tracking,
 * or if we are in the process of turning off tracking off, then tell
 * the stage to halt and update flags.
 */
int HandleStageTracker(Experiment* exp);

void ShutOffStage(Experiment* exp);

int RecordStageTracker(Experiment* exp);

#endif /* EXPERIMENT_H_ */



