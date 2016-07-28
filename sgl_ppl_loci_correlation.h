/*
 *  sgl_ppl_loci_correlation.h
 *  MDS
 *
 *  Created by yonghui on 2/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *  
 */

#ifndef sgl_ppl_loci_correlation_HEADER
#define sgl_ppl_loci_correlation_HEADER

#include <set>
#include <map>
#include <string>
#include <cstring>
#include <utility>
#include <fstream>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <algorithm>
#include <queue>
#include "SCP.h"
#include "thread_control_office.h"
#include "reaper_emulator.h"


using namespace std;

typedef pair<string,string> edge_type;
/*Each edge is a pair of SNPs*/

/*This class is to be accessed by a single thread*/

class sgl_ppl_loci_correlation
{
	private:
		float correlation_cut_off;
		float maf_cut_off;  
		set<string> loci; //A set of loci
		map<string,float> loci_maf_map;
		set<edge_type> edges; // A set of edges
	public:
		sgl_ppl_loci_correlation(float cut_off_threshold, float in_maf_cut_off); 
		sgl_ppl_loci_correlation();
		int read_in_ppl_correlation_file(string file_path, string maf_file_path);
		int clear();
		
		int set_correlation_cut_off(float cut_off_threshold,float in_maf_cut_off);
		set<string>* get_loci();
		set<edge_type>* get_edges();
		
		/*This static function will be called by multiple threads*/
		static void* thread_func_read_in_ppl_correlation_file(void* paras);
		static pthread_mutex_t mutex_cout ; //used for snychronizing access to the cout object

};

/*This struct is used for passing parameters to the thread entry function*/
struct thread_initialization_para
{
	sgl_ppl_loci_correlation* ppl;
	string file_path;
	string maf_file_path;
};

/*This object will be accessed by multiple thread*/
class graph_linked_list
{
	public:
		/*The number of SNP sites that a particular population contains*/
		int own_SNPs; 

		/*The total number of unique SNP sites that are present in all population*/
		int total_SNPs; 

		/*If a particular SNP site is present in this population, the corresponding element of this vector is true, otherwise it is false	
		vertices is used as an array
		The memory for this array is allocated and released in this object*/
		bool  * vertices; 

		/*edges in linked list format
		edges is used as an array
		The memory is allocated and released within this object*/
		vector<int> * edges;
		
		/*The constructor and destructor*/
		graph_linked_list(int in_total_SNPs);
		graph_linked_list();
		int initialize(int in_total_SNPs);
		int set_own_SNPs(int in_own_SNPs);
		~graph_linked_list();
	
};

/*This object will be accessed by multiple threads*/
class multi_ppi_loci_correlation
{
	friend class Reaper_Emulator;
	private:
		/*A dictionary maps a SNP string id into an integer id*/
		map<string,int> SNP_2_id; 
		
		/*A dictionary maps an integer id to the SNP string id*/
		map<int,string> id_2_SNP; 
		
		/*The set of all possible SNP string ids*/
		set<string> loci; 
		
		/*The total number of unique SNP sites in all populations*/
		int total_number_of_loci; 
		
		/*A vector of constituent graphs
		The memory is allocated and released within this object*/
		graph_linked_list * constituent_graphs;
		
		/*Number of constituent populations*/
		int number_of_ppls;
		
		/*Two vectors indicates whether a loci will be in the cover or not*/
		bool* in_cover;
		
	public:
	
		/*The constructor takes a set of populations and combine them. The parameters are passed via reference to avoid copying*/
		multi_ppi_loci_correlation(vector<sgl_ppl_loci_correlation> &populations);
		
		~multi_ppi_loci_correlation();
		/*Print out statistics of the current object for debugging. A function to be called by a single thread.*/
		int dump_statistics(bool verbose);
		
		/*Compute and return the connected components. A function to be called by a single thread
		The components array cnntd_cmpnts will be initialized within this function and will released outside of that function*/
		int connected_components( vector<int>  **cnntd_cmpnts, int &number_of_cnntd_cmpnts);
		
		/*This function will be called by multiple threads concurrently, but this should not cause any contension since each thread will be calling this function on a very different connected components*/
		int construct_SCP_instance(SCP_instance ** in_scp_instance, vector<int> &cnntd_component);
		
		string int_id_2_SNP_lable(int int_id);

};


#endif
