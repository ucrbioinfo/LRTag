/*
 *  constants.h
 *  MDS
 *
 *  Created by yonghui on 2/18/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef CONSTANTS_HEADER
#define CONSTANTS_HEADER

const int NUMBER_OF_CONCURRENT_THREADS=15;

/*constants related with subgradient optiization*/
const int MAX_NUMBER_OF_ITERATIONS = 1000;
const double STARTING_LAMBDA = 2;
const int INTERVAL_TO_HALVE_LAMBDA = 20;
const double ALPHA_THRESHOLD= 0.0001;

enum LR_Types {LR1, LR2};
enum MSC_Method{GREEDY_WITH_PREPROCESSING, GREEDY_NO_PREPROCESSING, LR1_WITH_PREPROCESSING, LR2_WITH_PREPROCESSING};


#endif
