/*
 *  multi_pop_select.h
 *  MDS
 *
 *  Created by yonghui on 3/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
 
#ifndef MULTI_POP_SELECT_HEADER
#define MULTI_POP_SELECT_HEADER

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

class Multi_Pop_Select
{
	private:
		set<string> all_tag_SNPs;
		map<string, int> SNP_2_ids;
		vector<string> id_2_SNPs;
		int number_of_distinct_tag_SNPs;
		
		vector<set<int> > *bins; //corresponding to elements of a SCP problem, or rows in our set cover problem
		vector<set<int> > *tag_SNPs; //corresponding to subsets of a SCP problem, or columns in our SCP
		
		
	public:
		Multi_Pop_Select(vector<vector<string> > &bin_SNPs);
		~Multi_Pop_Select();
		
		/*The function can be called only once
		When the function is called, the two objects bins and tag_SNPs will be destroyed.
		*/
		int find_min_covering_tag_SNPs(vector<string> & covering_tag_SNPs);
};


#endif