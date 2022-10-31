/*
 * NeuronAnalysis.cpp
 *
 *  Created on: Sep 5, 2013
 *      Author: quan
 */
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <time.h>


//OpenCV Headers
#include <opencv\cxcore.h>
#include <opencv\highgui.h>
#include <opencv\cv.h>

//Timer Lib
#include "../3rdPartyLibs/tictoc.h"


#include "AndysOpenCVLib.h"
#include "AndysComputations.h"

// Andy's Libraries
#include "NeuronAnalysis.h"




/*
 *
 * Every function here should have the word Neuron in it
 * because every function here is Neuron specific
 */



/************************************************************/
/* Creating, Destroying and Memory for 						*/
/*  NeuronAnalysisDataStruct 									*/
/*															*/
/************************************************************/



/*
 *  Create the NeuronAnalysisDataStruct
 *  Initialize Memory Storage
 *  Set all Pointers to Null.
 *  Run CvCreate Sequence
 *
 *  Note this does not allocate memory for images because the user may not know
 *  what size image is wanted yet.
 *
 *  To do that use LoadNeuronColorOriginal()
 *
 */





NeuronAnalysisData* CreateNeuronAnalysisDataStruct(){
	NeuronAnalysisData* NeuronPtr;
	NeuronPtr=(NeuronAnalysisData*) malloc(sizeof(NeuronAnalysisData));


	/*** Set Everything To NULL ***/
	NeuronPtr->ImgOrig =NULL;
	NeuronPtr->ImgThresh =NULL;
	NeuronPtr->Mask = NULL;

	NeuronPtr->frameNum=0;
	NeuronPtr->frameNumCamInternal=0;


	NeuronPtr->SizeOfImage.height = 0;
	NeuronPtr->SizeOfImage.width= 0;

	NeuronPtr->timestamp=0;
	NeuronPtr->MemScratchStorage=0;
	NeuronPtr->MemStorage=0;

	/** Position of the neuron on the image **/
	NeuronPtr->centerOfNeuron = NULL;
	NeuronPtr->mom=NULL;
	NeuronPtr->stagePosition = cvPoint(0, 0);
	
	/** tracking in dark **/
	NeuronPtr->darktracking = 0;




	/** Position on plate information **/
	NeuronPtr->stageVelocity=cvPoint(0,0);
	NeuronPtr->prestageVelocity = cvPoint(0, 0);
	return NeuronPtr;
}




/*
 *
 * Clears all the Memory and De-Allocates it
 */
void DestroyNeuronAnalysisDataStruct(NeuronAnalysisData* Neuron){
	if (Neuron->ImgOrig !=NULL)	cvReleaseImage(&(Neuron->ImgOrig));
	if (Neuron->ImgThresh !=NULL) cvReleaseImage(&(Neuron->ImgThresh));
	if (Neuron->Mask !=NULL) cvReleaseImage(&(Neuron->Mask));
	if (Neuron->mom !=NULL) free(Neuron->mom);
	
	cvReleaseMemStorage(&((Neuron)->MemScratchStorage));
	cvReleaseMemStorage(&((Neuron)->MemStorage));
	
	free(Neuron);
	Neuron=NULL;
}

/*
 * Create dynamic memory storage for the worm
 *
 */
void InitializeNeuronMemStorage(NeuronAnalysisData* Neuron){
	Neuron->MemScratchStorage=cvCreateMemStorage(0);
	Neuron->MemStorage=cvCreateMemStorage(0);
}

/*
 * Refersh dynamic memory storage for the worm
 * (clear the memory without freing it)
 *
 */
int RefreshNeuronMemStorage(NeuronAnalysisData* Neuron){
	if (Neuron->MemScratchStorage!=NULL){
		cvClearMemStorage(Neuron->MemScratchStorage);
	}else{
		printf("Error! MemScratchStorage is NULL in RefreshWormMemStorage()!\n");
		return -1;
	}
	if (Neuron->MemStorage!=NULL){
		cvClearMemStorage(Neuron->MemStorage);
	} else{
		printf("Error! MemStorage is NULL in RefreshWormMemStorage()!\n");
		return -1;
	}
	return 0;
}

/*
 * Create Blank Images for NeuronAnalysisData
 *
 */

void InitializeEmptyNeuronImages(NeuronAnalysisData* Neuron, CvSize ImageSize){
	Neuron->SizeOfImage=ImageSize;
	Neuron->ImgOrig= cvCreateImage(ImageSize,IPL_DEPTH_8U,1);
	Neuron->ImgThresh=cvCreateImage(ImageSize,IPL_DEPTH_8U,1);
	Neuron->Mask=cvCreateImage(ImageSize,IPL_DEPTH_8U,1);

	/** Clear the Time Stamp **/
	Neuron->timestamp=0;

}




/*
 * This function is run after IntializeEmptyImages.
 * And it loads a color original into the NeuronAnalysisData strucutre.
 * The color image is converted to an 8 bit grayscale image.
 *
 * It also sets the time stamp.
 */
void LoadNeuronColorOriginal(NeuronAnalysisData* Neuron, IplImage* ImgColorOrig){
	CvSize CurrentSize = cvGetSize(ImgColorOrig);
	if ( (Neuron->SizeOfImage.height != CurrentSize.height) || (Neuron->SizeOfImage.width != CurrentSize.width) ){
		printf("Error. Image size does not match in LoadNeuronColorOriginal()");
		return;
	}
	cvCvtColor( ImgColorOrig, Neuron->ImgOrig, CV_BGR2GRAY);

	/** Set the TimeStamp **/
	Neuron->timestamp=clock();

}

