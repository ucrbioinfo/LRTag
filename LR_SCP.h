/*
 *  LR_SCP.h
 *  MDS
 *
 *  Created by yonghui on 2/27/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef LR_SCP_HEADER
#define LR_SCP_HEADER

#include <vector>
#include <utility>
#include <set>
#include <queue>
#include <iostream>
#include <cmath>
#include <fstream>
#include "SCP.h"
#include "constants.h"

using namespace std;

/*The following class is only used in the compute_max_independent_set function*/
bool compare_row_id_size_pair(pair<int,int> a, pair<int,int> b);



class LR_SCP_instance
{
	private:
	
		/*defines the set cover problem*/
		vector<set<int> >* rows;
		vector<set<int> >* columns;
		int num_of_columns;
		int num_of_rows;
	
		/*feasible primal and dual solution*/
		vector<int> max_independent_set;
		vector<int> primal_solution; 
		vector<double> dual_solution;
		
		/*
		dual_solution_type = 0, the solution from the initialization, which is obtained from the greedy approach
		dual_solution_type = 1, the solution from the regular LR form
		dual_solution_type = 2, the solution from the improved LR form
		*/
		int dual_solution_type; 
		
		
		double lowerbound;
		int upperbound;
		int compute_max_independent_set();
		int compute_initial_primal_dual();
	
		/*The function will be called in both subgradient_optimize and subgradient_optimize_version2*/
		void dual_feasy();
		void subgradient_optimize(int max_iterations, double starting_lambda, int interval_to_halve_lambda, double alpha_threshold, bool silent=true);
		void subgradient_optimize_version2(int max_iterations, double starting_lambda, int interval_to_halve_lambda, double alpha_threshold, bool silent=true);
		
		/*the following function will be called by the constructor by the end of the object construction*/
		void consistency_checking();

	public:

		LR_SCP_instance(vector<set<int> > * in_rows, vector<set<int> >* in_columns);

		/*Construct the set cover problem from a file*/
		LR_SCP_instance(string file_path);
		~LR_SCP_instance();
		
		void to_string();
		int find_set_cover_LR(int &lower_bound, int &cost, vector<int> &solution, LR_Types LR_type );



};

#endif
