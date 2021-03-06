#include <iostream>
#include "pcst_fast.h"
#include "priority_queue.h"
#include "Graph.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>
#include <math.h>
#include <iostream>
#include <fstream>
#include <time.h>

using cluster_approx::PCSTFast;
using std::make_pair;
using std::vector;
using std:: ofstream;
using std:: endl;
using std:: cout;


PCSTFast::Statistics::Statistics() : total_num_edge_events(0),
                                     num_deleted_edge_events(0),
                                     num_merged_edge_events(0),
                                     total_num_merge_events(0),
                                     num_active_active_merge_events(0),
                                     num_active_inactive_merge_events(0),
                                     total_num_edge_growth_events(0),
                                     num_active_active_edge_growth_events(0),
                                     num_active_inactive_edge_growth_events(0),
                                     num_cluster_events(0) { };


PCSTFast::PruningMethod PCSTFast::parse_pruning_method(
        const std::string& input) {
    PruningMethod result = kUnknownPruning;
    std::string input_lower(' ', input.size());
    for (size_t ii = 0; ii < input.size(); ++ii) {
        input_lower[ii] = tolower(input[ii]);
    }

    if (input == "none") {
        result = kNoPruning;
    } else if (input == "simple") {
        result = kSimplePruning;
    } else if (input == "gw") {
        result = kGWPruning;
    } else if (input == "strong") {
        result = kStrongPruning;
    }

    return result;
}

PCSTFast::PCSTFast(const std::vector<std::pair<std::pair<long, long>, long> >& edges_,
                   const std::vector<double>& prizes_,
                   const std::set<int>& terminals_,
                   int root_,
                   int target_num_active_clusters_,
                   PruningMethod pruning_,
                   int verbosity_level_,
                   void (*output_function_)(const char*),
                   const std::map<long,std::vector<int>> edgeAttributeMap_,
                   const std::map<long, std::vector<double>> nodeAttrMap_,
                   const std::vector<cluster_approx::Graph::AttributeInfo> attribute_stat_)
        : edges(edges_), prizes(prizes_),
          terminals(terminals_),
          root(root_),
          target_num_active_clusters(target_num_active_clusters_),
          pruning(pruning_), verbosity_level(verbosity_level_),
          output_function(output_function_),
          edgeAttributeMap(edgeAttributeMap_),
          nodeAttrMap(nodeAttrMap_),
          attribute_stat(attribute_stat_){//Store all arguments into a this structure
    //this includes : edges,prizes,costs,root,target_num_active_clusters,pruning,verbosity_level,output_function
    //this also includes : cluster_approx::PCSTFast::Statistics
    //pairing_heap_buffer
    //edge_parts, each edge_part has next_event_val, deleted (default is false), heap_node
    //node_deleted, each node-deleted has a label to indicate the node is deleted or not
    //edge_info, each edge_info has inactive_merge_event

    clock_t pcst_ini_startTime, pcst_ini_endTime;
    pcst_ini_startTime = clock();
    edge_parts.resize(2 * edges.size());  //divides each edge into 2 parts,stored in the edge_parts
    node_deleted.resize(prizes.size(), false); //assignes each vertex a false label at first
    //std::cout << "edges size is" <<edges.size()<<std::endl;
    edge_info.resize(edges_.size());  //Initialize each edge with its inactive_merge_event

    //Assign each inactive_merge_event with the value -1
    for (size_t ii = 0; ii < edge_info.size(); ++ii) {
        edge_info[ii].inactive_merge_event = -1;
    }
    //set current_time is 0.
    current_time = 0.0;
    // TODO: set to min input value / 2.0?
    eps = 1e-6;
    //static_cast valid and safe conversion in classic C
    for (int ii = 0; ii < static_cast<int>(prizes.size()); ++ii) {//Go through each node with positive prize
        if (prizes[ii] < 0.0) {
            throw std::invalid_argument("Prize negative.");  //Prize negative currently will output error message
        }
        clusters.push_back(
                Cluster(&pairing_heap_buffer));//Push_back one PairingHeap into the Cluster and assign the value of edge_parts as pointer &pairing_heap_buffer,  the vertex prize and index is
        // stored in the minHeap
        //Each clusters[ii] has active, active_start_time, active_end_time, merged_into, prize_sum, subcluster_moat_sum,moat,contains_root,skip_up,skip_up_sum,merged_along,child_cluster_1,
        //child_cluster_2,necessary value
        //push_back Adds a new element at the end of the vector, after its current last element. The content of val is copied (or moved) to the new element.
        //only works for prize[ii]!= 0,mark the clusters[ii] as active
        if (prizes[ii] != 0) {
            clusters[ii].active = (ii != root);//Assign the cluster[ii] as true
            clusters[ii].active_start_time = 0.0;//Assign the cluster[ii] active_start_time
            clusters[ii].active_end_time = -1.0;//Assign the cluster[ii] active_end_time
            if (ii == root) {
                clusters[ii].active_end_time = 0.0;
            }
            clusters[ii].merged_into = -1; //Assign the cluster[ii] merged_into information as -1
            clusters[ii].prize_sum = prizes[ii]; //The clusters[ii]'s prize_sum value is initialized as the corresponding vertex's prize added
            clusters[ii].subcluster_moat_sum = 0.0;//Initialize the cluster[ii] subcluster_moat_sum
            clusters[ii].moat = 0.0;//Initialize the cluster[ii] moat
            clusters[ii].contains_root = (ii == root);//Cluster[ii] contains root always is false in this case
            clusters[ii].skip_up = -1;//Assign the cluster[ii] skip_up information as -1
            clusters[ii].skip_up_sum = 0.0;//Assign the cluster[ii] skip_up_sum information as 0
            clusters[ii].merged_along = -1;//Assign the cluster[ii] merged_along information as 0
            clusters[ii].child_cluster_1 = -1;//Assign the cluster[ii] child_cluster_1 information as -1
            clusters[ii].child_cluster_2 = -1;//Assign the cluster[ii] child_cluster_2 information as -1
            clusters[ii].necessary = false;//Assign the cluster[ii] necessary information as -1
            clusters[ii].edge_compute_flag = false;//Assign the cluster[ii] edge_compute_flag as false
            clusters[ii].vertex_list.push_back(ii);
            //printf("cluster %d has vertex %d \n",ii, clusters[ii].vertex_list.front());

        }else{
            clusters[ii].active =false;
            clusters[ii].active_start_time = 0.0;
            clusters[ii].active_end_time = 0.0;
            clusters[ii].merged_into = -1; //Assign the cluster[ii] merged_into information as -1
            clusters[ii].prize_sum = prizes[ii]; //The clusters[ii]'s prize_sum value is initialized as the corresponding vertex's prize added
            clusters[ii].subcluster_moat_sum = 0.0;//Initialize the cluster[ii] subcluster_moat_sum
            clusters[ii].moat = 0.0;//Initialize the cluster[ii] moat
            clusters[ii].contains_root = (ii == root);//Cluster[ii] contains root always is false in this case
            clusters[ii].skip_up = -1;//Assign the cluster[ii] skip_up information as -1
            clusters[ii].skip_up_sum = 0.0;//Assign the cluster[ii] skip_up_sum information as 0
            clusters[ii].merged_along = -1;//Assign the cluster[ii] merged_along information as 0
            clusters[ii].child_cluster_1 = -1;//Assign the cluster[ii] child_cluster_1 information as -1
            clusters[ii].child_cluster_2 = -1;//Assign the cluster[ii] child_cluster_2 information as -1
            clusters[ii].necessary = false;//Assign the cluster[ii] necessary information as -1
            clusters[ii].edge_compute_flag = false; //Assign the cluster[ii] edge_compute_flag as false
            clusters[ii].vertex_list.push_back(ii);
            //printf("cluster %d has vertex %d \n",ii, clusters[ii].vertex_list.front());
        }

        if (clusters[ii].active) {
            clusters_deactivation.insert(prizes[ii], ii);//Cluster_deactivation priority queue needs to insert vertex id and corresponding prize
            //clusters_deactivation maintains a sorted_set
            //a list of index_to_iterator that points to most updated vertex prize

            //test purpose
        }
    }



    for (int ii = 0; ii < static_cast<int>(edges.size()); ++ii) { //Go through each edge with two positive integer end-points
        int uu = edges[ii].first.first; //Get the edge left-end point
        int vv = edges[ii].first.second; //Get the edge right-end point
        if (uu < 0 || vv < 0) {
            throw std::invalid_argument("Edge endpoint negative."); //Edge endpoint cannot be negative
        }

        if (uu >= static_cast<int>(prizes.size())
            || vv >= static_cast<int>(prizes.size())) {
            throw std::invalid_argument("Edge endpoint out of range (too large)."); //Edge endpoint cannot go out of range
        }
        //give each edge a default cost, treated as we don't have the edge_cost computation result yet

        double cost = std::numeric_limits<double>::max();
        if (cost < 0.0) {
            throw std::invalid_argument("Edge cost negative.");//Edge cost cannot be negative
        }
        EdgePart& uu_part = edge_parts[2 * ii]; //Initialize Edge uu_part, points to edge_parts[2*ii]
        //correspondingly next_event_val, deleted (default is false), heap_node
        EdgePart& vv_part = edge_parts[2 * ii + 1]; //Initialize Edge vv_part
        Cluster& uu_cluster = clusters[uu]; //Assign the uu_cluster points to clusters[uu]
        Cluster& vv_cluster = clusters[vv]; //Assign the vv_cluster points to clusters[vv]

        uu_part.deleted = false; //Update uu_part deleted flag as false
        vv_part.deleted = false; //Update vv_part deleted flag as false

        uu_part.edge_compute_flag = false; //Update uu_part edge_compute_flag as false
        vv_part.edge_compute_flag = false; //Update vv_part edge_compute_flag as false



        //Treat all uu_cluster equally
        double event_time = cost / 2.0;   //event_time equals cost/2
        uu_part.next_event_val = event_time; //uu_part.next_event_val = event_time
        vv_part.next_event_val = event_time;



        // current_time = 0, so the next event time for each edge is the
        // same as the next_event_val
        uu_part.heap_node = uu_cluster.edge_parts.insert(
                uu_part.next_event_val, 2 * ii);
        //update uu_part heap_node, the value equals to next_event_val, payload equals to 2*ii, payload is edge_parts value
        vv_part.heap_node = vv_cluster.edge_parts.insert(
                vv_part.next_event_val, 2 * ii + 1);
        //update uu_part heap_node, the value equals to next_event_val, payload equals to 2*ii+1, payload is edge_parts value
    }

    /**  test paringHeap in the cluster structure
    for (int ii =0; ii< static_cast<int>(prizes.size());++ii){
        if(clusters[ii].active){
            while(!clusters[ii].edge_parts.is_empty()){
                double val;
                int edge_part;
                clusters[ii].edge_parts.delete_min(&val, &edge_part);
                std::cout<<"edge_part value is"<<val<<std::endl;
                std::cout<<"edge_part id is"<<edge_part<<std::endl;
            }
        }
    }

    **/



    //clusters_next_edge_event doesn't need to be initialized here since the edge cost is dummy value
    /*
    for (int ii = 0; ii < static_cast<int>(prizes.size()); ++ii) {
        if (clusters[ii].active) {
            if (!clusters[ii].edge_parts.is_empty()) {
                double val;
                int edge_part;
                clusters[ii].edge_parts.get_min(&val, &edge_part);
                clusters_next_edge_event.insert(val, ii); //assign clusters_next_edge_event clusters ID, edge_parts get_min
            }
        }
    }
    */
    //std::cout<<"Start printing Clusters_next_edge_event: \n";
    pcst_ini_endTime = clock();
    cout<<"Total Initialization time:" <<(double)(pcst_ini_endTime - pcst_ini_startTime)/CLOCKS_PER_SEC<<"s"<<endl;
}

