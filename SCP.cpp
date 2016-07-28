/*
 *  SCP.cpp
 *  MDS
 *
 *  Created by yonghui on 2/15/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "SCP.h"

SCP_instance::SCP_instance(vector<set<int> > *in_rows, vector<set<int> > *in_columns)
{
	rows = in_rows;
	columns = in_columns;
	number_of_rows = in_rows->size();
	number_of_columns = in_columns->size();
	column_chosen = new bool[number_of_columns];
	column_excluded = new bool[number_of_columns];
	row_covered = new bool[number_of_rows];
	row_excluded = new bool[number_of_rows];
	for (int ii = 0; ii < number_of_columns; ii++)
	{
		column_chosen[ii] = false;
		column_excluded[ii] = false;
	}
	for (int ii = 0; ii < number_of_rows; ii++)
	{
		row_covered[ii] = false;
		row_excluded[ii] = false;
	}
	lower_bound = 0;
	consistency_checking();
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*Read the set cover instance from a file*/
SCP_instance::SCP_instance(string input_file_path)
{
	rows = new vector<set<int> >;
	columns = new vector<set<int> >;

	ifstream input_file(input_file_path.c_str());
	input_file >> number_of_rows;
	rows->resize(number_of_rows);
	
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
	number_of_columns = columns->size();
	column_chosen = new bool[number_of_columns];
	column_excluded = new bool[number_of_columns];
	row_covered = new bool[number_of_rows];
	row_excluded = new bool[number_of_rows];
	for (int ii=0 ; ii < number_of_columns; ii++)
	{
		column_chosen[ii] = false;
		column_excluded[ii] = false;
	}
	for (int ii = 0; ii < number_of_rows; ii++)
	{
		row_covered[ii] = false;
		row_excluded[ii] = false;
	}	
	lower_bound = 0;
	consistency_checking();
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


SCP_instance::~SCP_instance()
{
	rows->clear();
	if (rows != NULL)
	{
		delete rows;
		rows = NULL;
	}
	columns->clear();
	if (columns != NULL)
	{
		delete columns;
		columns = NULL;
	}
	if (column_chosen != NULL)
	{
		delete[] column_chosen;
		column_chosen = NULL;
	}
	if (column_excluded != NULL)
	{
		delete[] column_excluded;
		column_excluded = NULL;
	}
	if (row_covered != NULL)
	{
		delete[] row_covered;
		row_covered = NULL;
	}
	if (row_excluded != NULL)
	{
		delete[] row_excluded;
		row_excluded = NULL;
	}
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::execute_exact_heuristic()
{
	bool changed = true;
	while(changed)
	{
		changed = false;
		columns_reduce();
		int tmp_int = 0 ;
		tmp_int = rows_reduce();
		if (tmp_int > 0) changed = true;
	}
	/*On success, return 0*/
	return 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*This function executes greedy only*/
int SCP_instance::find_set_cover_greedy_no_preprocessing(int &lower_bound, int &cost, vector<int> &solution)
{
	find_set_cover_greedy(lower_bound, cost, solution);
	return 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
/*This function executes preprocessing with exact heuristic and greedy*/
int SCP_instance::find_set_cover_greedy_with_preprocessing(int &lower_bound, int &cost, vector<int> &solution)
{
	execute_exact_heuristic();
	find_set_cover_greedy(lower_bound, cost, solution);
	return 0;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::find_set_cover_greedy(int &lower_bound, int &cost, vector<int> &solution)
{
	
	/*Should consider the following factors:
		1: Some of the Rows and Columns might be empty. In that case, that row or column has been chosen or excluded.
		2: The column_chosen and column_excluded vectors might not be all false before the execution of this function.
	*/

	/*The first element of a pair is the value of choosing a column, and the second element is the id of the column*/
	priority_queue<pair<int,int>, vector<pair<int, int> >, compare_two_pairs > next_column_to_choose;
	
	/*Implement lazy deletion in priority queue*/
	vector<int> clmn_values(number_of_columns);

	/*initizlize the priority queue*/
	for (int ii=0; ii < number_of_columns; ii++)
	{
		if ((not column_chosen[ii]) and (not column_excluded[ii] ))
		{
			int value_of_current_column = ((*columns)[ii]).size();
			clmn_values[ii] = value_of_current_column;
			if (value_of_current_column > 0)
			{
				next_column_to_choose.push(make_pair(value_of_current_column,ii));
			}
		}//end if
	}//end for

	/*While the queue is not empty*/
	while (not next_column_to_choose.empty())
	{
	

		pair<int,int> tmp_pair = next_column_to_choose.top();
		next_column_to_choose.pop();
		int next_clmn = tmp_pair.second;

		/*Do if tmp_pair hasn't been deleted yet*/
		if (tmp_pair.first == clmn_values[next_clmn])
		{
			/*Do if next_clmn has not been chosen yet. This is a safty guard.*/
			if (not column_chosen[next_clmn])
			{
				column_chosen[next_clmn] = true;
				set<int> column_picked = (*columns)[next_clmn]; //make a copy of the object;
				
				/*With the column picked, reduce the problem to a new problem*/
				/* (*iterator1, *iterator2) points to a particular element of an array*/
				for (set<int>::iterator iterator1=column_picked.begin(); iterator1 != column_picked.end(); iterator1++)
				{
					set<int> &rr_column_chosen((*rows)[*iterator1]);
					for (set<int>::iterator iterator2 = rr_column_chosen.begin(); iterator2 != rr_column_chosen.end(); iterator2++)
					{
						int elements_get_removed = (*columns)[*iterator2].erase(*iterator1);	
						clmn_values[*iterator2] = clmn_values[*iterator2] - elements_get_removed;
						if (clmn_values[*iterator2] > 0)
						{
							next_column_to_choose.push(make_pair(clmn_values[*iterator2] , *iterator2));
						}
					}
					((*rows)[*iterator1]).clear(); //when done for a row
					row_covered[*iterator1] = true; //The row has been covered
				} 
				((*columns)[next_clmn]).clear();
			}//end if
		}
	}
	
	/*The last step, construct the return value*/
	/*Should calculate a better lowerbound later*/
	lower_bound = this->lower_bound;
	cost = 0;
	solution.clear();
	for (int ii=0; ii < number_of_columns; ii++)
	{
		if (column_chosen[ii])
		{
			cost = cost + 1;
			solution.push_back(ii);
		}
	}
	/*On success, return 0*/
	return 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::to_string()
{
	cout << "print out the rows:" << endl;
	for (int ii = 0; ii < number_of_rows; ii++)
	{
		if (((*rows)[ii]).size() > 0)
		{
			cout << ii << ":" << "size " << ((*rows)[ii]).size() << ":";
			for (set<int>::iterator iterator1 = ((*rows)[ii]).begin(); iterator1 != ((*rows)[ii]).end() ; iterator1 ++)
			{
				cout << *iterator1 << "," ;
			}
			cout << endl;
		}
	}

	cout << "print out the columns:" << endl;
	for (int ii = 0; ii < number_of_columns; ii++)
	{
		if (((*columns)[ii]).size() > 0)
		{
			cout << ii << ":" << "size " << ((*columns)[ii]).size()  << ":";
			for (set<int>::iterator iterator1 = ((*columns)[ii]).begin(); iterator1 != ((*columns)[ii]).end() ; iterator1 ++)
			{
				cout << *iterator1 << "," ;
			}
			cout << endl;
		}
	}
	
	cout << "column_chosen:" << endl;
	for (int ii = 0 ; ii < number_of_columns; ii ++)
	{
		if (column_chosen[ii])
		{
			cout << ii << "," ;
		}
	}
	cout << endl;
	
	cout << "column_excluded:" << endl;
	for (int ii = 0 ; ii < number_of_columns; ii ++)
	{
		if (column_excluded[ii])
		{
			cout << ii << "," ;
		}
	}
	cout << endl;
	
	cout << "row_covered:" << endl;
	for (int ii = 0 ; ii < number_of_rows; ii ++)
	{
		if (row_covered[ii])
		{
			cout << ii << "," ;
		}
	}
	cout << endl;

	cout << "row_excluded:" << endl;
	for (int ii = 0 ; ii < number_of_rows; ii ++)
	{
		if (row_excluded[ii])
		{
			cout << ii << "," ;
		}
	}
	cout << endl;

	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::exclude_a_column(int column_id)
{
	column_excluded[column_id] = true;
	set<int> &current_column((*columns)[column_id]);
	for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1++)
	{
		((*rows)[*iterator1]).erase(column_id);
	}
	((*columns)[column_id]).clear();
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::include_a_column(int column_id)
{
	column_chosen[column_id] = true;

	/*Make a copy of the column*/
	set<int> copy_of_the_column = (*columns)[column_id];
	
	/* (*iterator1) is the row_id */
	for (set<int>::iterator iterator1 = copy_of_the_column.begin(); iterator1 != copy_of_the_column.end(); iterator1++)
	{
		set<int> &current_row((*rows)[*iterator1]);
		/* (*iterator2) is the column id */
		for (set<int>::iterator iterator2 = current_row.begin(); iterator2 != current_row.end(); iterator2++)
		{
			((*columns)[*iterator2]).erase(*iterator1);
		}
		((*rows)[*iterator1]).clear();
		row_covered[*iterator1] = true;
	};
	((*columns)[column_id]).clear();
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::exclude_a_row(int row_id)
{
	row_covered[row_id] = true;
	row_excluded[row_id] = true;
	set<int> &current_row((*rows)[row_id]);
	for (set<int>::iterator iterator1 = current_row.begin(); iterator1 != current_row.end(); iterator1++)
	{
		((*columns)[*iterator1]).erase(row_id);
	}
	((*rows)[row_id]).clear();
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::columns_reduce()
{
	bool reduced_any_column = false;
	vector<int> columns_to_remove;
	vector<int> list_of_columns_to_consider;
	for (int ii = 0 ; ii < number_of_columns; ii++)
	{
		/*do the following only if the status of this column has not been determined yet*/
		if ( not ((column_chosen[ii]) or (column_excluded[ii]) or (((*columns)[ii]).size() == 0)) )
		{
			list_of_columns_to_consider.push_back(ii);
		}
	}
	
	int total_number_of_columns_left = 	list_of_columns_to_consider.size();
	for (int ii = 0 ; ii < total_number_of_columns_left; ii ++)
	{
		int column1_id = list_of_columns_to_consider[ii];
		/*try to figure out if column list_of_columns_to_consider[ii] is fully contained in any other column */
		for (int jj = 0; jj < total_number_of_columns_left; jj ++)
		{
			int column2_id = list_of_columns_to_consider[jj];
			/*now compare if column1 is contained in column2*/
			bool column1_in_column2 = false;
			if ( ((((*columns)[column1_id]).size()) < (((*columns)[column2_id]).size())) 
					or (((((*columns)[column1_id]).size()) == (((*columns)[column2_id]).size())) and (column1_id < column2_id)))
			{
				column1_in_column2 = true;
				for (set<int>::iterator iterator1 = ((*columns)[column1_id]).begin(); iterator1 != ((*columns)[column1_id]).end(); iterator1++)
				{
					set<int>::iterator nulliterator = ((*columns)[column2_id]).end();
					if (((*columns)[column2_id]).find(*iterator1) == nulliterator)
					{
						column1_in_column2 = false;
						break;
					}
				}
			}
			if (column1_in_column2)
			{
				columns_to_remove.push_back(column1_id);
				reduced_any_column = true;
				break;
			}
			
		}
		
	}

	for (int ii = 0 ; ii < columns_to_remove.size(); ii ++)
	{
		exclude_a_column(columns_to_remove[ii]);
	}
	if (reduced_any_column)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SCP_instance::rows_reduce()
{
	bool reduced_any_row = false;
	/*remove the singletons*/
	for (int ii = 0 ; ii < number_of_rows; ii++)
	{
		if (not ((row_covered[ii]) or (row_excluded[ii])))
		{
			
			/*check the invariant of the object*/
			if (((*rows)[ii]).size() == 0)
			{
				cout << "EXCEPTION: the invariant condition is not met"<< endl;
				this->to_string();
			}
			if ( ((*rows)[ii].size()) == 1 )
			{
				/*We should fix that row*/
				this->lower_bound = this->lower_bound + 1;
				int column_id = (*((*rows)[ii]).begin());
				include_a_column(column_id);
				reduced_any_row = true;
			}
		}
	}
	
	/*remove some rows from consideration if it completely contains another row*/
	vector<int> rows_left;
	for (int ii = 0 ; ii < number_of_rows; ii++)
	{
		if (not ((row_covered[ii]) or (row_excluded[ii])))
		{
			if (((*rows)[ii]).size() == 0)
			{
				cout << "EXCEPTION: the invariant condition is not met"<< endl;
				this->to_string();
			}
			else
			{
				rows_left.push_back(ii);
			}
		}
	}
	int number_of_rows_left = rows_left.size();
	vector<int> rows_to_be_excluded;
	for (int ii = 0 ; ii < number_of_rows_left; ii++)
	{
		int row1_id = rows_left[ii];
		for (int jj = 0 ; jj < number_of_rows_left; jj++)
		{
			int row2_id = rows_left[jj];
			/*check if row_2 is completely contained in row_1, if so, row_1 can be removed from consideration*/
			bool row2_in_row1 = false;
			if (  (((*rows)[row1_id]).size() > ((*rows)[row2_id]).size()) or
					(((((*rows)[row1_id]).size() == ((*rows)[row2_id]).size())) and  (row1_id > row2_id)))
			{
				row2_in_row1 = true;
				for (set<int>::iterator iterator1 = ((*rows)[row2_id]).begin(); iterator1 != ((*rows)[row2_id]).end(); iterator1 ++)
				{
					set<int>::iterator null_iterator = ((*rows)[row1_id]).end();
					if ( ((*rows)[row1_id]).find(*iterator1) == null_iterator )
					{
						row2_in_row1 = false;
						break;
					}
				}
			}
			
			if (row2_in_row1)
			{
				rows_to_be_excluded.push_back(row1_id);
				reduced_any_row = true;
				break;
			}
		}
	}
	for (int ii = 0 ; ii < rows_to_be_excluded.size(); ii ++)
	{
		exclude_a_row(rows_to_be_excluded[ii]);
	}
	if (reduced_any_row)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int SCP_instance::find_set_cover_best_of_LR_and_greedy_with_preprocessing(int &lower_bound, int &cost, vector<int> &solution, LR_Types lr_type)
{
	execute_exact_heuristic();
	/*construct a new column set*/
	vector<int> new_columns_set;
	for (int ii = 0 ; ii < number_of_columns; ii++)
	{
		if ( (not column_chosen[ii]) and (not column_excluded[ii]) and (not ((*columns)[ii]).size() == 0) )
		{
			new_columns_set.push_back(ii);
		}
	}
	vector<int> new_rows_set;
	for (int ii = 0 ; ii < number_of_rows ; ii ++)
	{
		if ( (not row_covered[ii]) and ( not row_excluded[ii]) and (not (((*rows)[ii]).size()==0)))
		{
			new_rows_set.push_back(ii);
		}
	}
	vector<int> reverse_row_id_mapping(number_of_rows, -1);
	for (int ii = 0 ; ii < new_rows_set.size(); ii ++)
	{
		reverse_row_id_mapping[new_rows_set[ii]] = ii;
	}
	
	vector<set<int> > * tmp_columns = new vector<set<int> >(new_columns_set.size(), set<int>());
	vector<set<int> > * tmp_rows = new vector<set<int> >(new_rows_set.size(), set<int>());
	for (int ii = 0 ; ii < new_columns_set.size(); ii++)
	{
		set<int> &current_column((*columns)[new_columns_set[ii]]);
		for (set<int>::iterator iterator1 = current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
		{
			int jj = reverse_row_id_mapping[*iterator1];
			if (jj == -1)
			{
				cout << "ERROR! in find_set_cover, row_id is not mapped" <<endl;
			}
			(*tmp_columns)[ii].insert(jj);
			(*tmp_rows)[jj].insert(ii);
		}
	}
	
	/*check that each row is covered by at least one column*/
	for (int ii = 0 ; ii < tmp_rows->size(); ii++)
	{
		if (((*tmp_rows)[ii]).size() == 0)
		{
			cout << "ERROR!  some row is not covered by the reduced set" << endl;
		}
	}
	
	/*Call LR to solve this problem*/
	LR_SCP_instance tmp_lr_scp_instance(tmp_rows, tmp_columns);
	tmp_rows = NULL;
	tmp_columns = NULL; //tmp_lr_scp_instance will take care of the two instances
	
	int current_lower_bound = 0 ;
	int current_cost = 0;
	vector<int> current_solution ;
	
	tmp_lr_scp_instance.find_set_cover_LR(current_lower_bound, current_cost, current_solution, lr_type);

	
	/*combine the lr solution for the problem core with the solution obtained by exact heuristic and return the solution to the caller function*/
	lower_bound = this->lower_bound + current_lower_bound;
	cost = 0 ;
	solution.clear();
	for (int ii = 0 ; ii < current_solution.size(); ii ++)
	{
		solution.push_back(new_columns_set[current_solution[ii]]);
		cost = cost + 1;
	}
	
	for (int ii = 0 ; ii < number_of_columns; ii ++)
	{
		if (column_chosen[ii])
		{
			cost = cost + 1;
			solution.push_back(ii);
		}
	}
	
	return 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SCP_instance::consistency_checking()
{
	/*check that the numbers within the rows and the columns are all legitimate*/
	for (int ii = 0; ii < number_of_rows; ii++)
	{
		set<int> &current_row((*rows)[ii]);
		for (set<int>::iterator iterator1= current_row.begin(); iterator1 != current_row.end(); iterator1 ++)
		{
			if (((*iterator1) >= number_of_columns) or ((*iterator1) < 0))
			{
				cout << "EXCEPTION: within SCP_instance::consistency_checking the number is invalid" << endl;
			}
		}
	}
	
	for (int ii = 0; ii < number_of_columns; ii++)
	{
		set<int> &current_column((*columns)[ii]);
		for (set<int>::iterator iterator1= current_column.begin(); iterator1 != current_column.end(); iterator1 ++)
		{
			if (((*iterator1) >= number_of_rows) or ((*iterator1) < 0))
			{
				cout << "EXCEPTION: within SCP_instance::consistency_checking the number is invalid" << endl;
			}
		}
	}
	
	/*check that none of the rows or columns are empty*/
	for (int ii = 0 ; ii < number_of_rows; ii++)
	{
		if (((*rows)[ii]).size() == 0)
		{
			cout << "EXCEPTION within SCP_instance::consistency_checking, an empty row" << endl;
		}
	}
	
	for (int ii = 0 ; ii < number_of_columns; ii++)
	{
		if (((*columns)[ii]).size() == 0 )
		{
			cout << "EXCEPTION within SCP_instance::consistency_checking, an empty column" << endl;
		}		
	}
									
	/*check that the rows and the columns are consistent with one another, and none of them are empty*/
	for (int ii = 0 ; ii < number_of_rows; ii ++)
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
	
	for (int ii = 0 ; ii < number_of_columns; ii ++)
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
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int SCP_instance::pre_select(vector<int> preselected_columns)
{
	for (int ii = 0 ; ii < preselected_columns.size(); ii++)
	{
		if ((preselected_columns[ii] >= number_of_columns) or (preselected_columns[ii] < 0))
		{
			cout << "The id of the preselected column is out of range" << endl;
		}
		else
		{
			include_a_column(preselected_columns[ii]);

		}
	}
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



