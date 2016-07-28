/*
 *  thread_control_office.cpp
 *  MDS
 *
 *  Created by yonghui on 2/16/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "thread_control_office.h"

thread_control_office::thread_control_office( int in_number_of_cnntd_components)
{

	/*initialize the mutex1 object*/
	pthread_mutex_t tmp_mutex = PTHREAD_MUTEX_INITIALIZER;
	mutex1 = tmp_mutex;
	mutex2 = tmp_mutex;
	mutex3 = tmp_mutex;
	mutex4 = tmp_mutex;
	mutex5 = tmp_mutex;
	
	number_of_cnntd_components = in_number_of_cnntd_components;
	next_component_to_solve = 0;
	cnntd_cmpnts_solved = 0;
	lowerbound = 0;
	cost = 0;
	
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


thread_control_office::~thread_control_office()
{

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*If there is no more component to solve, return -1, otherwise, return the id of the next component*/
int thread_control_office::next_cnntd_component_id()
{
	int nxt_cmpnt_id;
	/*Start of the critical region*/
	pthread_mutex_lock( &mutex1 );
	if (next_component_to_solve >= number_of_cnntd_components)
	{
		nxt_cmpnt_id = -1;
	}
	else
	{
		nxt_cmpnt_id = next_component_to_solve;
		next_component_to_solve++;
	}
	/*End of the critical region*/
	pthread_mutex_unlock( &mutex1 );
	return nxt_cmpnt_id;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/*Retrun 0 on success. Otherwise, return -1*/
int thread_control_office::solved_one_more_component()
{
	bool has_error = false ;
	/*Begin of the critical region*/
	pthread_mutex_lock( &mutex2 );
	cnntd_cmpnts_solved++ ;
	if (cnntd_cmpnts_solved > number_of_cnntd_components)
	{
		has_error = true;
	}
	pthread_mutex_unlock( &mutex2 );
	/*End of the critical region*/
	if (has_error)
	{
		cout<< "ERROR: The function thread_control_office::solved_one_more_component is called more times than is expected" << endl;
		return -1;
	}
	else
	{
		return 0;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int thread_control_office::get_number_of_remaining_cmpnts()
{
	int number_of_cmpnts_remaining;
	/*Begin of a critical region*/
	pthread_mutex_lock( &mutex2 );
	number_of_cmpnts_remaining = number_of_cnntd_components - cnntd_cmpnts_solved;
	pthread_mutex_unlock( &mutex2 );
	/*End of a critical region*/
	return number_of_cmpnts_remaining;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void thread_control_office::controlled_print(string to_print)
{
	pthread_mutex_lock( &mutex3 );
	cout << to_print; 
	pthread_mutex_unlock( &mutex3);
	return;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void thread_control_office::add_to_lowerbound( int increment)
{
	pthread_mutex_lock( &mutex4 );
	lowerbound = lowerbound + increment;
	pthread_mutex_unlock( &mutex4);
	return;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void thread_control_office::add_to_cost( int increment)
{
	pthread_mutex_lock( &mutex5 );
	cost = cost + increment;
	pthread_mutex_unlock( &mutex5);
	return;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int thread_control_office::get_lowerbound()
{
	int to_return = 0;
	pthread_mutex_lock( &mutex4 );
	to_return = lowerbound;
	pthread_mutex_unlock( &mutex4);
	return to_return;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int thread_control_office::get_cost()
{
	int to_return = 0;
	pthread_mutex_lock( &mutex5 );
	to_return = cost;
	pthread_mutex_unlock( &mutex5);
	return to_return;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