void PCSTFast::get_next_edge_event(double* next_time,
                                   int* next_cluster_index,
                                   int* next_edge_part_index) {
    if (clusters_next_edge_event.is_empty()) {//clusters_next_edge_event has clusters ID and edge_parts get_min as a sorted set
        *next_time = std::numeric_limits<double>::infinity();
        *next_cluster_index = -1;
        *next_edge_part_index = -1;
        return;
    }

    clusters_next_edge_event.get_min(next_time, next_cluster_index);
    clusters[*next_cluster_index].edge_parts.get_min(next_time,
                                                     next_edge_part_index);
}

void PCSTFast::remove_next_edge_event(int next_cluster_index) {
    clusters_next_edge_event.delete_element(next_cluster_index);//clusters_next_edge_event deletes the current smallest edge part and cluster pair
    double tmp_value;
    int tmp_edge_part;
    clusters[next_cluster_index].edge_parts.delete_min(&tmp_value,
                                                       &tmp_edge_part);//delete corresponding edge_parts in the Cluster min-heap tree
    if (!clusters[next_cluster_index].edge_parts.is_empty()) {
        clusters[next_cluster_index].edge_parts.get_min(&tmp_value, &tmp_edge_part);
        clusters_next_edge_event.insert(tmp_value, next_cluster_index);
    }
}

void PCSTFast::get_next_cluster_event(double* next_time,
                                      int* next_cluster_index) {
    if (clusters_deactivation.is_empty()) {
        *next_time = std::numeric_limits<double>::infinity();
        *next_cluster_index = -1;
        return;
    }

    clusters_deactivation.get_min(next_time, next_cluster_index);
}

void PCSTFast::remove_next_cluster_event() {
    double tmp_value;
    int tmp_cluster;
    clusters_deactivation.delete_min(&tmp_value, &tmp_cluster);
}

void PCSTFast::get_sum_on_edge_part(int edge_part_index,
                                    double* total_sum,
                                    double* finished_moat_sum,
                                    int* cur_cluster_index) {
    int endpoint = edges[edge_part_index / 2].first.first;
    if (edge_part_index % 2 == 1) {
        endpoint = edges[edge_part_index / 2].first.second;
    }

    *total_sum = 0.0;
    *cur_cluster_index = endpoint;
    path_compression_visited.resize(0);

    while (clusters[*cur_cluster_index].merged_into != -1) {
        path_compression_visited.push_back(make_pair(*cur_cluster_index,
                                                     *total_sum));
        if (clusters[*cur_cluster_index].skip_up >= 0) {
            *total_sum += clusters[*cur_cluster_index].skip_up_sum;
            *cur_cluster_index = clusters[*cur_cluster_index].skip_up;
        } else {
            *total_sum += clusters[*cur_cluster_index].moat;
            *cur_cluster_index = clusters[*cur_cluster_index].merged_into;
        }
    }

    for (int ii = 0; ii < static_cast<int>(path_compression_visited.size());
         ++ii) {
        int visited_cluster_index = path_compression_visited[ii].first;
        double visited_sum = path_compression_visited[ii].second;
        clusters[visited_cluster_index].skip_up = *cur_cluster_index;
        clusters[visited_cluster_index].skip_up_sum = *total_sum - visited_sum;
    }

    if (clusters[*cur_cluster_index].active) {
        *finished_moat_sum = *total_sum;
        *total_sum += current_time - clusters[*cur_cluster_index].active_start_time;
    } else {
        *total_sum += clusters[*cur_cluster_index].moat;
        *finished_moat_sum = *total_sum;
    }
}

void PCSTFast::mark_nodes_as_good(int start_cluster_index) {
    cluster_queue.resize(0);
    int queue_index = 0;
    cluster_queue.push_back(start_cluster_index);
    while (queue_index < static_cast<int>(cluster_queue.size())) {
        int cur_cluster_index = cluster_queue[queue_index];
        queue_index += 1;
        if (clusters[cur_cluster_index].merged_along >= 0) {
            cluster_queue.push_back(clusters[cur_cluster_index].child_cluster_1);
            cluster_queue.push_back(clusters[cur_cluster_index].child_cluster_2);
        } else {
            node_good[cur_cluster_index] = true;
        }
    }
}

void PCSTFast::mark_clusters_as_necessary(int start_cluster_index) {
    int cur_cluster_index = start_cluster_index;
    while (!clusters[cur_cluster_index].necessary) {
        clusters[cur_cluster_index].necessary = true;
        if (clusters[cur_cluster_index].merged_into >= 0) {
            cur_cluster_index = clusters[cur_cluster_index].merged_into;
        } else {
            return;
        }
    }
}

void PCSTFast::mark_nodes_as_deleted(int start_node_index,
                                     int parent_node_index) {
    node_deleted[start_node_index] = true;
    cluster_queue.resize(0);
    int queue_index = 0;
    cluster_queue.push_back(start_node_index);
    while (queue_index < static_cast<int>(cluster_queue.size())) {
        int cur_node_index = cluster_queue[queue_index];
        queue_index += 1;
        for (int ii = 0;
             ii < static_cast<int>(phase3_neighbors[cur_node_index].size());
             ++ii) {
            int next_node_index = phase3_neighbors[cur_node_index][ii].first;
            if (next_node_index == parent_node_index) {
                continue;
            }
            if (node_deleted[next_node_index]) {
                // should never happen
                continue;
            }
            node_deleted[next_node_index] = true;
            cluster_queue.push_back(next_node_index);
        }
    }
}

