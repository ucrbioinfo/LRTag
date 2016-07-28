/*
 *  SCP.h
 *  MDS
 *
 *  Created by yonghui on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef SCP_HEADER
#define SCP_HEADER
 
#include <vector>
#include <utility>
#include <set>
#include <queue>
#include <iostream>
#include <fstream>
#include "LR_SCP.h"
#include "constants.h"


using namespace std;

/*A function object class that compares two pairs. This function object will be used in the priority queue*/
class compare_two_pairs
{
	public:
		int operator()(pair<int,int> a, pair<int,int> b)
		{
			return (a.first <= b.first);
		}

};

/*This object is to be accessed by a single thread*/
class SCP_instance
{

	private:
				
		/*The sub-sets are numbered from 0. */
		/*The elements are also numbered from 0. */
		/*The sparse matrix representation. When a row or a column is empty, it is removed from further consideration. */
		/*The rows and columns objects will be constructed OUTSIDE of this class, but will be destructed INSIDE this class*/
		vector<set<int> > *rows; //correspond to elements
		vector<set<int> > *columns; //correspond to subsets
		
		int number_of_rows; //also the numbere of elements in total
		int number_of_columns; // also the number of subsets in total
		
		/*two vectors indicate whether a sub-set is taken or not. Both are initialized to false.*/
		/*When both are false, the status of the set is not determined yet*/
		/*Whenever a column is fixed, by setting column_chosen[clmn_id] to true or column_excluded[clmn_id] to true, The sparse matrix will be reduced to a new matrix. 
		The fixed columns will be cleared, and those covered rows will be cleared also.
		*/
		bool* column_chosen;
		bool* column_excluded;
		
		/*row_covered[row_id] is set to true when a column is chosen, when a row is excluded from consideration
		  row_excluded[row_id] is set to true when a row is execluded from consideration
		*/
		bool* row_covered;
		bool* row_excluded;
		
		/*The lowerbound on the cost of the solution*/
		int lower_bound ;

		/*Simple heuristics*/
		int exclude_a_column(int column_id);
		int include_a_column(int column_id);
		int exclude_a_row(int row_id);
		
		/*to be called by the constructor by the end of the object construction*/
		void consistency_checking();

		/*Exact heuristics to preprocess the data set and select/execlude some columns and/or rows*/
		int execute_exact_heuristic();

		/*This function carries out greedy selection of SNPs*/
		int find_set_cover_greedy(int &lower_bound, int &cost, vector<int> &solution);
		
	public:
		SCP_instance(vector<set<int> > *in_rows, vector<set<int> > *in_columns);
		SCP_instance(string input_file_path);
		
		~SCP_instance();
		
		/*This function executes greedy only*/
		int find_set_cover_greedy_no_preprocessing(int &lower_bound, int &cost, vector<int> &solution);
		
		/*This function executes preprocessing with exact heuristic and greedy*/
		int find_set_cover_greedy_with_preprocessing(int &lower_bound, int &cost, vector<int> &solution);
		
		/*This function executes exact heuristic + best_of{LR,greedy}*/
		int find_set_cover_best_of_LR_and_greedy_with_preprocessing(int &lower_bound, int &cost, vector<int> &solution, LR_Types lr_type);
		
		/*If the function successfully fixed any column, return an positive number, otherwise, return 0*/
		int columns_reduce();
		
		/*If the function successfully fixed any row or excluded any row, retrun an positive number, otherwise, return 0 */
		int rows_reduce(); 
		
		/*This function used for debugging. Print the current object*/
		int to_string();
		
		/*preselect a set of SNPs*/
		/*If this function is ever caled, it should be called first*/
		int pre_select(vector<int> preselected_columns);
		
};

#endif