/*
 *  constants.h
 *  MDS
 *
 *  Created by yonghui on 2/18/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <map>
#include <set>
#include <queue>
#include <fstream>
#include <iterator>
#include <utility>
#include <pthread.h>
#include "sgl_ppl_loci_correlation.h"
#include "SCP.h"
#include <ctime>
#include "MDS_solver.h"
#include "LR_SCP.h"
#include "reaper_emulator.h"
#include "multi_pop_select.h"
#include "LD_select_emulator.h"
using namespace std;

void print_out_usage()
{
	cout << "ERROR! the correct usuage of this utility is as follows:" << endl;
 	cout << "main.exe  r_square_cut_off_threshold, maf_cut_off_threshold file1.lod file1.maf file2.lod file2.maf ..."<< endl;
};

int main(int argc, char** argv)
{      

	/*Print out the current time*/
	time_t starting_second;
	starting_second = time(NULL);
	cout << "starting time is:" << starting_second << endl;
	
	time_t ending_second;
	
	if (argc < 5)
	{
		print_out_usage();
		ending_second = time(NULL);
		cout << "ending time is:" << ending_second << endl;
		return -1;
	}
	
  	string mds_method = "LR1_WITH_PREPROCESSING";
	
	float cut_off_threshold = atof(argv[1]);
	float maf_cut_off = atof(argv[2]);
	int number_of_populations = (argc -3)/2;
	vector<sgl_ppl_loci_correlation> populations(number_of_populations, sgl_ppl_loci_correlation(cut_off_threshold,maf_cut_off));
	
	/*Read in the files concurrently*/
	pthread_t threads[number_of_populations];
	int ret_values[number_of_populations];
	thread_initialization_para paras[number_of_populations];
	for (int ii = 0 ; ii < number_of_populations; ii++)
	{
		paras[ii].ppl = &(populations[ii]);
		paras[ii].file_path = argv[2*ii+3];
		paras[ii].maf_file_path = argv[2*ii + 4];
		ret_values[ii] = pthread_create(&(threads[ii]),NULL,sgl_ppl_loci_correlation::thread_func_read_in_ppl_correlation_file, (void*)(&(paras[ii])));
	}
	for (int ii = 0; ii < number_of_populations; ii++)
	{
		pthread_join(threads[ii],NULL);
	}
	/*end of reading the input files*/
	
    multi_ppi_loci_correlation* combined = new multi_ppi_loci_correlation(populations);
    combined->dump_statistics(false);
    populations.clear();
    
    
    MDS_solver * mds_slvr_instance;

    mds_slvr_instance = new MDS_solver(combined, LR1_WITH_PREPROCESSING);	

    combined = NULL ; // mds_slvr_instance will take care of the destruction of the combined object.

    int lowerbound;
    int cost;
    vector<string> min_common_dom_set;
    mds_slvr_instance->concurrent_mds_slvr(lowerbound,cost,min_common_dom_set);

    cout << "The lowerbound is:" << lowerbound << endl;
    cout << "The cost is:" << cost << endl;
    cout << "The solution is:" << endl;
    for (int ii = 0 ; ii < min_common_dom_set.size(); ii++)
    {
        cout << min_common_dom_set[ii] << ",";
    }

    delete mds_slvr_instance;
    
    ending_second = time(NULL);
    cout << "ending time is:" << ending_second << endl;		
        
    return 0;
	
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