bool PCSTFast::run(std::vector<int>* result_nodes,
                   std::vector<int>* result_edges,
                   int attribute_index_number){
    //clock_t pcst_startTime, pcst_endTime;
    //pcst_startTime = clock();
    struct timespec pcst_startTime, pcst_endTime;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &pcst_startTime);
    result_nodes->clear();
    result_edges->clear();

    double costs [edges.size()];
    //store the attribute_index
    for (int i = 0; i < attribute_index_number; ++i) {
        attribute_index.push_back(i + 1);
    }


    //////////////////////////////////////////
    if (root >= 0 && target_num_active_clusters > 0) {
        snprintf(output_buffer, kOutputBufferSize,
                 "Error: target_num_active_clusters must be 0 in the rooted case.\n");
        output_function(output_buffer);
        return false;
    }
    //////////////////////////////////////////

    vector<int> phase1_result;//Initialize phase1_result
    int num_active_clusters = 0;
    vector<int> activeClustersList;

    //std::vector<int>* iterator1;
    for(int ii= 0; ii < static_cast<int>(prizes.size()); ++ii){
        if(prizes[ii]>0.0){
            //printf("i is %d \n",ii);
            num_active_clusters++;
            activeClustersList.push_back(ii);
        }
    }

    /*
    for (std::set<int>::iterator it = activeClustersList.begin() ; it != activeClustersList.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << '\n';
    */


    //int num_active_clusters = prizes.size();
    if (root >= 0) {
        num_active_clusters -= 1;
    }


    //Initialize the first_round related edges
    for (std::vector<int>::iterator it = activeClustersList.begin() ; it != activeClustersList.end(); ++it){
        if ((clusters[*it].active)&&(clusters[*it].edge_compute_flag==false)) { //only considers active cluster[ii] and edge_compute_flag as false

            //generate a queue to store all <val,edge_part_index> pairs
            std::vector<std::pair<double,int>> temp_pair;
            while (!clusters[*it].edge_parts.is_empty()) {
                double edge_val;
                double val;
                int edge_part_index;
                clusters[*it].edge_parts.delete_min(&val, &edge_part_index);
                //Compute this edge_part's real value and update clusters[*it]
                //printf("the edge_part_index is %d",edge_part_index);
                EdgePart& ww_part = edge_parts[edge_part_index];
		if(ww_part.edge_compute_flag == false){
                if (edge_part_index % 2 == 0) {

                    //PriorityQueueType multi_edge_event;
                    //printf("The origin edge id is %ld \n", edges[edge_part_index / 2].second);
                    //Map this origin edge to multiple edges and only keep the shortest edge
                    long sourceId = edges[edge_part_index / 2].first.first;
                    long sinkId = edges[edge_part_index / 2].first.second;
                    long origin_edge_id = edges[edge_part_index / 2].second;
                    //multiEdges.clear();
                    edge_compute_flag =false;
                    cost_min = std::numeric_limits<double>::max();
                    std::vector<double> sourceNodeAttr = nodeAttrMap[edges[edge_part_index / 2].first.first];
                    std::vector<double> sinkNodeAttr = nodeAttrMap[edges[edge_part_index / 2].first.second];
                    for(int k=1; k<=attribute_index_number;++k){
                        combCalculation(0,k,sourceId,sinkId,sourceNodeAttr, sinkNodeAttr,attribute_index, origin_edge_id, cost_min);
                    }

                    edge_val = cost_min;
                    costs[edge_part_index / 2] = edge_val;
                    //printf("The edge id %d, with cost %f \n",edge_part_index / 2, costs[edge_part_index / 2]);
                    edge_compute_count = edge_compute_count+ 1;
                    //printf("edgeid is %d, weight is %f \n", edge_part_index / 2, val);

                } else {
                    //PriorityQueueType multi_edge_event;
                    //printf("The origin edge id is %ld \n", edges[(edge_part_index-1) / 2].second);
                    //Map this origin edge to multiple edges and only keep the shortest edge
                    long sourceId = edges[(edge_part_index-1) / 2].first.first;
                    long sinkId = edges[(edge_part_index-1) / 2].first.second;
                    long origin_edge_id = edges[(edge_part_index-1) / 2].second;
                    //multiEdges.clear();
                    edge_compute_flag =false;
                    cost_min = std::numeric_limits<double>::max();
                    std::vector<double> sourceNodeAttr = nodeAttrMap[edges[(edge_part_index-1) / 2].first.first];
                    std::vector<double> sinkNodeAttr = nodeAttrMap[edges[(edge_part_index-1) / 2].first.second];
                    for(int k=1; k<=attribute_index_number;++k){
                        combCalculation(0,k,sourceId,sinkId,sourceNodeAttr, sinkNodeAttr,attribute_index, origin_edge_id, cost_min);
                    }
                    edge_val = cost_min;
                    costs[(edge_part_index-1) / 2] = edge_val;
                    //printf("The edge id %d, with cost %f \n",(edge_part_index-1) / 2, costs[(edge_part_index-1) / 2]);
                    edge_compute_count = edge_compute_count+ 1;
                    //printf("edgeid is %d, weight is %f \n", (edge_part_index-1)/ 2, val);
                }
                temp_pair.push_back(std::make_pair(edge_val, edge_part_index));
		}else{
		temp_pair.push_back(std::make_pair(val, edge_part_index));
		}

            }
            std::vector<std::pair<double, int> >::iterator it_temp;
            for (it_temp = temp_pair.end()-1; it_temp != temp_pair.begin()-1; it_temp--) {
                //std::cout<<"\n";
                double val= it_temp->first;
                int edge_part_index= it_temp->second;
                //printf("the edge_part index is %d", edge_part_index);
                EdgePart& uu_part = edge_parts[edge_part_index];
                int the_other_edge_part_index = get_other_edge_part_index(
                        edge_part_index);

                EdgePart& vv_part = edge_parts[the_other_edge_part_index];


                Cluster& uu_cluster = clusters[*it];

                //calculate the current the_other_cluster_index
                int endpoint = edges[the_other_edge_part_index / 2].first.first;
                if (the_other_edge_part_index % 2 == 1) {
                    endpoint = edges[the_other_edge_part_index / 2].first.second;
                }
                int the_other_cluster_index = endpoint;
                double total_sum = 0.0;
                path_compression_visited.resize(0);

                while (clusters[the_other_cluster_index].merged_into != -1) {
                    path_compression_visited.push_back(make_pair(the_other_cluster_index,
                                                                 total_sum));
                    if (clusters[the_other_cluster_index].skip_up >= 0) {
                        total_sum += clusters[the_other_cluster_index].skip_up_sum;
                        the_other_cluster_index = clusters[the_other_cluster_index].skip_up;
                    } else {
                        total_sum += clusters[the_other_cluster_index].moat;
                        the_other_cluster_index = clusters[the_other_cluster_index].merged_into;
                    }
                }

                Cluster& vv_cluster = clusters[the_other_cluster_index];

                double event_time = val / 2.0;   //event_time equals cost/2
                if(uu_part.edge_compute_flag==false){
                    uu_part.next_event_val = event_time;
                }
                if(vv_part.edge_compute_flag==false){
                    vv_part.next_event_val = event_time;
                }


                uu_part.heap_node = uu_cluster.edge_parts.insert(
                        uu_part.next_event_val, edge_part_index);

                if(vv_part.edge_compute_flag==false){
                    vv_cluster.edge_parts.decrease_key(
                            vv_part.heap_node,
                            (std::numeric_limits<double>::max()/2.0),
                            vv_part.next_event_val);//vv_part is the root as this next_event_val pairing heap
                }

                uu_part.edge_compute_flag = true;
                vv_part.edge_compute_flag = true;
            }
        }else {
                throw std::invalid_argument("Error: Something wrong with the activeClusterList");
                break;
            }

        clusters[*it].edge_compute_flag=true;
    }

    //std::cout<<"Intialize clusters_next_edge_event:"<<std::endl;
    for (std::vector<int>::iterator it = activeClustersList.begin() ; it != activeClustersList.end(); ++it){
        if ((clusters[*it].active)&&(clusters[*it].edge_compute_flag==true)) {
            if (!clusters[*it].edge_parts.is_empty()) {
                double val;
                int edge_part;
                clusters[*it].edge_parts.get_min(&val, &edge_part);
                //printf("the cluster is %d, min_value is %f, edge_part id is %d \n",*it,val,edge_part);
                clusters_next_edge_event.insert(val, *it); //assign clusters_next_edge_event clusters ID, edge_parts get_min
            }
        }
    }

   // clock_t pcst_startTime,pcst_endTime;
   // pcst_startTime = clock();
   // ofstream outfile;
   // outfile.open("output_the_cluster_merge_process.txt");

    while (num_active_clusters > target_num_active_clusters) {
        //////////////////////////////////////////
        if (verbosity_level >= 2) {    //verbosity_level by default is 0
            snprintf(output_buffer, kOutputBufferSize,
                     "-----------------------------------------\n");
            output_function(output_buffer);
        }
        //////////////////////////////////////////


        double next_edge_time;
        int next_edge_cluster_index;
        int next_edge_part_index = -1;
        get_next_edge_event(&next_edge_time,
                            &next_edge_cluster_index,
                            &next_edge_part_index);
       //printf("next_edge_time is %f, next_edge_part_index is %d, cluster is %d \n",next_edge_time, next_edge_part_index, next_edge_cluster_index);
       // outfile<<"next_edge_time is"<<next_edge_time<<", next_edge_part_index is"<<next_edge_part_index<<", cluster is"<<
              // next_edge_cluster_index<<std::endl;
        double next_cluster_time;
        int next_cluster_index;
        get_next_cluster_event(&next_cluster_time, &next_cluster_index);
        //printf("next_cluster_time is %f, next cluster is %d \n", next_cluster_time, next_cluster_index);
       // outfile<<"next_cluster_time is"<<next_cluster_time<<", next cluster is"<<next_edge_cluster_index<<std::endl;

        //////////////////////////////////////////
        if (verbosity_level >= 2) {
            snprintf(output_buffer, kOutputBufferSize,
                     "Next edge event: time %e, cluster %d, part %d\n",
                     next_edge_time, next_edge_cluster_index, next_edge_part_index);
            output_function(output_buffer);
            snprintf(output_buffer, kOutputBufferSize,
                     "Next cluster event: time %e, cluster %d\n",
                     next_cluster_time, next_cluster_index);
            output_function(output_buffer);
        }
        //////////////////////////////////////////

        if (next_edge_time < next_cluster_time) {
            stats.total_num_edge_events += 1;

            current_time = next_edge_time;
            remove_next_edge_event(next_edge_cluster_index);

            if (edge_parts[next_edge_part_index].deleted) {
                stats.num_deleted_edge_events += 1;

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Edge part %d already deleted, nothing to do\n",
                             next_edge_part_index);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                continue;
            }

            // collect all the relevant information about the edge parts
            int other_edge_part_index = get_other_edge_part_index(
                    next_edge_part_index);

            //calculate the current edge cost
            double current_edge_cost;

            EdgePart& ww_part =edge_parts[next_edge_part_index];
            EdgePart& tt_part =edge_parts[other_edge_part_index];
            //This if condition should never happen actually
            if (ww_part.edge_compute_flag==false){
                double val;
                //PriorityQueueType multi_edge_event;
                //printf("The origin edge id is %ld \n", edges[next_edge_part_index/ 2].second);
                //Map this origin edge to multiple edges and only keep the shortest edge
                long sourceId = edges[next_edge_part_index/ 2].first.first;
                long sinkId = edges[next_edge_part_index/ 2].first.second;
                long origin_edge_id = edges[next_edge_part_index/ 2].second;
                //multiEdges.clear();
                std::vector<double> sourceNodeAttr = nodeAttrMap[edges[next_edge_part_index/ 2].first.first];
                std::vector<double> sinkNodeAttr = nodeAttrMap[edges[next_edge_part_index/ 2].first.second];
                edge_compute_flag =false;
                cost_min = std::numeric_limits<double>::max();
                for(int k=1; k<=attribute_index_number;++k){
                    combCalculation(0,k,sourceId,sinkId,sourceNodeAttr, sinkNodeAttr,attribute_index, origin_edge_id, cost_min);
                }
                //int edge_id;
                //multi_edge_event.get_min(&val,&edge_id);
                /*
                if(edge_id != origin_edge_id ){
                    printf("something is wrong \n");
                    break;
                }
                */
                val = cost_min;
                current_edge_cost = val;

                //current_edge_cost = PCSTFast::computeEdgeWeight(sourceNodeAttr,sinkNodeAttr,attribute_stat,attribute_index_combination);
                edge_compute_count = edge_compute_count+ 1;
                costs[next_edge_part_index / 2] = current_edge_cost;
                //printf("The edge id %d, with cost %f \n",next_edge_part_index / 2, costs[ next_edge_part_index / 2]);
                ww_part.edge_compute_flag =true;
                tt_part.edge_compute_flag = true;

            } else{
                current_edge_cost = costs[next_edge_part_index/2];
            }

            double sum_current_edge_part;
            int current_cluster_index;
            double current_finished_moat_sum;
            get_sum_on_edge_part(next_edge_part_index,
                                 &sum_current_edge_part,
                                 &current_finished_moat_sum,
                                 &current_cluster_index);
            double sum_other_edge_part;
            int other_cluster_index;
            double other_finished_moat_sum;
            get_sum_on_edge_part(other_edge_part_index,
                                 &sum_other_edge_part,
                                 &other_finished_moat_sum,
                                 &other_cluster_index);

            double remainder = current_edge_cost
                               - sum_current_edge_part - sum_other_edge_part;

            Cluster& current_cluster = clusters[current_cluster_index];
            Cluster& other_cluster = clusters[other_cluster_index];
            EdgePart& next_edge_part = edge_parts[next_edge_part_index];
            EdgePart& other_edge_part = edge_parts[other_edge_part_index];

            //////////////////////////////////////////
            if (verbosity_level >= 2) {
                snprintf(output_buffer, kOutputBufferSize,
                         "Edge event at time %e, current edge part %d (cluster %d), "
                                 "other edge part %d (cluster %d)\n",
                         current_time, next_edge_part_index, current_cluster_index,
                         other_edge_part_index, other_cluster_index);
                output_function(output_buffer);
                snprintf(output_buffer, kOutputBufferSize,
                         "Sum current part %e, other part %e, total length %e, "
                                 "remainder %e\n",
                         sum_current_edge_part, sum_other_edge_part, current_edge_cost,
                         remainder);
                output_function(output_buffer);
            }
            //////////////////////////////////////////

            if (current_cluster_index == other_cluster_index) {
                stats.num_merged_edge_events += 1;

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Clusters already merged, ignoring edge\n");
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                edge_parts[other_edge_part_index].deleted = true;

                continue;
            }


            if (remainder < eps * current_edge_cost || remainder == 0.0) {
                stats.total_num_merge_events += 1;


                std::vector<std::pair<double,int>> temp_pair;
                //clock_t  computeEdgeBeginTime, computeEdgeEndTime;

                while (!other_cluster.edge_parts.is_empty()&&(other_cluster.edge_compute_flag==false)) {
                    double edge_val;
                    double val;
                    int edge_part_index;
                    other_cluster.edge_parts.delete_min(&val, &edge_part_index);
                    EdgePart& ww_part = edge_parts[edge_part_index];
                    if(ww_part.edge_compute_flag==false){
                        //Compute this edge_part's real value and update clusters[*it]
                        if (edge_part_index % 2 == 0) {
                            //PriorityQueueType multi_edge_event;
                            //printf("The origin edge id is %ld \n", edges[edge_part_index / 2].second);
                            //Map this origin edge to multiple edges and only keep the shortest edge
                            long sourceId = edges[edge_part_index / 2].first.first;
                            long sinkId = edges[edge_part_index / 2].first.second;
                            long origin_edge_id = edges[edge_part_index / 2].second;
                            //multiEdges.clear();
                            std::vector<double> sourceNodeAttr = nodeAttrMap[edges[edge_part_index / 2].first.first];
                            std::vector<double> sinkNodeAttr = nodeAttrMap[edges[edge_part_index / 2].first.second];
                            edge_compute_flag =false;
                            cost_min = std::numeric_limits<double>::max();
                            for(int k=1; k<=attribute_index_number;++k){
                                combCalculation(0,k,sourceId,sinkId,sourceNodeAttr, sinkNodeAttr,attribute_index, origin_edge_id, cost_min);
                            }
                            //int edge_id;
                            //multi_edge_event.get_min(&val,&edge_id);
                            /*
                            if(edge_id != origin_edge_id ){
                                printf("something is wrong \n");
                                break;
                            }
                            */
                            edge_val =cost_min;
                            costs[edge_part_index / 2] = edge_val;
                            //printf("The edge id %d, with cost %f \n",edge_part_index / 2, costs[edge_part_index / 2]);
                            edge_compute_count = edge_compute_count+ 1;

                        } else {
                            //PriorityQueueType multi_edge_event;
                            //printf("The origin edge id is %ld \n", edges[(edge_part_index-1) / 2].second);
                            //Map this origin edge to multiple edges and only keep the shortest edge
                            long sourceId = edges[(edge_part_index-1) / 2].first.first;
                            long sinkId = edges[(edge_part_index-1) / 2].first.second;
                            long origin_edge_id = edges[(edge_part_index-1) / 2].second;
                            //multiEdges.clear();
                            std::vector<double> sourceNodeAttr = nodeAttrMap[edges[(edge_part_index-1) / 2].first.first];
                            std::vector<double> sinkNodeAttr = nodeAttrMap[edges[(edge_part_index-1) / 2].first.second];
                            edge_compute_flag =false;
                            cost_min = std::numeric_limits<double>::max();
                            for(int k=1; k<=attribute_index_number;++k){
                                combCalculation(0,k,sourceId,sinkId,sourceNodeAttr, sinkNodeAttr,attribute_index, origin_edge_id, cost_min);
                            }

                            edge_val =cost_min;
                            costs[(edge_part_index-1) / 2] = edge_val;
                            //printf("The edge id %d, with cost %f \n",(edge_part_index-1) / 2, costs[(edge_part_index-1) / 2]);
                            edge_compute_count = edge_compute_count+ 1;

                        }

                    }

                    temp_pair.push_back(std::make_pair(edge_val, edge_part_index));
                }
                if(other_cluster.edge_parts.is_empty()&&(other_cluster.edge_compute_flag==false)){
                    std::vector<std::pair<double, int> >::iterator it_temp;
                    for (it_temp = temp_pair.end()-1; it_temp != temp_pair.begin()-1; it_temp--) {
                        //std::cout<<"\n";
                        double val= it_temp->first;
                        int edge_part_index= it_temp->second;
                        EdgePart& uu_part = edge_parts[edge_part_index];
                        int the_other_edge_part_index = get_other_edge_part_index(
                                edge_part_index);

                        EdgePart& vv_part = edge_parts[the_other_edge_part_index];


                        Cluster& uu_cluster = other_cluster;

                        //calculate the current the_other_cluster_index
                        int endpoint = edges[the_other_edge_part_index / 2].first.first;
                        if (the_other_edge_part_index % 2 == 1) {
                            endpoint = edges[the_other_edge_part_index / 2].first.second;
                        }
                        int the_other_cluster_index = endpoint;
                        double total_sum = 0.0;
                        path_compression_visited.resize(0);

                        while (clusters[the_other_cluster_index].merged_into != -1) {
                            path_compression_visited.push_back(make_pair(the_other_cluster_index,
                                                                         total_sum));
                            if (clusters[the_other_cluster_index].skip_up >= 0) {
                                total_sum += clusters[the_other_cluster_index].skip_up_sum;
                                the_other_cluster_index = clusters[the_other_cluster_index].skip_up;
                            } else {
                                total_sum += clusters[the_other_cluster_index].moat;
                                the_other_cluster_index = clusters[the_other_cluster_index].merged_into;
                            }
                        }

                        Cluster& vv_cluster = clusters[the_other_cluster_index];


                        double event_time = val / 2.0;   //event_time equals cost/2
                        if(uu_part.edge_compute_flag==false){
                            uu_part.next_event_val = event_time;
                        }
                        if(vv_part.edge_compute_flag==false){
                            vv_part.next_event_val = event_time;
                        }

                        uu_part.heap_node = uu_cluster.edge_parts.insert(
                                uu_part.next_event_val, edge_part_index);

                        if(vv_part.edge_compute_flag==false){
                            vv_cluster.edge_parts.decrease_key(
                                    vv_part.heap_node,
                                    (std::numeric_limits<double>::max()/2.0),
                                    vv_part.next_event_val);//vv_part is the root as this next_event_val pairing heap
                        }


                        uu_part.edge_compute_flag = true;
                        vv_part.edge_compute_flag = true;


                    }
                    other_cluster.edge_compute_flag = true;
                }
                //update other_edge_part edge_compute_flag
                next_edge_part.edge_compute_flag = true;
                other_edge_part.edge_compute_flag = true;






                phase1_result.push_back(next_edge_part_index / 2);//indicate the edge index is pushed into phase1_result
                edge_parts[other_edge_part_index].deleted = true;//mark edge_parts[other_edge_part_index] delete flag as true

                int new_cluster_index = clusters.size();//get the current clusters.size()
                clusters.push_back(Cluster(&pairing_heap_buffer));//new a new clusters
                Cluster& new_cluster = clusters[new_cluster_index];//new_cluster points to clusters[new_cluster_index]
                // This is slightly evil because current_cluster and other_cluster
                // mask the outside definitions. However, the outside definitons
                // might have become invalid due to the call to push_back.
                Cluster& current_cluster = clusters[current_cluster_index]; //current_cluster points to clusters[current_cluster_index]
                Cluster& other_cluster = clusters[other_cluster_index];//

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Merge %d and %d into %d\n", current_cluster_index,
                             other_cluster_index, new_cluster_index);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                new_cluster.moat = 0.0;
                new_cluster.prize_sum = current_cluster.prize_sum
                                        + other_cluster.prize_sum;
                new_cluster.subcluster_moat_sum = current_cluster.subcluster_moat_sum
                                                  + other_cluster.subcluster_moat_sum;
                new_cluster.contains_root = current_cluster.contains_root
                                            || other_cluster.contains_root;
                new_cluster.active = !new_cluster.contains_root;
                new_cluster.merged_along = next_edge_part_index / 2;
                new_cluster.child_cluster_1 = current_cluster_index;
                new_cluster.child_cluster_2 = other_cluster_index;
                new_cluster.necessary = false;
                new_cluster.skip_up = -1;
                new_cluster.skip_up_sum = 0.0;
                new_cluster.merged_into = -1;
                new_cluster.vertex_list.reserve(current_cluster.vertex_list.size()+ other_cluster.vertex_list.size());
                new_cluster.vertex_list.insert(new_cluster.vertex_list.end(),current_cluster.vertex_list.begin(),
                                               current_cluster.vertex_list.end());
                new_cluster.vertex_list.insert(new_cluster.vertex_list.end(),other_cluster.vertex_list.begin(),
                                               other_cluster.vertex_list.end());
                //printf("new cluster is %d \n",new_cluster_index);
                //printf("the vertex id is \n");
                /*
                for (std::vector<int>::iterator it = new_cluster.vertex_list.begin() ; it != new_cluster.vertex_list.end(); ++it)
                    std::cout << ' ' << *it;
                std::cout << '\n';
                */


                current_cluster.active = false;
                current_cluster.active_end_time = current_time + remainder;
                current_cluster.merged_into = new_cluster_index;
                current_cluster.moat = current_cluster.active_end_time
                                       - current_cluster.active_start_time;
                clusters_deactivation.delete_element(current_cluster_index);
                num_active_clusters -= 1;


                if (!current_cluster.edge_parts.is_empty()) {
                    clusters_next_edge_event.delete_element(current_cluster_index);
                }

                if (other_cluster.active) {
                    stats.num_active_active_merge_events += 1;

                    other_cluster.active = false;
                    other_cluster.active_end_time = current_time + remainder;
                    other_cluster.moat = other_cluster.active_end_time
                                         - other_cluster.active_start_time;
                    clusters_deactivation.delete_element(other_cluster_index);
                    if (!other_cluster.edge_parts.is_empty()) {
                        clusters_next_edge_event.delete_element(other_cluster_index);
                    }
                    num_active_clusters -= 1;
                } else {
                    stats.num_active_inactive_merge_events += 1;
                    if (!other_cluster.contains_root) {
                        double edge_event_update_time = current_time + remainder
                                                        - other_cluster.active_end_time;
                        other_cluster.edge_parts.add_to_heap(edge_event_update_time);
                        inactive_merge_events.push_back(InactiveMergeEvent());
                        InactiveMergeEvent& merge_event =
                                inactive_merge_events[inactive_merge_events.size() - 1];

                        merge_event.active_cluster_index = current_cluster_index;
                        merge_event.inactive_cluster_index = other_cluster_index;
                        int active_node_part = edges[next_edge_part_index / 2].first.first;
                        int inactive_node_part = edges[next_edge_part_index / 2].first.second;
                        if (next_edge_part_index % 2 == 1) {
                            std::swap(active_node_part, inactive_node_part);
                        }
                        merge_event.active_cluster_node = active_node_part;
                        merge_event.inactive_cluster_node = inactive_node_part;
                        edge_info[next_edge_part_index / 2].inactive_merge_event =
                                inactive_merge_events.size() - 1;

                    }
                }
                other_cluster.merged_into = new_cluster_index;

                new_cluster.edge_parts = PairingHeapType::meld(
                        &(current_cluster.edge_parts), &(other_cluster.edge_parts));//Merge the current_cluster and other_cluster edge_parts heap together

                new_cluster.subcluster_moat_sum += current_cluster.moat;
                new_cluster.subcluster_moat_sum += other_cluster.moat;

                if (new_cluster.active) {
                    new_cluster.active_start_time = current_time + remainder;
                    double becoming_inactive_time = current_time + remainder
                                                    + new_cluster.prize_sum
                                                    - new_cluster.subcluster_moat_sum;
                    clusters_deactivation.insert(becoming_inactive_time,
                                                 new_cluster_index);
                    if (!new_cluster.edge_parts.is_empty()) {
                        double tmp_val;
                        int tmp_index;
                        //The pairingHeap property requires this !!!!!!!
                        //new_cluster.edge_parts.delete_min(&tmp_val, &tmp_index);
                        new_cluster.edge_parts.get_min(&tmp_val, &tmp_index);

                        //test the new edge_part must be outgoing edges
                        int endpoint = edges[tmp_index/ 2].first.first;
                        if(tmp_index % 2 == 1){
                            endpoint = edges[tmp_index / 2].first.second;
                        }
                        //printf("the added clusters_next_edge_event corresponds to node %d \n",endpoint);
                        //printf("new cluster next_edge_event_update \n");
                        //printf("the added edge_part is %d \n",tmp_index);
                        //printf("the added value is %f \n",tmp_val);

                        clusters_next_edge_event.insert(tmp_val, new_cluster_index);
                    }
                    new_cluster.edge_compute_flag=true;
                    num_active_clusters += 1;
                }
            } else if (other_cluster.active) {  //corresponding generateNewEdgeEvents(pu,pv)
                stats.total_num_edge_growth_events += 1;
                stats.num_active_active_edge_growth_events += 1;

                double next_event_time = current_time + remainder / 2.0;
                next_edge_part.next_event_val = sum_current_edge_part + remainder / 2.0;
                if (!current_cluster.edge_parts.is_empty()) {
                    clusters_next_edge_event.delete_element(current_cluster_index);
                }
                next_edge_part.heap_node = current_cluster.edge_parts.insert(
                        next_event_time, next_edge_part_index);
                double tmp_val = -1.0;
                int tmp_index = -1;
                current_cluster.edge_parts.get_min(&tmp_val, &tmp_index);
                //printf("the min_value is %f, edge_part id is %d \n",tmp_val,tmp_index);

                clusters_next_edge_event.insert(tmp_val, current_cluster_index);

                clusters_next_edge_event.delete_element(other_cluster_index);
                other_cluster.edge_parts.decrease_key(
                        other_edge_part.heap_node,
                        other_cluster.active_start_time + other_edge_part.next_event_val
                        - other_finished_moat_sum,
                        next_event_time);
                other_cluster.edge_parts.get_min(&tmp_val, &tmp_index);
                //printf("the min_value is %f, edge_part id is %d \n",tmp_val,tmp_index);
                clusters_next_edge_event.insert(tmp_val, other_cluster_index);
                other_edge_part.next_event_val = sum_other_edge_part + remainder / 2.0;

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Added new event at time %e\n", next_event_time);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////
            } else {
                stats.total_num_edge_growth_events += 1;
                stats.num_active_inactive_edge_growth_events += 1;

                double next_event_time = current_time + remainder;
                next_edge_part.next_event_val = current_edge_cost
                                                - other_finished_moat_sum;
                if (!current_cluster.edge_parts.is_empty()) {
                    clusters_next_edge_event.delete_element(current_cluster_index);
                }
                next_edge_part.heap_node = current_cluster.edge_parts.insert(
                        next_event_time, next_edge_part_index);
                double tmp_val = -1.0;
                int tmp_index = -1;
                current_cluster.edge_parts.get_min(&tmp_val, &tmp_index);
                //printf("the min_value is %f, edge_part id is %d \n",tmp_val,tmp_index);
                clusters_next_edge_event.insert(tmp_val, current_cluster_index);

                other_cluster.edge_parts.decrease_key(
                        other_edge_part.heap_node,
                        other_cluster.active_end_time + other_edge_part.next_event_val
                        - other_finished_moat_sum,
                        other_cluster.active_end_time);
                other_edge_part.next_event_val = other_finished_moat_sum;

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Added new event at time %e and and event for inactive edge "
                                     "part\n", next_event_time);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////
            }
        } else {
            stats.num_cluster_events += 1;

            // cluster deactivation is first

            current_time = next_cluster_time;
            remove_next_cluster_event();

            Cluster& cur_cluster = clusters[next_cluster_index];
            cur_cluster.active = false;
            cur_cluster.active_end_time = current_time;
            cur_cluster.moat = cur_cluster.active_end_time
                               - cur_cluster.active_start_time;
            if (!cur_cluster.edge_parts.is_empty()) {
                clusters_next_edge_event.delete_element(next_cluster_index);
            }
            num_active_clusters -= 1;

            //////////////////////////////////////////
            if (verbosity_level >= 2) {
                snprintf(output_buffer, kOutputBufferSize,
                         "Cluster deactivation: cluster %d at time %e (moat size %e)\n",
                         next_cluster_index, current_time, cur_cluster.moat);
                output_function(output_buffer);
            }
            //////////////////////////////////////////
        }
    }
    //pcst_endTime = clock();
    //cout<<"Total Running Time : " <<(double)(pcst_endTime - pcst_startTime) / CLOCKS_PER_SEC << "s" << endl;
    /*
    pcst_endTime = clock();
    ofstream outfile;
    outfile./open("pcst_running_breakdown.txt");

    outfile <<"------------------------Final result-------------------------------------" << endl;
    outfile << "Total Running Time : " <<(double)(pcst_endTime - pcst_startTime) / CLOCKS_PER_SEC << "s" << endl;
    outfile <<"Edge Result:"<<endl;
    */
    //outfile.close();

    //////////////////////////////////////////
    if (verbosity_level >= 1) {
        snprintf(output_buffer, kOutputBufferSize,
                 "Finished GW clustering: final event time %e, number of edge events "
                         "%lld\n", current_time, stats.total_num_edge_events);
        output_function(output_buffer);
    }
    //////////////////////////////////////////


    // Mark root cluster or active clusters as good.
    node_good.resize(prizes.size(), false);

    if (root >= 0) {
        // find the root cluster
        for (size_t ii = 0; ii < clusters.size(); ++ii) {
            if (clusters[ii].contains_root && clusters[ii].merged_into == -1) {
                mark_nodes_as_good(ii);
                break;
            }
        }
    } else {
        for (size_t ii = 0; ii < clusters.size(); ++ii) {
            if (clusters[ii].active) {
                mark_nodes_as_good(ii);
            }
        }
    }

    if (pruning == kNoPruning) {
        build_phase1_node_set(phase1_result, result_nodes);
        *result_edges = phase1_result;
        //double result_sum;
        for (std::vector<int>::iterator it = (*result_edges).begin(); it != (*result_edges).end(); ++it)
            result_sum +=costs[*it];
        return true;
    }

    //////////////////////////////////////////
    if (verbosity_level >= 2) {
        snprintf(output_buffer, kOutputBufferSize,
                 "------------------------------------------\n");
        output_function(output_buffer);
        snprintf(output_buffer, kOutputBufferSize, "Starting pruning\n");
        output_function(output_buffer);
    }
    //////////////////////////////////////////

    for (size_t ii = 0; ii < phase1_result.size(); ++ii) {
        if (node_good[edges[phase1_result[ii]].first.first]
            && node_good[edges[phase1_result[ii]].first.second]) {
            phase2_result.push_back(phase1_result[ii]);
        }
    }

    if (pruning == kSimplePruning) {
        build_phase2_node_set(result_nodes);
        *result_edges = phase2_result;
        return true;
    }

    vector<int> phase3_result;
    phase3_neighbors.resize(prizes.size());
    for (size_t ii = 0; ii < phase2_result.size(); ++ii) {
        int cur_edge_index = phase2_result[ii];
        int uu = edges[cur_edge_index].first.first;
        int vv = edges[cur_edge_index].first.second;
        double cur_cost = costs[cur_edge_index];
        phase3_neighbors[uu].push_back(make_pair(vv, cur_cost));
        phase3_neighbors[vv].push_back(make_pair(uu, cur_cost));
    }

    if (pruning == kGWPruning) {

        //////////////////////////////////////////
        if (verbosity_level >= 2) {
            snprintf(output_buffer, kOutputBufferSize,
                     "Starting GW pruning, phase 2 result:\n");
            output_function(output_buffer);
            for (size_t ii = 0; ii < phase2_result.size(); ++ii) {
                snprintf(output_buffer, kOutputBufferSize, "%d ", phase2_result[ii]);
                output_function(output_buffer);
            }
            snprintf(output_buffer, kOutputBufferSize, "\n");
            output_function(output_buffer);
        }
        //////////////////////////////////////////

        for (int ii = phase2_result.size() - 1; ii >= 0; --ii) {
            int cur_edge_index = phase2_result[ii];
            int uu = edges[cur_edge_index].first.first;
            int vv = edges[cur_edge_index].first.second;

            if (node_deleted[uu] && node_deleted[vv]) {

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Not keeping edge %d (%d, %d) because both endpoints already "
                                     "deleted\n", cur_edge_index, uu, vv);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                continue;
            }

            if (edge_info[cur_edge_index].inactive_merge_event < 0) {
                mark_clusters_as_necessary(uu);
                mark_clusters_as_necessary(vv);
                phase3_result.push_back(cur_edge_index);

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Both endpoint clusters were active, so keeping edge %d "
                                     "(%d, %d)\n", cur_edge_index, uu, vv);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

            } else {
                InactiveMergeEvent& cur_merge_event = inactive_merge_events[
                        edge_info[cur_edge_index].inactive_merge_event];
                int active_side_node = cur_merge_event.active_cluster_node;
                int inactive_side_node = cur_merge_event.inactive_cluster_node;
                int inactive_cluster_index = cur_merge_event.inactive_cluster_index;

                if (clusters[inactive_cluster_index].necessary) {
                    phase3_result.push_back(cur_edge_index);
                    mark_clusters_as_necessary(inactive_side_node);
                    mark_clusters_as_necessary(active_side_node);

                    //////////////////////////////////////////
                    if (verbosity_level >= 2) {
                        snprintf(output_buffer, kOutputBufferSize,
                                 "One endpoint was inactive but is marked necessary (%d), so "
                                         "keeping edge %d (%d, %d)\n", inactive_cluster_index,
                                 cur_edge_index, uu, vv);
                        output_function(output_buffer);
                    }
                    //////////////////////////////////////////

                } else {
                    mark_nodes_as_deleted(inactive_side_node, active_side_node);

                    //////////////////////////////////////////
                    if (verbosity_level >= 2) {
                        snprintf(output_buffer, kOutputBufferSize,
                                 "One endpoint was inactive and not marked necessary (%d), so "
                                         "discarding edge %d (%d, %d)\n", inactive_cluster_index,
                                 cur_edge_index, uu, vv);
                        output_function(output_buffer);
                    }
                    //////////////////////////////////////////

                }
            }
        }

        build_phase3_node_set(result_nodes);
        *result_edges = phase3_result;
        for (std::vector<int>::iterator it = (*result_edges).begin(); it != (*result_edges).end(); ++it)
            result_sum +=double (costs[*it]);
        return true;
    } else if (pruning == kStrongPruning) {

        //////////////////////////////////////////
        if (verbosity_level >= 2) {
            snprintf(output_buffer, kOutputBufferSize,
                     "Starting Strong pruning, phase 2 result:\n");
            output_function(output_buffer);
            for (size_t ii = 0; ii < phase2_result.size(); ++ii) {
                snprintf(output_buffer, kOutputBufferSize, "%d ", phase2_result[ii]);
                output_function(output_buffer);
            }
            snprintf(output_buffer, kOutputBufferSize, "\n");
            output_function(output_buffer);
        }
        //////////////////////////////////////////

        final_component_label.resize(prizes.size(), -1);
        root_component_index = -1;
        strong_pruning_parent.resize(prizes.size(), make_pair(-1, -1.0));
        strong_pruning_payoff.resize(prizes.size(), -1.0);

        for (size_t ii = 0; ii < phase2_result.size(); ++ii) {
            int cur_node_index = edges[phase2_result[ii]].first.first;
            if (final_component_label[cur_node_index] == -1) {
                final_components.push_back(std::vector<int>());
                label_final_component(cur_node_index, final_components.size() - 1);
            }
        }

        for (int ii = 0; ii < static_cast<int>(final_components.size()); ++ii) {

            //////////////////////////////////////////
            if (verbosity_level >= 2) {
                snprintf(output_buffer, kOutputBufferSize,
                         "Strong pruning on final component %d (size %d):\n",
                         ii, static_cast<int>(final_components[ii].size()));
                output_function(output_buffer);
            }
            //////////////////////////////////////////

            if (ii == root_component_index) {

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Component contains root, pruning starting at %d\n", root);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                strong_pruning_from(root, true);
            } else {

                int best_component_root = find_best_component_root(ii);

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Best start node for current component: %d, pruning from "
                                     "there\n", best_component_root);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

                strong_pruning_from(best_component_root, true);
            }
        }

        for (int ii = 0; ii < static_cast<int>(phase2_result.size()); ++ii) {
            int cur_edge_index = phase2_result[ii];
            int uu = edges[cur_edge_index].first.first;
            int vv = edges[cur_edge_index].first.second;

            if (node_deleted[uu] || node_deleted[vv]) {

                //////////////////////////////////////////
                if (verbosity_level >= 2) {
                    snprintf(output_buffer, kOutputBufferSize,
                             "Not keeping edge %d (%d, %d) because at least one endpoint "
                                     "already deleted\n", cur_edge_index, uu, vv);
                    output_function(output_buffer);
                }
                //////////////////////////////////////////

            } else {
                phase3_result.push_back(cur_edge_index);
            }
        }

        build_phase3_node_set(result_nodes);
        *result_edges = phase3_result;
        for (std::vector<int>::iterator it = (*result_edges).begin(); it != (*result_edges).end(); ++it){
         printf("result id is %d, cost is %f \n", *it, costs[*it]);  
	 result_sum +=double (costs[*it]);
	 printf("current summation is %f \n",result_sum);
}
	//pcst_endTime = clock();
	//cout<<"Total Running time of PCST: "<<(double)(pcst_endTime - pcst_startTime)/ CLOCKS_PER_SEC <<"s"<<endl;
        clock_gettime(CLOCK_MONOTONIC, &pcst_endTime);
        elapsed = (pcst_endTime.tv_sec - pcst_startTime.tv_sec);
        elapsed += (pcst_endTime.tv_nsec - pcst_startTime.tv_nsec)/1000000000.0;
        cout<<"Total Running time of PCST is: "<<(double)(elapsed)<<"s"<<endl;
        return true;
    }

    snprintf(output_buffer, kOutputBufferSize,
             "Error: unknown pruning scheme.\n");
    output_function(output_buffer);
    return false;
}

