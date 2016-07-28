/*
 *  LR_SCP.cpp
 *  MDS
 *
 *  Created by yonghui on 2/27/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "LR_SCP.h"
#include <algorithm>

bool compare_row_id_size_pair(pair<int,int> a, pair<int,int> b)
{
	return a.second<b.second;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LR_SCP_instance::LR_SCP_instance(vector<set<int> > * in_rows, vector<set<int> >* in_columns )
{

	rows = in_rows;
	columns = in_columns;
	num_of_columns = columns->size();
	num_of_rows = rows->size();
	primal_solution.resize(num_of_columns);
	dual_solution.resize(num_of_rows);
	compute_initial_primal_dual();
	compute_max_independent_set();
	dual_solution_type = 0;
	consistency_checking();
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LR_SCP_instance::LR_SCP_instance(string input_file_path)
{
	rows = new vector<set<int> >;
	columns = new vector<set<int> >;

	ifstream input_file(input_file_path.c_str());
	input_file >> num_of_rows;
	rows->resize(num_of_rows);
	
	int tmp_int;
	set<int> tmp_column;	
	int current_column_id = 0;
	while (input_file >> tmp_int)
	{
		if (tmp_int == -1)
		{
			columns->push_back(tmp_column);
			tmp_column.clear();
			current_column_id = current_column_id + 1;
		}
		else
		{
			(*rows)[tmp_int].insert(current_column_id);
			tmp_column.insert(tmp_int);
		}
	}
	input_file.close();
	num_of_columns = columns->size();
	
	primal_solution.resize(num_of_columns);
	dual_solution.resize(num_of_rows);
	compute_initial_primal_dual();
	compute_max_independent_set();
	dual_solution_type = 0;
	consistency_checking();

};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int LR_SCP_instance::compute_initial_primal_dual()
{
	/*Initialize a feasible dual solution*/
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		dual_solution[ii] = 1;
	}
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		set<int> &current_column((*columns)[ii]);
		double greedy_feasible =1.0 / (current_column.size());
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
		{
			if ( dual_solution[*iterator1] > greedy_feasible )
			{
				dual_solution[*iterator1] = greedy_feasible;
			}
		}
	}
	
	lowerbound = 0;
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		lowerbound = lowerbound + dual_solution[ii];
	}
	
	/*Use the greedy heuristic to compute an initial feasible primary solution, and compute the upper bound*/
	/*make a copyf of the rows and the columns objects*/
	vector<set<int> > * tmp_rows = new vector<set<int> >(*rows) ;
	vector<set<int> > * tmp_columns = new vector<set<int> >(*columns);
	SCP_instance scp_problem(tmp_rows, tmp_columns);
	tmp_rows = NULL;
	tmp_columns = NULL; //scp_problem will take care of those two objects
	
	int dummy;
	int cost;
	vector<int> solution;
	scp_problem.find_set_cover_greedy_with_preprocessing(dummy,cost,solution);
	upperbound = cost;
	
	if (upperbound != solution.size())
	{
		cout<< "ERROR: the upperbound is not equal to the cost of the solution, please double check" << endl;
	}
	for (int ii = 0 ; ii < num_of_columns; ii ++)
	{
		primal_solution[ii] = 0;
	}
	for (int ii = 0 ; ii < solution.size(); ii++)
	{
		primal_solution[solution[ii]] = 1;
	}
	
	/*The last step: make some integrity checking. 
	Make sure that the primary solution is indeed a feasible solution.
	*/
	for ( int ii = 0 ; ii < num_of_rows; ii++)
	{
		set<int> &current_row((*rows)[ii]);
		int coverage = 0;
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
		{
			coverage = coverage + primal_solution[*iterator1];
		}
		if (coverage < 1)
		{
			cout << "ERROR: some element is not covered by the initial primal solution" << endl;
		}
	}
	
	return 0;
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