/*
 * This function is run after IntializeEmptyImages.
 * And it loads a properly formated 8 bit grayscale image
 * into the NeuronAnalysisData strucutre.
 *
 * It also sets the timestamp.
 */
int LoadNeuronImg(NeuronAnalysisData* Neuron, NeuronAnalysisParam* Params, IplImage* Img, CvPoint stageFeedbackTargetOffset){
	CvSize CurrentSize = cvGetSize(Img);
	if ( (Neuron->SizeOfImage.height != CurrentSize.height) || (Neuron->SizeOfImage.width != CurrentSize.width) ){
		printf("Error. Image size does not match in  LoadNeuronImg()");
		return -1;
	}
	/** Set the TimeStamp **/
	Neuron->timestamp=clock();
	
	CvPoint maskCenter; // Center of an mask
	
	maskCenter=cvPoint(CurrentSize.width /2+stageFeedbackTargetOffset.x, CurrentSize.height /2+stageFeedbackTargetOffset.y);
	
	cvZero(Neuron->Mask);
	cvCircle(Neuron->Mask, maskCenter,(Params->MaskDiameter)/2, cvScalar(255),-1,CV_AA,0);

	/** Copy the Image **/
	
	IplImage* TempImage=cvCreateImage(CurrentSize,IPL_DEPTH_8U,1);
	cvZero(TempImage);
	cvCopy( Img, TempImage,Neuron->Mask);
	cvCopy( TempImage,Neuron->ImgOrig, NULL);
	cvReleaseImage(&TempImage);

	return 0;

}

/************************************************************/
/* Creating, Destroying NeuronAnalysisParam					*/
/*  					 									*/
/*															*/
/************************************************************/

/*
 *  Allocate memory for a NeuronAnalysisParam struct
 *  And set default values for the parameters.
 */
NeuronAnalysisParam* CreateNeuronAnalysisParam(){
	NeuronAnalysisParam* ParamPtr;
	ParamPtr=(NeuronAnalysisParam*) malloc(sizeof(NeuronAnalysisParam));

	/** Turn the System On or Off **/
	ParamPtr->OnOff=0;
	ParamPtr->Record = 0;
	/** Single Frame Analysis Parameters **/
	ParamPtr->BinThresh=48;
	
	/** Mask Diameter **/
	ParamPtr->MaskDiameter=480;

	/** DIsplay Parameters **/
	ParamPtr->DispRate=1;
	ParamPtr->Display=0;

	/** Stage Control Parameters **/
	ParamPtr->stageTrackingOn=0;
	ParamPtr->stageSpeedFactor=10;
	ParamPtr->maxstagespeed=45;

	return ParamPtr;
}


void DestroyNeuronAnalysisParam(NeuronAnalysisParam* ParamPtr){
	free(ParamPtr);
}


/************************************************************/
/* Higher Level Routines									*/
/*  					 									*/
/*															*/
/************************************************************/



/*
 * thresholds and finds the Neurons center.
 * The original image must already be loaded into Neuron.ImgOrig

 * The thresholded image is deposited into Neuron.ImgThresh
 *
 */
int FindNeuronCenter(NeuronAnalysisData* Neuron, NeuronAnalysisParam* Params){


	TICTOC::timer().tic("cvThreshold");

	cvThreshold(Neuron->ImgOrig,Neuron->ImgThresh,Params->BinThresh,255,CV_THRESH_BINARY);

	CvScalar pixelsum;
	pixelsum=cvSum(Neuron->ImgThresh);
	TICTOC::timer().toc("cvThreshold");


	if (pixelsum.val[0]==0){
		Neuron->darktracking = 1;
		//printf("Fail to track the neuron. \n");
		//printf("Fail to track the neuron. \n");
		//return 0;

	}
	else{
		IplImage* TempImage;
		TempImage=cvCreateImage(cvGetSize(Neuron->ImgThresh),IPL_DEPTH_8U,1);
		cvCopy(Neuron->ImgThresh,TempImage);
		TICTOC::timer().tic("cvMoments");
		if (Neuron->mom==NULL) Neuron->mom=(CvMoments*) malloc(sizeof(CvMoments));
		cvMoments(TempImage,Neuron->mom,1);
		TICTOC::timer().toc("cvMoments");
		if (Neuron->centerOfNeuron==NULL) Neuron->centerOfNeuron=(CvPoint*) malloc(sizeof(CvPoint));
		*(Neuron->centerOfNeuron)=cvPoint(Neuron->mom->m10/Neuron->mom->m00,Neuron->mom->m01/Neuron->mom->m00);
		cvReleaseImage(&TempImage);
		Neuron->darktracking = 0;
		return 0;
	}

}



/*
 * Generates the original image together with the center of the neuron
 */
void DisplayNeuronCenter(NeuronAnalysisData* Neuron, const char* WinDisp){

	IplImage* TempImage=cvCreateImage(cvGetSize(Neuron->ImgOrig),IPL_DEPTH_8U,1);
	cvCopyImage(Neuron->ImgOrig,TempImage);
	int CircleDiameterSize=10;
	cvCircle(TempImage,*(Neuron->centerOfNeuron),CircleDiameterSize,cvScalar(255,255,255),1,CV_AA,0);
	cvShowImage(WinDisp,TempImage);
	cvReleaseImage(&TempImage);


}