void PCSTFast::label_final_component(int start_node_index,
                                     int new_component_index) {
    cluster_queue.resize(0);
    cluster_queue.push_back(start_node_index);
    final_component_label[start_node_index] = new_component_index;

    int queue_next = 0;
    while (queue_next < static_cast<int>(cluster_queue.size())) {
        int cur_node_index = cluster_queue[queue_next];
        queue_next += 1;
        final_components[new_component_index].push_back(cur_node_index);
        if (cur_node_index == root) {
            root_component_index = new_component_index;
        }
        for (size_t ii = 0; ii < phase3_neighbors[cur_node_index].size(); ++ii) {
            int next_node_index = phase3_neighbors[cur_node_index][ii].first;
            if (final_component_label[next_node_index] == -1) {
                cluster_queue.push_back(next_node_index);
                final_component_label[next_node_index] = new_component_index;
            }
        }
    }
}

void PCSTFast::strong_pruning_from(int start_node_index,
                                   bool mark_as_deleted) {
    stack.resize(0);
    stack.push_back(make_pair(true, start_node_index));
    strong_pruning_parent[start_node_index] = make_pair(-1, 0.0);

    while (!stack.empty()) {
        bool begin = stack.back().first;
        int cur_node_index = stack.back().second;
        stack.pop_back();

        if (begin) {
            stack.push_back(make_pair(false, cur_node_index));
            for (size_t ii = 0; ii < phase3_neighbors[cur_node_index].size(); ++ii) {
                int next_node_index = phase3_neighbors[cur_node_index][ii].first;
                double next_cost = phase3_neighbors[cur_node_index][ii].second;

                if (next_node_index == strong_pruning_parent[cur_node_index].first) {
                    continue;
                }

                strong_pruning_parent[next_node_index].first = cur_node_index;
                strong_pruning_parent[next_node_index].second = next_cost;
                stack.push_back(make_pair(true, next_node_index));
            }
        } else {
            strong_pruning_payoff[cur_node_index] = prizes[cur_node_index];
            for (size_t ii = 0; ii < phase3_neighbors[cur_node_index].size(); ++ii) {
                int next_node_index = phase3_neighbors[cur_node_index][ii].first;
                double next_cost = phase3_neighbors[cur_node_index][ii].second;

                if (next_node_index == strong_pruning_parent[cur_node_index].first) {
                    continue;
                }

                double next_payoff = strong_pruning_payoff[next_node_index] - next_cost;
                if (next_payoff <= 0.0) {
                    if (mark_as_deleted) {

                        //////////////////////////////////////////
                        if (verbosity_level >= 2) {
                            snprintf(output_buffer, kOutputBufferSize,
                                     "Subtree starting at %d has a nonpositive contribution of "
                                             "%e, pruning (good side: %d)\n", next_node_index,
                                     next_payoff,  cur_node_index);
                            output_function(output_buffer);
                        }
                        //////////////////////////////////////////

                        mark_nodes_as_deleted(next_node_index, cur_node_index);
                    }
                } else {
                    strong_pruning_payoff[cur_node_index] += next_payoff;
                }
            }
        }
    }
}

