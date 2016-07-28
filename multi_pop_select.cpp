/*
 *  multi_pop_select.cpp
 *  MDS
 *
 *  Created by yonghui on 3/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "multi_pop_select.h"

Multi_Pop_Select::Multi_Pop_Select(vector<vector<string> > &bin_SNPs)
{
	all_tag_SNPs.clear();
	for (int ii = 0 ; ii < bin_SNPs.size(); ii++)
	{
		all_tag_SNPs.insert((bin_SNPs[ii]).begin(),(bin_SNPs[ii]).end());
	}
	number_of_distinct_tag_SNPs = all_tag_SNPs.size();
	int counter = 0;
	for (set<string>::iterator iter1 = all_tag_SNPs.begin(); iter1 != all_tag_SNPs.end(); iter1++ )
	{
		SNP_2_ids[*iter1] = counter;
		id_2_SNPs.push_back(*iter1);
		counter = counter + 1;
	}
	tag_SNPs = new vector<set<int> >(number_of_distinct_tag_SNPs);
	bins = new vector<set<int> >(bin_SNPs.size());
	for (int ii = 0 ; ii < bin_SNPs.size(); ii++)
	{
		for (int jj = 0; jj < (bin_SNPs[ii]).size(); jj++ )
		{
			string tag_SNP_jj = (bin_SNPs[ii])[jj];
			int tag_SNP_jj_id = SNP_2_ids[tag_SNP_jj];
			(*tag_SNPs)[tag_SNP_jj_id].insert(ii);
			((*bins)[ii]).insert(tag_SNP_jj_id);
		}
	}

};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Multi_Pop_Select::~Multi_Pop_Select()
{
	if (bins != NULL)
	{
		delete bins;
		bins = NULL;
	}
	if (tag_SNPs != NULL)
	{
		delete tag_SNPs;
		tag_SNPs = NULL;
	}
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int Multi_Pop_Select::find_min_covering_tag_SNPs(vector<string> & covering_tag_SNPs) 
{
	SCP_instance bin_covering_instance(bins, tag_SNPs);
	bins = NULL;
	tag_SNPs = NULL; // bin_covering_instance will take care of this instance
	int lower_bound = 0;
	int cost = 0;
	vector<int> covering_SNP_ids;
	bin_covering_instance.find_set_cover_greedy_no_preprocessing(lower_bound, cost, covering_SNP_ids);
	covering_tag_SNPs.clear();
	for (int ii = 0; ii < covering_SNP_ids.size(); ii++)
	{
		covering_tag_SNPs.push_back(id_2_SNPs[covering_SNP_ids[ii]]);
	}
	return covering_SNP_ids.size();
};
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////