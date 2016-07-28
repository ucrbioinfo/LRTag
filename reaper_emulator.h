/*
 *  reaper_emulator.h
 *  MDS
 *
 *  Created by yonghui on 3/6/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef REAPER_EMULATOR_HEADER
#define REAPER_EMULATOR_HEADER

#include <set>
#include <map>
#include <string>
#include <cstring>
#include <utility>
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <queue>
#include "SCP.h"
#include "constants.h"
#include "thread_control_office.h"
#include "sgl_ppl_loci_correlation.h"

class multi_ppi_loci_correlation;

class Reaper_Emulator
{
	private:
		int number_of_loci;
		multi_ppi_loci_correlation* MDS_problem_instance;

	public:
		Reaper_Emulator(multi_ppi_loci_correlation* in_MDS_problem_instance);
		~Reaper_Emulator();
		int find_min_dominant_set(vector<int> & costs, vector<string> & solution,vector<int> permutation);
};

#endif