int PCSTFast::find_best_component_root(int component_index) {
    int cur_best_root_index = final_components[component_index][0];
    strong_pruning_from(cur_best_root_index, false);
    double cur_best_value = strong_pruning_payoff[cur_best_root_index];

    stack2.resize(0);
    for (size_t ii = 0; ii < phase3_neighbors[cur_best_root_index].size(); ++ii) {
        stack2.push_back(phase3_neighbors[cur_best_root_index][ii].first);
    }

    while (!stack2.empty()) {
        int cur_node_index = stack2.back();
        stack2.pop_back();
        int cur_parent_index = strong_pruning_parent[cur_node_index].first;
        double parent_edge_cost = strong_pruning_parent[cur_node_index].second;
        double parent_val_without_cur_node =
                strong_pruning_payoff[cur_parent_index];
        double cur_node_net_payoff = strong_pruning_payoff[cur_node_index]
                                     - parent_edge_cost;
        if (cur_node_net_payoff > 0.0) {
            parent_val_without_cur_node -= cur_node_net_payoff;
        }
        if (parent_val_without_cur_node > parent_edge_cost) {
            strong_pruning_payoff[cur_node_index] += (parent_val_without_cur_node
                                                      - parent_edge_cost);
        }
        if (strong_pruning_payoff[cur_node_index] > cur_best_value) {
            cur_best_root_index = cur_node_index;
            cur_best_value = strong_pruning_payoff[cur_node_index];
        }
        for (size_t ii = 0; ii < phase3_neighbors[cur_node_index].size(); ++ii) {
            int next_node_index = phase3_neighbors[cur_node_index][ii].first;
            if (next_node_index != cur_parent_index) {
                stack2.push_back(next_node_index);
            }
        }
    }

    return cur_best_root_index;
}