LR_SCP_instance::~LR_SCP_instance()
{
	if (rows != NULL)
	{
		delete rows;
		rows = NULL;
	}
	if (columns != NULL)
	{
		delete columns;
		columns = NULL;
	}
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int LR_SCP_instance::compute_max_independent_set()
{
	/*use a greedy approach to select the maximal independent set*/
	vector<pair<int,int> > row_id_size(num_of_rows);
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		int row_ii_size = ((*rows)[ii]).size();
		row_id_size[ii] = make_pair(ii,row_ii_size);
	}
	
	stable_sort(row_id_size.begin(), row_id_size.end(), compare_row_id_size_pair);
	
	/*choose the independent set*/
	vector<int> columns_chosen(num_of_columns,0);
	
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		/*Check if the row can been added to the max independent set*/
		int current_row_id = row_id_size[ii].first;
	
		set<int> &current_row((*rows)[current_row_id]);
		bool row_is_independent = true;
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
		{
			if (columns_chosen[*iterator1] == 1)
			{
				row_is_independent = false;
				break;
			}
		}
		if (row_is_independent)
		{
			max_independent_set.push_back(current_row_id);
			for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
			{
				columns_chosen[*iterator1] = 1;
			}
		}
	}
	
	stable_sort(max_independent_set.begin(), max_independent_set.end());
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void LR_SCP_instance::to_string()
{
	cout << "the rows:" << endl;
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		cout << "row "<< ii << ":";
		set<int> &current_row((*rows)[ii]);
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
		{
			cout << *iterator1 << ",";
		}
		cout << endl;
	}
	
	cout << "the columns:" << endl;
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		cout << "column "<< ii << ":";
		set<int> &current_column((*columns)[ii]);
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
		{
			cout << *iterator1 << ",";
		}
		cout << endl;
	}
	
	
	cout << "lowerbound: " << lowerbound <<endl;
	cout << "upperbound: " << upperbound << endl;
	cout << "dual solutions:" << endl;
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		cout << dual_solution[ii] << ',';
	}
	cout << endl;
	
	/*Compute the slack for the dual solution*/
	cout << "The slacks are:" << endl;
	double re_calc_lowerbound = 0;
	for (int ii = 0; ii < num_of_rows ; ii++)
	{
		re_calc_lowerbound = re_calc_lowerbound + dual_solution[ii];
	}
	
	for (int ii = 0; ii < num_of_columns ; ii++)
	{
		double slack = 1 ;
		set<int> &current_column((*columns)[ii]);
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
		{
			slack = slack - dual_solution[*iterator1];
		}
		cout << slack << ",";
		if (slack < 0)
		{
			re_calc_lowerbound = re_calc_lowerbound + slack;
		}
	}
	cout << endl;
	cout << "the lowerbound that is recalculated: "<< re_calc_lowerbound << endl;
		
	cout << "primal solution: " << endl;
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		if (primal_solution[ii] > 0)  cout << ii << ',';
	}
	cout << endl;
	
	cout << "the maximal independent set: "<< endl;
	for (int ii = 0 ; ii < max_independent_set.size(); ii++ )
	{
		cout << max_independent_set[ii] << ",";
	}
	cout << endl;
};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*silent is default to be true*/
void LR_SCP_instance::subgradient_optimize(int max_iterations, double starting_lambda, int interval_to_halve_lambda, double alpha_threshold, bool silent)
{
	dual_solution_type = 1;
	/*copy the best and current dual solution from the initial dual solution computed by greedy method*/
	vector<double> best_dual_solution(dual_solution);
	vector<double> current_dual_solution(dual_solution);
	
	double best_lower_bound = lowerbound;
	double current_lower_bound = lowerbound;
	
	double current_lambda = starting_lambda;
	int iterations_left = max_iterations;

	int num_of_iterations_since_no_change = 0;
	while (iterations_left > 0 )
	{
		iterations_left = iterations_left-1;
		
		/*Compute the optimal primal value based on current dual values*/
		/*Compute the new lowerbound*/
		vector<int> current_primal_solution(num_of_columns,0);
		current_lower_bound = 0 ;
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			current_lower_bound = current_lower_bound + current_dual_solution[ii];
		}
	
		for(int ii = 0 ; ii < num_of_columns; ii++)
		{
			set<int> &current_column((*columns)[ii]);
			double slack = 1;
			for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
			{
				slack = slack - current_dual_solution[*iterator1];
			}
			if (slack > 0)
			{
				current_primal_solution[ii] = 0;
			}
			else
			{
				current_lower_bound = current_lower_bound + slack;
				current_primal_solution[ii] = 1;
			}
		}
		
		/*update the best dual solution*/
		if (current_lower_bound > best_lower_bound)
		{
			best_lower_bound = current_lower_bound;
			for (int ii = 0 ; ii < num_of_rows; ii ++)
			{
				best_dual_solution[ii] = current_dual_solution[ii];
			}
			num_of_iterations_since_no_change = 0;
		}
		else
		{
			num_of_iterations_since_no_change = num_of_iterations_since_no_change + 1;
		}
		
		
		/*halve the lambda if the number of iterations since the last increase is more than the threshold*/
		if (num_of_iterations_since_no_change > interval_to_halve_lambda)
		{
			current_lambda = current_lambda/2;
			num_of_iterations_since_no_change = 0;
		}
		
		/*Compute the subgradient*/
		vector<double> current_sub_gradient(num_of_rows, 0);
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			double sub_gradient_ii = 1;
			set<int> &current_row((*rows)[ii]);
			for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
			{
				sub_gradient_ii = sub_gradient_ii - current_primal_solution[*iterator1];
			}
			current_sub_gradient[ii] = sub_gradient_ii;
		}
		
		double sub_gradient_norm = 0 ;
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			sub_gradient_norm = sub_gradient_norm + ((current_sub_gradient[ii]) *(current_sub_gradient[ii]));
		}
		sub_gradient_norm = sqrt(sub_gradient_norm);
		
		if (sub_gradient_norm == 0 )
		{
			/*then optimal solution has been found*/
			break;
		}
		
		double alpha = (upperbound - best_lower_bound) * current_lambda;
		
		/*if alpha is small enough then, we break from the loop*/
		if (alpha < alpha_threshold)
		{
			break;
		}
		
		double sigma = alpha / sub_gradient_norm;
		
		/*Compute the new dual solution*/
		for (int ii = 0 ; ii < num_of_rows; ii ++)
		{
			double new_item = current_dual_solution[ii] + sigma * current_sub_gradient[ii];
			if (new_item > 0)
			{
				current_dual_solution[ii] = new_item;
			}
			else
			{
				current_dual_solution[ii] = 0;
			}
		}

		/*print out some statistics information*/
		if (!silent)
		{
			cout << "current_lower_bound: " << current_lower_bound << ", current lambda:" << current_lambda  << ", current alpha: " << alpha << ", num_of_iterations_since_no_change" << num_of_iterations_since_no_change << ", number of iterations left:" << iterations_left << endl;
		}
		
		/*if the difference between the best lowerbound and the current upper_bound is less than 1, then the optimum solution has been found. */
		if ( upperbound -  best_lower_bound < 1  )
		{
			break;
		}		
	}//end of while
	
	/*copy the best lowerbound and the best dual solution to the member variables*/
	lowerbound = best_lower_bound;
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		dual_solution[ii] = best_dual_solution[ii];
	}

	/*Call dual_feasy to update the dual solution and the lowerbound*/
	dual_feasy();
	
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void LR_SCP_instance::subgradient_optimize_version2(int max_iterations, double starting_lambda, int interval_to_halve_lambda, double alpha_threshold, bool silent)
{
	dual_solution_type = 2;
	
	vector<bool> M_bar(num_of_rows, false);
	for (int ii = 0 ; ii < max_independent_set.size(); ii++)
	{
		M_bar[max_independent_set[ii]] = true;
	}

	/*Construct the new matrix A_prime_columns*/
	vector<set<int> > A_prime_columns(num_of_columns);
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		A_prime_columns[ii] = (*columns)[ii];
		for (int jj = 0 ; jj < max_independent_set.size(); jj ++)
		{
			(A_prime_columns[ii]).erase(max_independent_set[jj]);
		}
	}
	
	
	/*construct  the best_dual_solution and the current_dual_solution vectors*/
	vector<double> best_dual_solution(dual_solution);
	for (int ii = 0 ; ii < num_of_rows; ii ++)
	{
		if (M_bar[ii])
		{
			best_dual_solution[ii] = 0;
		}
	}
	vector<double> current_dual_solution(best_dual_solution);
	
	/*compute the current lowerbound */
	double best_lower_bound = 0; 
	double current_lower_bound = best_lower_bound;
	
	double current_lambda = starting_lambda;
	int iterations_left = max_iterations;

	int num_of_iterations_since_no_change = 0;
	while (iterations_left > 0 )
	{
		iterations_left = iterations_left-1;
		
		/*Compute the optimal primal value based on current dual values*/
		/*Compute the new lowerbound*/
		vector<int> current_primal_solution(num_of_columns,0);
		vector<double> slacks(num_of_columns,0);
		/*Satisfy all the constraints in M_bar by setting some of the primal variables to 1*/
		
		/*Compute the slacks and the current primal solution disregard of the constraints in M_bar*/
		for(int ii = 0 ; ii < num_of_columns; ii++)
		{
			set<int> &current_column(A_prime_columns[ii]);
			double current_slack = 1;
			for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
			{
				current_slack = current_slack - current_dual_solution[*iterator1];
			}
			slacks[ii] = current_slack;
			if (current_slack > 0)
			{
				current_primal_solution[ii] = 0;
			}
			else
			{
				current_primal_solution[ii] = 1;
			}
		}

		/*Adjust the primal solution such that it will satisfy all the constraints in M_bar*/
		for (int ii = 0 ; ii < max_independent_set.size(); ii++)
		{
			int min_index = -1 ;
			double min_slack =  2;
			set<int> &current_row((*rows)[max_independent_set[ii]]);
			for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
			{
				double current_slack_iterator1 = slacks[*iterator1];
				if (current_slack_iterator1 < min_slack) 
				{
					min_slack = current_slack_iterator1;
					min_index = *iterator1;
				}
			}
			if (min_slack >= 0)
			{
				/*The columns has to be chosen*/
				current_primal_solution[min_index] = 1;
			}
		}
		
		/*compute the current lowerbound seperately*/
		current_lower_bound = 0 ;
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			if (M_bar[ii] == false)
			{
				current_lower_bound = current_lower_bound + current_dual_solution[ii];
			}
		}
		for (int ii = 0 ; ii < num_of_columns; ii++)
		{
			current_lower_bound = current_lower_bound + (slacks[ii] * current_primal_solution[ii]);
		}
	
		/*update the best dual solution*/
		if (current_lower_bound > best_lower_bound)
		{
			best_lower_bound = current_lower_bound;
			for (int ii = 0 ; ii < num_of_rows; ii ++)
			{
				best_dual_solution[ii] = current_dual_solution[ii];
			}
			num_of_iterations_since_no_change = 0;
		}
		else
		{
			num_of_iterations_since_no_change = num_of_iterations_since_no_change + 1;
		}
		
		
		/*halve the lambda if the number of iterations since the last increase is more than the threshold*/
		if (num_of_iterations_since_no_change > interval_to_halve_lambda)
		{
			current_lambda = current_lambda/2;
			num_of_iterations_since_no_change = 0;
		}
		
		/*Compute the subgradient*/
		vector<double> current_sub_gradient(num_of_rows, 0);
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			if (not M_bar[ii])
			{
				double sub_gradient_ii = 1;
				set<int> &current_row((*rows)[ii]);
				for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
				{
					sub_gradient_ii = sub_gradient_ii - current_primal_solution[*iterator1];
				}
				current_sub_gradient[ii] = sub_gradient_ii;
			}
		}
		
		double sub_gradient_norm = 0 ;
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			sub_gradient_norm = sub_gradient_norm + ((current_sub_gradient[ii]) *(current_sub_gradient[ii]));
		}
		sub_gradient_norm = sqrt(sub_gradient_norm);
		
		if (sub_gradient_norm == 0 )
		{
			/*then optimal solution has been found*/
			break;
		}
		
		double alpha = (upperbound - best_lower_bound) * current_lambda;
		
		/*if alpha is small enough then, we break from the loop*/
		if (alpha < alpha_threshold)
		{
			break;
		}
		
		double sigma = alpha / sub_gradient_norm;
		
		/*Compute the new dual solution*/
		for (int ii = 0 ; ii < num_of_rows; ii ++)
		{
			double new_item = current_dual_solution[ii] + sigma * current_sub_gradient[ii];
			if (new_item > 0)
			{
				current_dual_solution[ii] = new_item;
			}
			else
			{
				current_dual_solution[ii] = 0;
			}
		}

		/*print out some statistics information*/
		if (!silent)
		{
			cout << "current_lower_bound: " << current_lower_bound << ", current lambda:" << current_lambda  << ", current alpha: " << alpha << ", num_of_iterations_since_no_change" << num_of_iterations_since_no_change << ", number of iterations left:" << iterations_left << endl;
		}
		
		/*if the difference between the best lowerbound and the current upper_bound is less than 1, then the optimum solution has been found. */
		if ( upperbound -  best_lower_bound < 1  )
		{
			break;
		}		
	}//end of while
	
	/*copy the best lowerbound and the best dual solution to the member variables*/
	lowerbound = best_lower_bound;
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		dual_solution[ii] = best_dual_solution[ii];
	}
	
	/*Call dual_feasy to update the dual solution and the lowerbound*/
	dual_feasy();
	return;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LR_SCP_instance::dual_feasy()
{
	vector<double> slacks(num_of_columns,1);
	/*compute the slack values for each column*/
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		double slack_ii = 1;
		set<int> &current_column((*columns)[ii]);
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
		{
			slack_ii = slack_ii - dual_solution[*iterator1];
		}
		slacks[ii] = slack_ii;
	}
	
	/*reduce the dual solution such that none_of the slacks will be a negative value*/
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		int min_id = -1;
		double min_slack = 2;
		set<int> &current_row((*rows)[ii]);
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
		{
			double slack_iterator1 = slacks[*iterator1];
			if (slack_iterator1 < min_slack)
			{
				min_id = * iterator1;
				min_slack = slack_iterator1;
			}
		}
		if (min_slack < 0)
		{
			double dual_ii = dual_solution[ii];
			if (dual_ii + min_slack >= 0)
			{
				dual_solution[ii] = dual_ii + min_slack;
				for (set<int>::iterator iterator1=current_row.begin(); iterator1 != current_row.end(); iterator1++)
				{
					slacks[*iterator1] = slacks[*iterator1] - min_slack;
				}
			}
			else
			{
				dual_solution[ii] = 0 ;
				for (set<int>::iterator iterator1=current_row.begin(); iterator1 != current_row.end(); iterator1++)
				{
					slacks[*iterator1] = slacks[*iterator1] + dual_ii;
				}				
			}
		}//end if
	} //end for
	
	/*Increase the dual solution greedily to a local maximal solution*/
	/*If the dual solution is of type 2, then, increase the dual valables associated with M_bar first*/
	if (dual_solution_type == 2)
	{
		for (int ii = 0 ; ii < max_independent_set.size(); ii++)
		{
			int current_row_id = max_independent_set[ii];
			int min_id = -1;
			double min_slack = 2;
			set<int> &current_row((*rows)[current_row_id]);
			for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
			{
				double slack_iterator1 = slacks[*iterator1];
				if (slack_iterator1 < min_slack)
				{
					min_id = * iterator1;
					min_slack = slack_iterator1;
				}
			}
			if (min_slack > 0)
			{
				dual_solution[current_row_id] = dual_solution[current_row_id] + min_slack;
				for (set<int>::iterator iterator1=current_row.begin(); iterator1 != current_row.end(); iterator1++)
				{
					slacks[*iterator1] = slacks[*iterator1] - min_slack;
				}
			}//end if
		} //end for	
	}//end if
	
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		int min_id = -1;
		double min_slack = 2;
		set<int> &current_row((*rows)[ii]);
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
		{
			double slack_iterator1 = slacks[*iterator1];
			if (slack_iterator1 < min_slack)
			{
				min_id = * iterator1;
				min_slack = slack_iterator1;
			}
		}
		if (min_slack > 0)
		{
			dual_solution[ii] = dual_solution[ii] + min_slack;
			for (set<int>::iterator iterator1=current_row.begin(); iterator1 != current_row.end(); iterator1++)
			{
				slacks[*iterator1] = slacks[*iterator1] - min_slack;
			}
		}//end if
	} //end for	
	
	/*compute the new lowerbound */		
	double new_lower_bound = 0 ;
	for(int ii = 0 ; ii < num_of_rows; ii ++)
	{
		new_lower_bound = new_lower_bound + dual_solution[ii];
	}
	
	if (new_lower_bound < lowerbound - 0.001)// 0.001 is added for numerical stability
	{
		cout << "ERROR, the lowerbound is weakened during dual_feasy" << endl;
		cout << "old lowerbound:"<< lowerbound << "  new_lower_bound:" << new_lower_bound << endl;
		lowerbound = new_lower_bound;
	}
	else
	{
		lowerbound = new_lower_bound;
	}
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LR_SCP_instance::find_set_cover_LR(int &lower_bound, int &cost, vector<int> &solution, LR_Types lr_type)
{
	if (lr_type == LR1)
	{
		subgradient_optimize(MAX_NUMBER_OF_ITERATIONS,STARTING_LAMBDA,INTERVAL_TO_HALVE_LAMBDA,ALPHA_THRESHOLD);
	}
	else if (lr_type == LR2)
	{
		subgradient_optimize_version2(MAX_NUMBER_OF_ITERATIONS,STARTING_LAMBDA,INTERVAL_TO_HALVE_LAMBDA,ALPHA_THRESHOLD);
	}
	
	if (upperbound - lowerbound < 1)
	{
		lower_bound = upperbound;
		cost = upperbound;
		solution.clear();
		for (int ii = 0 ; ii < num_of_columns; ii ++)
		{
			if (primal_solution[ii] == 1)
			{
				solution.push_back(ii);
			}
		}
		/*return the initial greedy solution since it is optimal already*/
	}
	else 
	{
		/*
		compute the slacks for each columns
		pick columns from based on the slacks
		construct a new SCP problem
		compute greedy solution
		compare the new solution to the old solution, and return whichever that is cheaper
		*/
		vector<double> slacks(num_of_columns);
		for (int ii = 0 ; ii < num_of_columns; ii++)
		{
			double slack_ii = 1;
			set<int> &current_column((*columns)[ii]);
			for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
			{
				slack_ii = slack_ii - dual_solution[*iterator1];
			}
			slacks[ii] = slack_ii;
		}

		/*Pick a set of columns based on the slack values*/
		vector<int> tentative_solution;
		for (int ii = 0 ; ii < num_of_columns; ii ++)
		{
			if (slacks[ii] <= 0.0001) //use 0.0001 instead of 0 for numerical stability
			{
				/*then choose the column*/
				tentative_solution.push_back(ii);
			}
		}
		
		/*Run the greedy set cover algorithm on the set of chosen columns*/
		vector<set<int> > * tmp_columns = new vector<set<int> >(tentative_solution.size());
		vector<set<int> > * tmp_rows = new vector<set<int> >(num_of_rows, set<int>());
		for (int ii = 0 ; ii < tentative_solution.size(); ii++)
		{
			(*tmp_columns)[ii] = (*columns)[tentative_solution[ii]];
			set<int> &current_column((*tmp_columns)[ii]);
			for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
			{
				(*tmp_rows)[*iterator1].insert(ii);
			}
		}
		/*check that each row is covered by at least one column*/
		for (int ii = 0 ; ii < num_of_rows; ii++)
		{
			if (((*tmp_rows)[ii]).size() == 0 )
			{
				cout << "ERROR! in function find_set_cover_LR, some row is not covered by any column" << endl;
			}
		}
		SCP_instance tmp_scp_instance(tmp_rows,tmp_columns);
		tmp_rows = NULL;
		tmp_columns = NULL; //tmp_scp_instance will take care of the two objects from now on
		int current_lowerbound = 0;
		int current_cost = 0;
		vector<int> current_solution;
		tmp_scp_instance.find_set_cover_greedy_with_preprocessing(current_lowerbound,current_cost, current_solution);
		
		/*now, return a better solution*/
		if (current_cost < upperbound )
		{
			lower_bound = ceil(lowerbound);
			cost = current_cost;
			solution.clear();
			for (int ii = 0 ; ii< current_solution.size(); ii++)
			{
				solution.push_back(tentative_solution[current_solution[ii]]);
			}
		}
		else //return the initial greedy solution
		{
			lower_bound = ceil(lowerbound);
			cost = upperbound;
			solution.clear();
			for (int ii = 0 ; ii < num_of_columns; ii ++)
			{
				if (primal_solution[ii] == 1)
				{
					solution.push_back(ii);
				}
			}
		}
	}
	return 0;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LR_SCP_instance::consistency_checking()
{
	/*check that the numbers within the rows and the columns are all legitimate*/
	for (int ii = 0; ii < num_of_rows; ii++)
	{
		set<int> &current_row((*rows)[ii]);
		for (set<int>::iterator iterator1= current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
		{
			if (((*iterator1) >= num_of_columns) or ((*iterator1) < 0))
			{
				cout << "EXCEPTION: within SCP_instance::consistency_checking the number is invalid" << endl;
			}
		}
	}
	
	for (int ii = 0; ii < num_of_columns; ii++)
	{
		set<int> &current_column((*columns)[ii]);
		for (set<int>::iterator iterator1= current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
		{
			if (((*iterator1) >= num_of_rows) or ((*iterator1) < 0))
			{
				cout << "EXCEPTION: within SCP_instance::consistency_checking the number is invalid" << endl;
			}
		}
	}
	
	/*check that none of the rows or columns are empty*/
	for (int ii = 0 ; ii < num_of_rows; ii++)
	{
		if (((*rows)[ii]).size() == 0)
		{
			cout << "EXCEPTION within SCP_instance::consistency_checking, an empty row" << endl;
		}
	}
	
	for (int ii = 0 ; ii < num_of_columns; ii++)
	{
		if (((*columns)[ii]).size() == 0 )
		{
			cout << "EXCEPTION within SCP_instance::consistency_checking, an empty column" << endl;
		}		
	}
									
	/*check that the rows and the columns are consistent with one another, and none of them are empty*/
	for (int ii = 0 ; ii < num_of_rows; ii ++)
	{
		set<int> & current_row((*rows)[ii]);
		for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
		{
			if (((*columns)[*iterator1]).find(ii) == ((*columns)[*iterator1]).end())
			{
				cout << "ERROR, rows is not consistent with columns" << endl;
			}
		}
	}
	
	for (int ii = 0 ; ii < num_of_columns; ii ++)
	{
		set<int> & current_column((*columns)[ii]);
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
		{
			if (((*rows)[*iterator1]).find(ii) == ((*rows)[*iterator1]).end())
			{
				cout << "ERROR, rows is not consistent with columns" << endl;
			}
		}
	}
	
};

