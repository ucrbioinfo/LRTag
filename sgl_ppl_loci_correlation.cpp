/*
 *  sgl_ppl_loci_correlation.cpp
 *  MDS
 *
 *  Created by yonghui on 2/10/07.
 *  Copyright 2007 __MyCompanyName__. All rights reserved.
 *
 */

#include "sgl_ppl_loci_correlation.h"

sgl_ppl_loci_correlation::sgl_ppl_loci_correlation(float cut_off_threshold,float in_maf_cut_off)
{
	maf_cut_off = in_maf_cut_off;
	correlation_cut_off = cut_off_threshold;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


sgl_ppl_loci_correlation::sgl_ppl_loci_correlation()
{
	correlation_cut_off = 0;
	maf_cut_off = 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

pthread_mutex_t sgl_ppl_loci_correlation::mutex_cout= PTHREAD_MUTEX_INITIALIZER; //initialize the static mutex object
///////////////////////////////////////////////////////////////////////////////////////////////////

int sgl_ppl_loci_correlation::set_correlation_cut_off(float cut_off_threshold, float in_maf_cut_off)
{
	correlation_cut_off = cut_off_threshold;
	maf_cut_off = in_maf_cut_off;
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


int sgl_ppl_loci_correlation::clear()
{
	correlation_cut_off = 0;
	loci.clear();
	edges.clear();
	loci_maf_map.clear();
	return 0;
};

int sgl_ppl_loci_correlation::read_in_ppl_correlation_file(string file_path, string maf_file_path)
{
	ifstream maf_input_file(maf_file_path.c_str());
	if (maf_input_file.fail())
	{
		pthread_mutex_lock( &mutex_cout );
		cout << "file " << maf_file_path << " can not be opened" << endl;
		pthread_mutex_unlock( &mutex_cout );
		return -1;
	}
	string locus_snp;
	float locus_maf;
	while (! maf_input_file.eof())
	{
		maf_input_file >> locus_snp;
		maf_input_file >> locus_maf;
		loci_maf_map[locus_snp] = locus_maf;
		if (locus_maf >= maf_cut_off)
		{
			loci.insert(locus_snp);
		}
	}//end while
	maf_input_file.close();

	string first_locus;
	string second_locus;
	float correlation_coefficient;
	ifstream input_file(file_path.c_str());
	
	/*Make sure the file is opened successfully */
	if (input_file.fail())
	{
		pthread_mutex_lock( &mutex_cout );
		cout << "file " << file_path << " can not be opened" << endl;
		pthread_mutex_unlock( &mutex_cout );
		return -1;
	}//end if
	
	/*The last line may get read twice, but that is fine*/
	map<string, float>::iterator null_iterator = loci_maf_map.end();
	while (!input_file.eof())
	{
		input_file >> first_locus;
		input_file >> second_locus;
		input_file >> correlation_coefficient;
		
		//loci.insert(first_locus);
		//loci.insert(second_locus);
		map<string,float>::iterator iter1 = loci_maf_map.find(first_locus);
		map<string,float>::iterator iter2 = loci_maf_map.find(second_locus);
		if ((iter1 == null_iterator) or (iter2 == null_iterator))
		{
			pthread_mutex_lock( &mutex_cout );
			cout << "ERROR: " << first_locus << " or "<< second_locus<< "is not exist in the maf file" << endl;
			pthread_mutex_unlock( &mutex_cout );

		}
		else
		{
			float maf_1 = loci_maf_map[first_locus];
			float maf_2 = loci_maf_map[second_locus];

			/*Do the following only if the correlation coefficient is higher than the threshold*/
			if ((correlation_coefficient >= correlation_cut_off) and (maf_1 >= maf_cut_off) and (maf_2 >= maf_cut_off) )
			{
				edges.insert(edge_type(first_locus,second_locus));
			}//end of if
		}
		
	} // end of while

	/*Print out some statistics */
	pthread_mutex_lock( &mutex_cout );
	cout << "read in " << file_path << " with threshold value:" << correlation_cut_off << endl;
	cout << "the total number of loci:" << loci.size() << endl;
	cout << "the total number of edges:" << edges.size() << endl;
	pthread_mutex_unlock( &mutex_cout );
	
	input_file.close();
	
	/* Return 0 on success */
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

void* sgl_ppl_loci_correlation::thread_func_read_in_ppl_correlation_file(void* paras)
{
	sgl_ppl_loci_correlation* this_ppl =((thread_initialization_para*)paras)->ppl;

	string file_path = ((thread_initialization_para*)paras)->file_path;
	string maf_file_path = ((thread_initialization_para*)paras)->maf_file_path;
	this_ppl->read_in_ppl_correlation_file(file_path,maf_file_path);
	
	return NULL;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


set<string>* sgl_ppl_loci_correlation::get_loci()
{
	return &(loci);
};
////////////////////////////////////////////////////////////////////////////////////////////////////

set<edge_type>* sgl_ppl_loci_correlation::get_edges()
{
	return &(edges);
};
////////////////////////////////////////////////////////////////////////////////////////////////////

graph_linked_list::graph_linked_list(int in_total_SNPs)
{
	total_SNPs = in_total_SNPs;
	own_SNPs = 0;
	vertices = new bool[total_SNPs];
	for (int ii = 0 ; ii < total_SNPs; ii++)
	{
		vertices[ii] = false;
	}
	edges = new vector<int>[total_SNPs];
};
////////////////////////////////////////////////////////////////////////////////////////////////////


int graph_linked_list::set_own_SNPs(int in_own_SNPs)
{
	own_SNPs = in_own_SNPs;
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


graph_linked_list::graph_linked_list()
{
	own_SNPs = 0;
	total_SNPs = 0;
	vertices = NULL;
	edges = NULL;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


int graph_linked_list::initialize(int in_total_SNPs)
{
	if (vertices != NULL)
	{
		delete[] vertices;
		vertices = NULL;
	}
	if (edges != NULL)
	{
		delete[] edges;
		edges = NULL;
	}
	total_SNPs = in_total_SNPs;
	vertices = new bool[total_SNPs];
	for (int ii = 0 ; ii < total_SNPs; ii++)
	{
		vertices[ii] = false;
	}
	edges = new vector<int>[total_SNPs];
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


graph_linked_list::~graph_linked_list()
{
	/*release memory*/
	if (vertices != NULL)
	{
		delete[] vertices;
		vertices = NULL;
	}
	if (edges != NULL)
	{
		delete[] edges;
		edges = NULL;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////


multi_ppi_loci_correlation::multi_ppi_loci_correlation(vector<sgl_ppl_loci_correlation> &populations)
{
	/*Take the union of the SNP sites that are found in all the populations*/
	number_of_ppls = populations.size();
	
	for (int ii = 0; ii < populations.size(); ii++)
	{
		sgl_ppl_loci_correlation &current_ppl(populations[ii]); //define an alias to the populations[ii] object
		loci.insert((current_ppl.get_loci())->begin(), (current_ppl.get_loci())->end());
	}
	
	/*Build the string id to integer id mapping and the integer id to string id mapping*/
	set<string>::iterator ii;
	ii = loci.begin();
	int counter = 0;
	while( ii != loci.end())	
	{
		SNP_2_id[*ii] = counter;
		id_2_SNP[counter] = *ii;
		counter++; ii++;
	};
	
	/*Set the total_number_of_loci member*/
	total_number_of_loci = counter;
	
	/*Construct the linked list graph for each population*/
	int total_number_of_populations = populations.size();
	constituent_graphs = new graph_linked_list[total_number_of_populations];
	for (int ii = 0 ; ii < total_number_of_populations; ii++)
	{
		constituent_graphs[ii].initialize(total_number_of_loci);
	}
	
	for (int ii = 0 ; ii < total_number_of_populations ; ii++)
	{
		/*Give short names to the two objects*/
		sgl_ppl_loci_correlation &aaa(populations[ii]); 
		graph_linked_list &bbb(constituent_graphs[ii]);
		
		set<string>* aaa_loci_ptr = aaa.get_loci();		
		bbb.set_own_SNPs(aaa_loci_ptr->size());
		
		/*construct the bbb.vertices vector*/
		set<string>::iterator iterator1;
		iterator1 = aaa_loci_ptr->begin();
		while (iterator1 != aaa_loci_ptr->end())
		{
			int int_id = SNP_2_id[*iterator1];
			bbb.vertices[int_id] = true;
			iterator1 ++;
		}
		
	
		/*construct edges linked list*/
		set<edge_type>::iterator iterator2;
		set<edge_type>* aaa_edges_ptr = aaa.get_edges();
		iterator2 = aaa_edges_ptr->begin();
		while(iterator2 != aaa_edges_ptr->end())
		{
			int id1 = SNP_2_id[iterator2->first];
			int id2 = SNP_2_id[iterator2->second];
			if ((bbb.vertices[id1] == false) or (bbb.vertices[id2] == false))
			{
				cout << "error: " << iterator2->first <<" does not exist !";
			}
			bbb.edges[id1].push_back(id2);
			bbb.edges[id2].push_back(id1);
			iterator2++;
		}//end while
		
		/*remove possible duplicates*/
		for (int ii = 0  ; ii < total_number_of_loci ; ii++)	
		{
			vector<int> &current_linked_list(bbb.edges[ii]);
			
			/*Make a copy of the current linked list*/
			vector<int> tmp_linked_list = bbb.edges[ii];
			sort(tmp_linked_list.begin(), tmp_linked_list.end());
			current_linked_list.resize(0);
			if (tmp_linked_list.size() > 0)
			{
				current_linked_list.push_back(tmp_linked_list[0]);
			}

			for (int ii = 1 ; ii < tmp_linked_list.size() ; ii++)
			{
				if (tmp_linked_list[ii] != tmp_linked_list[ii-1])
				{
					current_linked_list.push_back(tmp_linked_list[ii]);
				}
			}//end for
		}//end for
	}//end for
	
	/*The last step is to allocate and construct the in_cover and not_in_cover vectors*/
	in_cover = new bool[total_number_of_loci];
	/*initialize the above two vectors*/
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		in_cover[ii] = false;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////



multi_ppi_loci_correlation::~multi_ppi_loci_correlation()
{
	if (constituent_graphs != NULL)
	{
		delete[] constituent_graphs;
		constituent_graphs = NULL;
	}
	if (in_cover != NULL)
	{
		delete[] in_cover;
		in_cover = NULL;
	}
};
////////////////////////////////////////////////////////////////////////////////////////////////////


int multi_ppi_loci_correlation::dump_statistics(bool verbose)
{
	cout<< "----------------printing out statistics" << endl;
	cout<< "The total number of populations: " << number_of_ppls << endl;
	cout<< "The total number of unique SNP sites: " << total_number_of_loci << endl;
	for (int ii=1 ; ii <= number_of_ppls; ii++)
	{
		graph_linked_list &current_graph(constituent_graphs[ii-1]);
		cout<< "Statistics for population " << ii << endl;
		cout<< "    Total number of SNP sites:" << current_graph.own_SNPs << endl;
		int total_number_of_edges = 0;
		for (int ii=0; ii < total_number_of_loci; ii++)
		{
			total_number_of_edges = total_number_of_edges + current_graph.edges[ii].size();
		}
		cout << "    Total number of edges:" << total_number_of_edges << endl;
		
		if(verbose)
		{
			/*print out the graph*/
			for (int ii = 0 ; ii < total_number_of_loci; ii ++)
			{
				if (current_graph.vertices[ii] == true)
				{
					cout << "      " << id_2_SNP[ii] << ":\t" ;
					vector<int>::iterator tmp_iterator;
					tmp_iterator = current_graph.edges[ii].begin();
					while (tmp_iterator != current_graph.edges[ii].end())
					{
						cout << id_2_SNP[*tmp_iterator] <<"\t ";
						tmp_iterator++;
					}
					cout << endl;
				}//end if
			}//end for 
		}
	}//end for 
	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////


int multi_ppi_loci_correlation::connected_components( vector<int>   **cnntd_cmpnts, int &number_of_cnntd_cmpnts)
{

	/*Clear components in case it is not empty*/
	vector<vector<int> > cnntd_components;	
	
	/*Run modified BFS to compute the connected components*/
	vector<bool> is_visited(total_number_of_loci,false);
	
	int component_id = 0;
	for (int ii = 0 ; ii < total_number_of_loci; ii++)
	{
		/*a new component*/
		if (is_visited[ii]==false)
		{
			cnntd_components.push_back(vector<int>());
			/*Construct a queue*/
			queue<int> active_nodes;
			active_nodes.push(ii);
			is_visited[ii] = true;
			while(not active_nodes.empty())
			{
				/*Pop out the header of the queue and put it to the corresponding component*/
				int tmp_node = active_nodes.front();
				active_nodes.pop();
				cnntd_components[component_id].push_back(tmp_node);
				/*Explore the neighbors of tmp_node, and push them to the queue if unvisited*/
				for (int jj = 0; jj < number_of_ppls; jj++ ) //do for each population
				{
					vector<int> &one_hop_neighbors(constituent_graphs[jj].edges[tmp_node]);
					int tmp_number_of_edges = one_hop_neighbors.size();
					for (int kk = 0 ; kk < tmp_number_of_edges; kk++)
					{
						int current_neighbor = one_hop_neighbors[kk];
						if (is_visited[current_neighbor] == false)
						{
							active_nodes.push(current_neighbor);
							is_visited[current_neighbor] = true;
						}
					}
				}//end for 
			}//end while
			
			/*Increase the component_id by 1*/
			component_id ++;
		}//end if
	}//end for
	
	/*Print out some statistics*/
	cout << "The total number of connected components: " << component_id << endl;
	cout << "The size of each connected components: " << endl;
	
	int total_nodes_in_components = 0 ;
	for (int ii = 0 ; ii < cnntd_components.size(); ii ++)
	{
		total_nodes_in_components = total_nodes_in_components + cnntd_components[ii].size();
		cout  << cnntd_components[ii].size() << ", ";
	}
	cout << endl;
	if (total_nodes_in_components != total_number_of_loci)
	{
		cout << "ERROR: total number of vertices in all components does not match the total number of loci." << endl ; 
	}
	
	
	/*prepare for the output objects*/
	if ((*cnntd_cmpnts) != NULL)
	{
		delete[] (*cnntd_cmpnts);
		(*cnntd_cmpnts) = NULL;
	}
	(*cnntd_cmpnts) = new vector<int>[cnntd_components.size()];
	
	for (int ii = 0 ; ii < cnntd_components.size() ; ii++ )
	{
		(*cnntd_cmpnts)[ii] = cnntd_components[ii];
	}
	
	number_of_cnntd_cmpnts = cnntd_components.size();
	cnntd_components.clear();
	/*On success, return 0*/
	return 0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////


int multi_ppi_loci_correlation::construct_SCP_instance(SCP_instance ** in_scp_instance, vector<int> &cnntd_component)
{
	if ((*in_scp_instance) != NULL)
	{
		delete (*in_scp_instance);
		(*in_scp_instance) = NULL;
	}
	/*The rows and columns of the set cover problem*/
	vector<set<int> > * rows = NULL; //rows are the elements
	vector<set<int> > * columns = NULL; //columns are the sets
	rows = new vector<set<int> >;
	columns = new vector<set<int> > ;

	int total_number_of_columns = cnntd_component.size();

	/*A mapping of the id from the origial space to the new reduced space*/
	map<int,int> id_2_id;
	for (int ii = 0 ; ii < total_number_of_columns; ii++)
	{
		id_2_id[cnntd_component[ii]] = ii;
	}
	
	/*construct the rows and columns objects and make the consistency checking.*/
	columns->resize(total_number_of_columns);
	int row_id = 0;
	for (int ii = 0; ii < number_of_ppls; ii ++ )
	{
		for (int jj = 0 ; jj < total_number_of_columns; jj++)
		{
			int current_loci = cnntd_component[jj];
			
			/*if current_loci is a population of population ii*/
			if (constituent_graphs[ii].vertices[current_loci])
			{
				rows->push_back(set<int>() ); //add one more row into rows
				(*rows)[row_id].insert(id_2_id[current_loci]);
				((*columns)[id_2_id[current_loci]]).insert(row_id);
				vector<int> &current_edges(constituent_graphs[ii].edges[current_loci]);
				map<int,int>::iterator iterator1;
				map<int,int>::iterator end_iterator = id_2_id.end();
				for ( int kk = 0; kk < current_edges.size(); kk++)
				{
					iterator1 = id_2_id.find(current_edges[kk]);
					if (iterator1 == end_iterator)
					{
						cout << "ERROR, the connected component is not self contained" << endl;
					}
					else
					{
						(*rows)[row_id].insert(iterator1->second);
						((*columns)[iterator1->second]).insert(row_id);						
					}
				}//end for
				row_id ++;
			}
		}
	}
	
	(*in_scp_instance) = new SCP_instance(rows, columns);
	rows = NULL; 
	columns = NULL;
	return 0;
	
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


string multi_ppi_loci_correlation::int_id_2_SNP_lable(int int_id)
{
	map<int,string>::iterator iterator1;
	iterator1 = id_2_SNP.find(int_id);
	if (iterator1 == id_2_SNP.end())
	{
		cout << "string multi_ppi_loci_correlation::int_id_2_SNP_lable(int int_id)" << endl;
		cout << "ERROR! id not found" << endl;
		return "ERROR";
	}
	else
	{
		return iterator1->second;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

