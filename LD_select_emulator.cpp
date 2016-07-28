/*
 *  LD_select_emulator.cpp
 *  MDS
 *
 *  Created by yonghui on 3/8/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "LD_select_emulator.h"

LD_select::LD_select(sgl_ppl_loci_correlation * input_population)
{

	total_number_of_loci = 0;
	string_loci.clear();
	string_2_id_map.clear();
	edges.clear();
	
	set<string>* inpt_loci = input_population->get_loci();
	set<edge_type>* inpt_str_edges = input_population->get_edges();
	for (set<string>::iterator iterator1 = inpt_loci->begin(); iterator1 != inpt_loci->end(); iterator1 ++)
	{
		string_loci.push_back(*iterator1);
		string_2_id_map[*iterator1] = total_number_of_loci;
		total_number_of_loci = total_number_of_loci + 1;
	}
	edges.resize(total_number_of_loci);
	for (set<edge_type>::iterator iterator1 = inpt_str_edges->begin(); iterator1 != inpt_str_edges->end(); iterator1 ++)
	{
		int id1 = string_2_id_map[iterator1->first];
		int id2 = string_2_id_map[iterator1->second];
		(edges[id1]).insert(id2);
		(edges[id2]).insert(id1);
	}
	
	/*Add all self-loops into the graph*/
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		(edges[ii]).insert(ii);
	}
	
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LD_select::~LD_select()
{
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int LD_select::greedy_select(string output_file_path, vector<vector<string> > &bins)
{
	bins.clear();
	ofstream output_bin_file(output_file_path.c_str());
	vector<bool> vertex_covered(total_number_of_loci,false);
	
	vector<int> vertex_utility(total_number_of_loci);
	priority_queue<pair<int,int>, vector<pair<int,int> >, compare_two_pairs> next_vertex_to_choose;
	
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		vertex_utility[ii] = (edges[ii]).size();
		next_vertex_to_choose.push(make_pair(vertex_utility[ii],ii));
	}
	
	int next_bin_id = 1;
	
	while (not (next_vertex_to_choose.empty()))
	{
		pair<int, int> tmp_pair = next_vertex_to_choose.top();
		next_vertex_to_choose.pop();
		int current_vertex_id = tmp_pair.second;
		int current_utility = tmp_pair.first;
		
		/*If the current vertex is not covered and the pair is not deleted (lazy deletion)*/
		if (not (vertex_covered[current_vertex_id] or (vertex_utility[current_vertex_id] < current_utility) ))
		{
			set<int> tag_SNPs;
			set<int> other_SNPs;
			
			/*Make a new bin*/
			set<int> &allSNPs(edges[current_vertex_id]);
			
			/*Check the alternative tagSNPs*/
			for ( set<int>::iterator iter1 = allSNPs.begin(); iter1 != allSNPs.end(); iter1 ++ )
			{
				if ((edges[current_vertex_id]).size() == (edges[*iter1]).size())
				{
					/*the two edge sets are the same?*/
					if (allSNPs == edges[*iter1])
					{
						tag_SNPs.insert(*iter1);
					}
					else
					{
						other_SNPs.insert(*iter1);
					}
				}
				else
				{
					other_SNPs.insert(*iter1);
				}
			}
			/*write the output to the file*/
			output_bin_file << "Bin "<< next_bin_id << "\ttotal_sites: "<< (edges[current_vertex_id]).size()<< " average_minor_allele_frequency: 50.0%" << endl;
			output_bin_file << "Bin "<< next_bin_id << "\tTagSnps:";
			for (set<int>::iterator iter1 = tag_SNPs.begin(); iter1 != tag_SNPs.end(); iter1 ++)
			{
				output_bin_file << " " << string_loci[*iter1];
			}
			output_bin_file << endl;
			output_bin_file << "Bin "<< next_bin_id << "\tother_snps:";
			for (set<int>::iterator iter1 = other_SNPs.begin(); iter1 != other_SNPs.end(); iter1 ++)
			{
				output_bin_file << " " << string_loci[*iter1];
			}
			output_bin_file << endl;
			output_bin_file << endl;
			
			/*Insert a new bin to the bins objects*/
			bins.push_back(vector<string>());
			for (set<int>::iterator iter1 = tag_SNPs.begin(); iter1 != tag_SNPs.end(); iter1 ++)
			{
				bins[next_bin_id-1].push_back(string_loci[*iter1]);
			}			
	
			/*Modify the graph, at the same time modify the priority queue*/
			for (set<int>::iterator iter1 = allSNPs.begin(); iter1 != allSNPs.end(); iter1++)
			{
				vertex_covered[*iter1] = true;
			}
			for(set<int>::iterator iter1 = allSNPs.begin(); iter1 != allSNPs.end(); iter1++)
			{
				for (set<int>::iterator iter2 = (edges[*iter1]).begin(); iter2 != (edges[*iter1]).end(); iter2++)
				{
					if (vertex_covered[*iter2] == false)
					{
						edges[*iter2].erase(*iter1);
						vertex_utility[*iter2] = vertex_utility[*iter2] - 1;
						next_vertex_to_choose.push(make_pair(vertex_utility[*iter2],*iter2));

					}
				}
			}
						
			for (set<int>::iterator iter1 = allSNPs.begin(); iter1 != allSNPs.end(); iter1 ++)
			{
				if (not (*iter1 == current_vertex_id))
				{
					vertex_utility[*iter1] = 0;
					(edges[*iter1]).clear();
				}
			}
			vertex_utility[current_vertex_id] = 0 ;
			(edges[current_vertex_id]).clear();
			/*Increase the bin id*/
			next_bin_id = next_bin_id + 1;
			/*continue */
		}
	}
	
	output_bin_file.close();
	return next_bin_id - 1;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int LD_select::dump()
{
	cout << total_number_of_loci << endl;
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		cout << string_loci[ii] << ",";
	}
	cout << endl;
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		cout << ii <<":";
		for (set<int>::iterator iter1=(edges[ii]).begin(); iter1 != (edges[ii]).end(); iter1++ )
		{
			cout << *iter1 << ",";
		}
		cout << endl;
	}
	return 0;
};
