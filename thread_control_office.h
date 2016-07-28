/*
 *  thread_control_office.h
 *  MDS
 *
 *  Created by yonghui on 2/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef thread_control_office_HEADER
#define thread_control_office_HEADER

#include <vector>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include "sgl_ppl_loci_correlation.h"


using namespace std;

class thread_control_office 
{

	private:
		/*A constant. The total number of connected components*/
		int number_of_cnntd_components;
		
		/*Start of global resource*/
		int next_component_to_solve; //numbered from 0
		int cnntd_cmpnts_solved; //number of already solved components
		int lowerbound;
		int cost;
		/*End of the global resource*/
		
		/*All access to global resource should first obtain the following mutex*/
		pthread_mutex_t mutex1; //control the access to next_component_to_solve
		pthread_mutex_t mutex2; //control the access to cnntd_cmpnts_solved
		pthread_mutex_t mutex3; //control the access to cout
		pthread_mutex_t mutex4; //control the access to lowerbound 
		pthread_mutex_t mutex5; //control the access to cost 


		
	public:
		/*The constructor*/
		thread_control_office(int in_number_of_cnntd_components);
		~thread_control_office();
		
		/*If there is no more component to solve, return -1, otherwise, return the id of the next component*/
		int next_cnntd_component_id();
		
		/*Return 0 on success. Otherwise, return -1*/
		int solved_one_more_component();

		/*Return the number of remaining components to be solved. If error occurred, return a negative number*/
		int get_number_of_remaining_cmpnts();
		
		void controlled_print(string to_print);
		
		void add_to_lowerbound( int increment);
		void add_to_cost(int increment);
		int get_lowerbound();
		int get_cost();
};

#endif