void PCSTFast::build_phase3_node_set(std::vector<int>* node_set) {
    node_set->clear();
    for (int ii = 0; ii < static_cast<int>(prizes.size()); ++ii) {
        if (!node_deleted[ii] && node_good[ii]) {
            node_set->push_back(ii);
        }
    }
}


void PCSTFast::build_phase2_node_set(std::vector<int>* node_set) {
    node_set->clear();
    for (int ii = 0; ii < static_cast<int>(prizes.size()); ++ii) {
        if (node_good[ii]) {
            node_set->push_back(ii);
        }
    }
}


void PCSTFast::build_phase1_node_set(const std::vector<int>& edge_set,
                                     std::vector<int>* node_set) {
    vector<int> included(prizes.size(), false);
    node_set->clear();
    for (size_t ii = 0; ii < edge_set.size(); ++ii) {
        int uu = edges[edge_set[ii]].first.first;
        int vv = edges[edge_set[ii]].first.second;
        if (!included[uu]) {
            included[uu] = true;
            node_set->push_back(uu);
        }
        if (!included[vv]) {
            included[vv] = true;
            node_set->push_back(vv);
        }
    }
    for (int ii = 0; ii < static_cast<int>(prizes.size()); ++ii) {
        if (node_good[ii] && !included[ii]) {
            node_set->push_back(ii);
        }
    }
}

