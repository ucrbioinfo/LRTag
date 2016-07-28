/*
 *  LD_select_emulator.h
 *  MDS
 *
 *  Created by yonghui on 3/8/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LD_SELECT_EMULATOR_HEADER
#define LD_SELECT_EMULATOR_HEADER

#include <set>
#include <map>
#include <string>
#include <cstring>
#include <utility>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <queue>
#include "SCP.h"
#include "constants.h"
#include "thread_control_office.h"
#include "sgl_ppl_loci_correlation.h"

class LD_select
{
	private:
		/*An id to SNP mapping*/
		int total_number_of_loci;
		vector<string> string_loci;
		map<string, int> string_2_id_map;
		vector<set<int> > edges;
	public:
		LD_select(sgl_ppl_loci_correlation * input_population);
		~LD_select();
		int greedy_select(string output_file_path, vector<vector<string> > &bins);
		int dump();
};

#endif