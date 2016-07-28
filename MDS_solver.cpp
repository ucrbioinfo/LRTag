/*
 *  MDS_solver.cpp
 *  MDS
 *
 *  Created by yonghui on 2/18/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "MDS_solver.h"

MDS_solver::MDS_solver(multi_ppi_loci_correlation* in_MDS_problem_instance, MSC_Method in_method)
{
	this->method = in_method;
	MDS_problem_instance = in_MDS_problem_instance;
	maximum_number_of_threads = NUMBER_OF_CONCURRENT_THREADS;
	
	/*Compute the connected components of MDS_problem_instance*/
	cnntd_components = NULL;
	number_of_cnntd_components = 0;
	MDS_problem_instance->connected_components(&cnntd_components, number_of_cnntd_components);

	/*Construct the multi_threading_controller object*/
	multi_threading_controller = new thread_control_office(number_of_cnntd_components);
	component_status = new int[number_of_cnntd_components];
	MDS_solution = new vector<int>[number_of_cnntd_components];
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MSC_Method MDS_solver::get_method()
{
	return method;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


MDS_solver::~MDS_solver()
{
	if (MDS_problem_instance != NULL)
	{
		delete MDS_problem_instance;
		MDS_problem_instance = NULL;
	}
	if (multi_threading_controller != NULL)
	{
		delete multi_threading_controller;
		multi_threading_controller = NULL;
	}
	if (cnntd_components != NULL)
	{
		delete[] cnntd_components;
		cnntd_components = NULL;
	}
	if (MDS_solution != NULL)
	{
		delete[] MDS_solution;
		MDS_solution = NULL;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* MDS_solver::MDS_slvr_trd_func(void * in_mds_solver_ptr)
{
	MDS_solver * mds_solver_ptr = (MDS_solver *) in_mds_solver_ptr;
	/*Do until Min Set Cover has been found for all the components*/
	int counter = 0;
	while (1)
	{
		/*Get the id of the next connected component to be solved*/
		int next_component_id;
		next_component_id = (mds_solver_ptr->multi_threading_controller)->next_cnntd_component_id();
		if (next_component_id < 0)
		{
			/*There is no more component to be solved*/
			char to_string[100]; 
			sprintf(to_string,"Thread terminating, called on  %d components \n", counter);
			(mds_solver_ptr->multi_threading_controller)->controlled_print(to_string);
			return 0;
		}
		else
		{
			counter = counter + 1;
			/*computation of the connected components in progress*/
			(mds_solver_ptr->component_status)[next_component_id] = 1;
			SCP_instance* crrt_scp_prblm = NULL;
			(mds_solver_ptr->MDS_problem_instance)->construct_SCP_instance( &crrt_scp_prblm , (mds_solver_ptr->cnntd_components)[next_component_id] );
			int cost = 0;
			int lowerbound = 0;
			vector<int> scp_solution;
			
			/*Should be flexible to choose which function to call to solve the set cover problem*/
			switch (mds_solver_ptr->get_method())
			{
				case GREEDY_WITH_PREPROCESSING:
					crrt_scp_prblm->find_set_cover_greedy_with_preprocessing(lowerbound,cost,scp_solution);
					break;
					
				case GREEDY_NO_PREPROCESSING:
					crrt_scp_prblm->find_set_cover_greedy_no_preprocessing(lowerbound,cost,scp_solution);
					break;
				case LR1_WITH_PREPROCESSING:
					crrt_scp_prblm->find_set_cover_best_of_LR_and_greedy_with_preprocessing(lowerbound,cost,scp_solution,LR1);
					break;
				case LR2_WITH_PREPROCESSING:
					crrt_scp_prblm->find_set_cover_best_of_LR_and_greedy_with_preprocessing(lowerbound,cost,scp_solution,LR2);
					break;
			}
			
			(mds_solver_ptr->multi_threading_controller)->add_to_cost(cost);
			(mds_solver_ptr->multi_threading_controller)->add_to_lowerbound(lowerbound);
			((mds_solver_ptr->MDS_solution)[next_component_id]).clear();
			for (int ll = 0 ; ll < scp_solution.size(); ll++)
			{
				((mds_solver_ptr->MDS_solution)[next_component_id]).push_back((mds_solver_ptr->cnntd_components)[next_component_id][scp_solution[ll]]);
			}
			
			/*Set the status of the component to 2*/
			(mds_solver_ptr->component_status)[next_component_id] = 2;
			(mds_solver_ptr->multi_threading_controller)->solved_one_more_component();
			
			if (crrt_scp_prblm != NULL)
			{
				delete crrt_scp_prblm;
				crrt_scp_prblm = NULL;
			}
		}
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int MDS_solver::concurrent_mds_slvr(int &lowerbound, int &cost, vector<string> &min_common_dom_set)
{
	pthread_t threads[maximum_number_of_threads];
	int ret_values[maximum_number_of_threads];
	for (int ii = 0 ; ii < maximum_number_of_threads; ii++)
	{
		ret_values[ii] = pthread_create(&(threads[ii]),NULL,MDS_slvr_trd_func, (void *)this);		
	}
	for (int ii = 0 ; ii < maximum_number_of_threads; ii++)
	{
		pthread_join(threads[ii], NULL);
		char to_print_buffer[100];
		sprintf(to_print_buffer, "thread %d has terminated \n", ii);
		multi_threading_controller->controlled_print(to_print_buffer);
	}
	
	/*Set up the output objects*/
	lowerbound = multi_threading_controller->get_lowerbound();
	cost = multi_threading_controller->get_cost();
	min_common_dom_set.clear();
	for (int ii = 0 ; ii < number_of_cnntd_components; ii++)
	{
		for (int jj = 0; jj < (MDS_solution[ii]).size(); jj++)
		{
			min_common_dom_set.push_back(MDS_problem_instance->int_id_2_SNP_lable((MDS_solution[ii])[jj]));
		}
	}
	
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