PCSTFast::~PCSTFast() {
    for (size_t ii = 0; ii < clusters.size(); ++ii) {
        clusters[ii].edge_parts.release_memory();
    }
}


PCSTFast::Statistics PCSTFast::get_statistics() {
    return stats;
}

struct Result{
    std::vector<int> result_nodes;
    std::vector<int> result_edges;
};

double PCSTFast::computeDis(double dou1, double dou2, double& doubleMax, double& doubleMin){
    double weight = abs((dou1 - dou2)/(doubleMax-doubleMin));
    return weight;
}


void PCSTFast::pretty_print(const std::vector<int>& v, const int sourceId, const int sinkId,
                         std::vector<double>  sourceNode, std::vector<double>  sinkNode,
                         long origin_edge_id,
                         double &cost_min
                         ) {


    eps = 1e-10;
    //printf("sourceId is %d, sinkId is %d \n", sourceId, sinkId);
    //multiEdges.push_back(std::make_pair(std::make_pair(sourceId, sinkId),origin_edge_id));
    //printf("edge id is %lu \n",origin_edge_id);
    //edgeAttributeMap.insert(
    //std::pair<long, std::vector<int>>(origin_edge_id, v));
    if (edge_compute_flag == false) {
        double cost = PCSTFast::computeEdgeWeight(sourceNode, sinkNode, attribute_stat,
                                                  v);
        if (cost < cost_min) {
            cost_min = cost;
            //printf("cost_min value is %f \n", cost_min);
        }
        if (cost < eps) {
            edge_compute_flag = true;
        }
    } else {
        return;
    }


}

