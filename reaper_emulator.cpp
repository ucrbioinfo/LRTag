/*
 *  reaper_emulator.cpp
 *  MDS
 *
 *  Created by yonghui on 3/6/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "reaper_emulator.h"

Reaper_Emulator::Reaper_Emulator(multi_ppi_loci_correlation* in_MDS_problem_instance)
{
	MDS_problem_instance = in_MDS_problem_instance;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Reaper_Emulator::~Reaper_Emulator()
{
	if (MDS_problem_instance != NULL)
	{
		delete MDS_problem_instance;
		MDS_problem_instance = NULL;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Reaper_Emulator::find_min_dominant_set(vector<int> & costs, vector<string>  &solution,vector<int> permutation)
{
	/*initialize the pass-in parameters*/
	costs.clear();
	solution.clear();
	
	graph_linked_list* constituent_graphs = MDS_problem_instance->constituent_graphs;
	int number_of_ppls = MDS_problem_instance->number_of_ppls;
	int total_number_of_loci = MDS_problem_instance -> total_number_of_loci;
	map<int,string> &id_2_SNP(MDS_problem_instance -> id_2_SNP);
	
	set<int> current_solution;
	for (int ii = 0 ; ii < number_of_ppls; ii++)
	{
		//very important! use the reference instead the object itself
		graph_linked_list &current_graph(constituent_graphs[permutation[ii]]); 
		
		bool* current_vertices = current_graph.vertices;
		int own_SNPs = current_graph.own_SNPs;
		
		/*construct a mapping of ids from the entire space to the ii^th population only*/
		/*construct a mapping of ids from the ii^th space to the entire space*/

		vector<int> entire_space_2_ii_space(total_number_of_loci,-1);
		vector<int> ii_space_2_entire_space;
		int counter = 0;
		for (int jj = 0 ; jj < total_number_of_loci ; jj ++)
		{
			if (current_vertices[jj])
			{
				ii_space_2_entire_space.push_back(jj);
				entire_space_2_ii_space[jj] = counter;
				counter = counter + 1;
			}
		}
		if (own_SNPs != counter)
		{
			cout << "ERROR! Reaper_Emulator::find_min_dominant_set, constructing the mappings failed" << endl;
		}
		
		/*construct a set cover problem for the ii^th population*/
		vector<set<int> > * tmp_rows = new vector<set<int> >(own_SNPs, set<int>());
		vector<set<int> > * tmp_columns = new vector<set<int> >(own_SNPs, set<int>());
		for (int cc = 0 ; cc < own_SNPs; cc++)
		{
			((*tmp_columns)[cc]).insert(cc);
			((*tmp_rows)[cc]).insert(cc);
			vector<int> &current_linked_list((current_graph.edges)[ii_space_2_entire_space[cc]]);
			for (vector<int>::iterator iterator1 = current_linked_list.begin(); iterator1 != current_linked_list.end(); iterator1 ++)
			{
				int rr = entire_space_2_ii_space[*iterator1];
				((*tmp_rows)[rr]).insert(cc);
				((*tmp_columns)[cc]).insert(rr);
			}
		}
		
		SCP_instance * current_scp_problem = new SCP_instance(tmp_rows, tmp_columns);
		tmp_rows = NULL;
		tmp_columns = NULL; //current_scp_problem will take care of the two objects from now on
		
		/*construct the list of ids to be preselected*/
		vector<int> preselected_id;
		for (set<int>::iterator iterator1 = current_solution.begin(); iterator1 != current_solution.end(); iterator1 ++)
		{
			int column_id = entire_space_2_ii_space[*iterator1];
			if (column_id >= 0 )
			{
				preselected_id.push_back(column_id);
			};
		}

		/*obtain the solution*/
		current_scp_problem->pre_select(preselected_id);
		int lower_bound;
		int cost;
		vector<int> ii_solution;
		current_scp_problem->find_set_cover_greedy_no_preprocessing(lower_bound, cost, ii_solution);
		/*map the solution back to the entire space*/
		
		vector<int> ii_solution_entire_space;
		for (vector<int>::iterator iterator1 = ii_solution.begin(); iterator1 != ii_solution.end(); iterator1 ++)
		{
			ii_solution_entire_space.push_back(ii_space_2_entire_space[*iterator1]);
		}
		
		/*update the costs vector*/
		current_solution.insert(ii_solution_entire_space.begin(), ii_solution_entire_space.end());
		costs.push_back(current_solution.size()); 
		
		/*destruct the current_scp_problem after use*/
		if (current_scp_problem != NULL)
		{
			delete current_scp_problem;
			current_scp_problem = NULL;
		}

	}
	
	for (set<int>::iterator iterator1 = current_solution.begin(); iterator1 != current_solution.end(); iterator1 ++)
	{
		solution.push_back(id_2_SNP[*iterator1]);
	}
	return 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
