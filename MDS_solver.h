/*
*  MDS_solver.h
*  MDS
*
*  Created by yonghui on 2/18/07.
*  Copyright 2007 __MyCompanyName__. All rights reserved.
*
*/
 
#ifndef MDS_solver_HEADER
#define MDS_solver_HEADER
 
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
#include <pthread.h>

class MDS_solver
{
		
	private:
		
		/*The following objects are all instantiated out of this class. But they will be destructed in this class*/
		multi_ppi_loci_correlation* MDS_problem_instance;
		thread_control_office* multi_threading_controller;
		vector<int> * cnntd_components; //an array of connected components
		vector<int> * MDS_solution;//the cover found
		
		/*An array of integers. 
		0: If the min set cover for the component has not been computed yet
		1: The computation of min set cover is in progress
		2: The min set cover has been found
		*/
		int* component_status;
		int number_of_cnntd_components;
		
		/*use which method to solve the Minimum set cover problem*/
		MSC_Method method;
		int maximum_number_of_threads;
	public:
		MSC_Method get_method();
		MDS_solver( multi_ppi_loci_correlation* in_MDS_problem_instance, MSC_Method in_method);
		~MDS_solver();
		
		int concurrent_mds_slvr(int &lowerbound, int &cost, vector<string> &min_common_dom_set);

		/*The working thread function*/
		static void* MDS_slvr_trd_func(void * in_mds_solver_ptr); 
		
};



#endif