void PCSTFast::combCalculation(int offset, int k, const int sourceId, const int sinkId,
                            std::vector<double> sourceNode, std::vector<double> sinkNode,
                            std::vector<int> attribute_index,
                            long origin_edge_id,
                            double &cost_min
) {
    if (k == 0) {
        /*
        printf("Edge attribute vector is \n");
        for (std::vector<int>::iterator it = attribute_index_combination.begin() ; it != attribute_index_combination.end(); ++it)
            cout << ' ' << *it<<endl;
        cout << '\n';
        */
        pretty_print(attribute_index_combination, sourceId, sinkId,sourceNode, sinkNode, origin_edge_id,cost_min);
        return;
    }
    if(edge_compute_flag==false){
        for (int i = offset; i <= attribute_index.size() - k; ++i) {
            attribute_index_combination.push_back(attribute_index[i]);
            PCSTFast::combCalculation(i+1, k-1, sourceId, sinkId,sourceNode,sinkNode ,attribute_index, origin_edge_id, cost_min);
            attribute_index_combination.pop_back();
        }
    }else{
        return;
    }

}

double PCSTFast::computeEdgeWeight(std::vector<double> sourceNode, std::vector<double> sinkNode, std::vector<Graph::AttributeInfo> attribute_stat,
                                std::vector<int> attribute_index_combination
) {
    std::vector<double> dis_value;
    std::vector<int>::iterator it;
    for(int i=0;i<attribute_index_combination.size();i++){

        //printf("i is %d, the value is %d \n", i, attribute_index_combination[i]);
        double temp_dis = PCSTFast::computeDis(sourceNode[attribute_index_combination[i]-1],sinkNode[attribute_index_combination[i]-1], attribute_stat[attribute_index_combination[i]-1].max_value,attribute_stat[attribute_index_combination[i]-1].min_value);
        dis_value.push_back(temp_dis);

    }
    double weighted_value = 0;
    std::vector<double>::iterator it2;
    for (it2=dis_value.begin(); it2!=dis_value.end();++it2){
        weighted_value +=*it2;
    }
    double weight = (weighted_value)/(0.0+ attribute_index_combination.size());
    //printf("the edge weight is %f \n",weight);
    return weight;


}









