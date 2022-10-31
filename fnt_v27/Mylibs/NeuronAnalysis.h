/*
 * Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
 * This file is part of MindControl.
 *
 * MindControl is free software: you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MindControl is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MindControl. If not, see <http://www.gnu.org/licenses/>.
 *
 * For the most up to date version of this software, see:
 * http://github.com/samuellab/mindcontrol
 *
 *
 *
 * NOTE: If you use any portion of this code in your research, kindly cite:
 * Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
 * 	"Optogenetic manipulation of neural activity with high spatial resolution in
 *	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
 */


/*
 * NeuronAnalysis.h
 *
 *  Created on: Oct 12, 2009
 *      Author: andy
 *
 *      This library contains functions that are specific to analyzing Neurons.
 *
 *      Functions in this library depend on:
 *      	AndysOpenCVLib.h
 *      	AndysComputations.h
 */

#ifndef NEURONANALYSIS_H_
#define NEURONANALYSIS_H_

//#ifndef ANDYSOPENCVLIB_H_
// #error "#include AndysOpenCVLib.h" must appear in source files before "#include NeuronAnalysis.h"
//#endif


typedef struct NeuronAnalysisParamStruct{
	/** Turn Analysis On Generally **/
	int OnOff;
	int Record;


	/** Single Frame Analysis Parameters**/
	int BinThresh;
	
	/** Mask Diameter for Neuron Tracking **/
	int MaskDiameter;

	/** Display Stuff**/
	int DispRate; //Deprecated
	int Display;

	/** Stage Control Parameters **/
	int stageTrackingOn;
	int stageSpeedFactor;
	int maxstagespeed;



} NeuronAnalysisParam;


typedef struct NeuronImageAnalysisStruct{
	CvSize SizeOfImage;

	/** Frame Info **/
	int frameNum;
	int frameNumCamInternal;

	/** Images **/
	IplImage* ImgOrig;
	IplImage* ImgThresh;
	
	/** mask for ROI selection **/
	IplImage* Mask;

	/** Memory **/
	CvMemStorage* MemStorage;
	CvMemStorage* MemScratchStorage;

	/** Neuron Features **/
	CvPoint* centerOfNeuron;

	CvMoments* mom;


	/** TimeStamp **/
	unsigned long timestamp;


	/** tracking in dark **/
	int darktracking;
	/** Information about location on plate **/
	CvPoint stageVelocity; //compensating velocity of stage.
	CvPoint prestageVelocity;
	CvPoint stagePosition; //Position of the motorized stage.


}NeuronAnalysisData;



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
NeuronAnalysisData* CreateNeuronAnalysisDataStruct();




/*
 *
 * Clear's all the Memory and De-Allocates it
 */
void DestroyNeuronAnalysisDataStruct(NeuronAnalysisData* NeuronPtr);


/*
 * Create Blank Images for NeuronAnalysisData given the image size.
 *
 */
 
 
 /*
 * Create dynamic memory storage for the worm
 *
 */
void InitializeNeuronMemStorage(NeuronAnalysisData* Neuron);

/*
 * Refersh dynamic memory storage for the worm
 * (clear the memory without freing it)
 *
 */
int RefreshNeuronMemStorage(NeuronAnalysisData* Neuron);


void InitializeEmptyNeuronImages(NeuronAnalysisData* Neuron, CvSize ImageSize);

/*
 * This function is run after IntializeEmptyImages.
 * And it loads a color original into the WoirmAnalysisData structure.
 * The color image is converted to an 8 bit grayscale.
 *
 *
 */
void LoadNeuronColorOriginal(NeuronAnalysisData* Neuron, IplImage* ImgColorOrig);


/*
 * This function is run after IntializeEmptyImages.
 * And it loads a properly formated 8 bit grayscale image
 * into the NeuronAnalysisData structure.
 */
int LoadNeuronImg(NeuronAnalysisData* Neuron, NeuronAnalysisParam* Params, IplImage* Img, CvPoint stageFeedbackTargetOffset);



/************************************************************/
/* Creating, Destroying NeuronAnalysisParam					*/
/*  					 									*/
/*															*/
/************************************************************/

/*
 *  Allocate memory for a NeuronAnalysisParam struct
 */
NeuronAnalysisParam* CreateNeuronAnalysisParam();

void DestroyNeuronAnalysisParam(NeuronAnalysisParam* ParamPtr);




/************************************************************/
/* Higher Level Routines									*/
/*  					 									*/
/*															*/
/************************************************************/


/*
 * thresholds and finds the center of the neuron.
 * The original image must already be loaded into Neuron.ImgOrig
 * The thresholded image is deposited into Neuron.ImgThresh
 *
 */
int FindNeuronCenter(NeuronAnalysisData* Neuron, NeuronAnalysisParam* NeuronParams);


/*
 * Generates the original image and the center of the neuron
 *
 */
void DisplayNeuronCenter(NeuronAnalysisData* Neuron,const char* WinDisp);




#endif /* NeuronANALYSIS_H_ */
