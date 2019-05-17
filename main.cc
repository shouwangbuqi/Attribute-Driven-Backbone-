#include <iostream>
#include <fstream>
#include <time.h>
#include "pcst_fast.h"
#include "priority_queue.h"
#include "Graph.h"

#include <algorithm>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>
#include <set>
#include <iomanip>

using cluster_approx::PCSTFast;
using std::make_pair;
using std::vector;
using std:: ofstream;
using std:: endl;
using std:: cout;

//Try to implement pcst_fast from pcst_fast_pybind.cc
void output (const char*p){
    std::cout<< *p;
}



int main(int argc, char **argv) {

    /*
    std::vector<std::pair<int, int>> edges ={{0,1},{1,2},{2,3}};
    //Compute the prizes for each node NodePrizesComputation()
    std::vector<double> prizes ={1.0,
                                 1.0,
                                 1.0,
                                 1.0};

    //Compute the costs for each edge EdgeCostsComputation()
    std::vector<double> costs = {0.8,
                                 1.8,
                                 2.8};

    //add the new parameters as terminalSet
    std::set<int> newTerminals = {0,5,6,7};



    */

    /*
    std::vector<std::pair<int, int> > newEdges ={{0,1},{0,2},{1,4},{2,5},{4,5},{0,3},{3,5},{5,6},{6,7},{1,2},{2,3},{2,4},{2,6}};
    std::vector<double> newPrizes ={5,
                                    5,
                                    5,
                                    5,
                                    5,
                                    5,
                                    5,
                                    5};
    std::set<int> newTerminals = {0,1,2,3,4,5,6,7};
    */


    std::vector<int> result_nodes;
    std::vector<int> result_edges;




    /*
    std:: string file = "/Users/April/Downloads/sampleGraph/sampleGraphSmall.txt";
    std:: string attributeFile = "/Users/April/Downloads/sampleGraph/UserProfileExample.txt";
    std:: string idMapFilePath = "/Users/April/Downloads/uidlistExample.txt";
    std:: string penaltyFilePath = "/Users/April/Downloads/sampleGraph/penaltySmall.txt";
    int attribute_index_number = 2;
    */




















    std:: string file = "/Users/April/Downloads/sampleGraph/graph_4000_e62500.txt";
    std:: string attributeFile = "/Users/April/Downloads/sampleGraph/graph_4000_e62500-magfitf-8attr";
    std:: string penaltyFilePath = "/Users/April/Downloads/sampleGraph/_4_250nodes.penalties-attr8-e62500.txt";
    std:: string idMapFilePath = "/Users/April/Downloads/sampleGraph/_4_250nodes.penalties-attr8-e62500.txt";
    int attribute_index_number_main = 8;
















    /*
    std:: string file(argv[1]);
    std:: string attributeFile(argv[2]);
    std:: string idMapFilePath(argv[3]);
    std:: string penaltyFilePath(argv[4]);
    int attribute_index_number_main= atoi(argv[5]);
    */











    cluster_approx::Graph *graph = new cluster_approx::Graph(file,attributeFile,idMapFilePath,penaltyFilePath);

    std::vector<std::pair<std::pair<long, long>, long> > edges  = graph->edges;
    std::vector<double> prizes = graph->prizes;
    std::set<int> terminals = graph->terminals;
    std::map<long,std::vector<int>>edgeAttributeMap = graph->edgeAttributeMap;
    std::map<long, std::vector<double>> nodeAttrMap = graph->nodeAttrMap;
    std::vector<cluster_approx::Graph::AttributeInfo> attribute_stat = graph->attribute_stat;



    graph->edges.clear();
    graph->costs.clear();
    graph->terminals.clear();
    graph->attribute_index.clear();




    std::cout << "Number of edges is " <<  edges.size() <<endl;

    clock_t startTime,endTime;
    startTime = clock();

    PCSTFast *PCSTFastInstance = new PCSTFast(edges, prizes, terminals, -1, 1, cluster_approx::PCSTFast::PruningMethod::kStrongPruning, 0,output,
            edgeAttributeMap, nodeAttrMap,attribute_stat
    );
    //printf("test \n");
    bool a = (*PCSTFastInstance).run(&result_nodes, &result_edges, attribute_index_number_main);
    endTime = clock();

    int edge_computation_count = (*PCSTFastInstance).edge_compute_count;
    printf("the edge_computation_count is %d \n", edge_computation_count);

    PCSTFast::Statistics b =(*PCSTFastInstance).get_statistics();
/*
    int active_edge_growth_events_count =b.num_cluster_events;
    std::cout << active_edge_growth_events_count;
*/
    long long eventsNumber = b.total_num_edge_events;


    std::cout << "------------------------Final result-------------------------------------" << std::endl;

    std::sort(result_edges.begin(),result_edges.end());
    std::cout<<"Edge Result"<<std::endl;
    for (std::vector<int>::iterator it = result_edges.begin() ; it != result_edges.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << '\n';

    ofstream outfile1;

    outfile1.open("result_nodes.txt");
    outfile1 << "---------------NodeID----------------" << std::endl;
    for (std::vector<int>::iterator it = result_nodes.begin() ; it != result_nodes.end(); ++it)
        outfile1 << ' ' << *it <<endl;
    outfile1 << '\n';



    std::cout<<"Node Result"<<std::endl;
    for (std::vector<int>::iterator it = result_nodes.begin() ; it != result_nodes.end(); ++it)
        std::cout << ' ' << *it;
    std::cout << '\n';
    printf("The total number of nodes is %d \n",result_nodes.size());

    //std::set<int> result_nodes_set(result_nodes.begin(),result_nodes.end());


    ofstream outfile;
    outfile.open("result_clusters.txt");

    outfile <<"------------------------Final result-------------------------------------" << endl;
    outfile << "Total Running Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;
    outfile <<"Total number of edge events:" << eventsNumber <<std::endl;
    outfile <<"Edge Result:"<<endl;


    std::sort(result_edges.begin(),result_edges.end());


    //sort result_edges first
    /*
    for (std::vector<int>::iterator it = result_edges.begin() ; it != result_edges.end(); ++it)
        outfile <<edges[*it].first << "--->" << edges[*it].second << endl;
    outfile << '\n';
    */

    /*
    outfile << "---------------EdgeID----------------" << std::endl;
    for (std::vector<int>::iterator it = result_edges.begin() ; it != result_edges.end(); ++it)
        outfile <<*it<< endl;
    outfile << '\n';
    */


    outfile <<"----------------The total cost in the Tree ---------------------" <<endl;
    double result_sum = (*PCSTFastInstance).get_result_sum();
    printf("result sum is %f",result_sum);
    outfile<<std::fixed<<std::setprecision(4)<<result_sum;
    outfile<< "\n";




    outfile <<"-------------------Output nodes who choose to pay penalties----------------"<<endl;
    std::vector<int> penalty_nodes(terminals.size());
    std::vector<int>::iterator it;
    it=std::set_difference(terminals.begin(),terminals.end(),result_nodes.begin(),result_nodes.end(),penalty_nodes.begin());
    penalty_nodes.resize(it-penalty_nodes.begin());

    double penalty_sum = 0.0;
    for (it = penalty_nodes.begin(); it != penalty_nodes.end(); ++it){
            outfile<<' '<<*it<<endl;
            penalty_sum += prizes[*it];
    }
    printf("penalty sum is %f",penalty_sum);
    outfile<< "\n";
    outfile <<"-------------------Total penalty is ----------------"<<endl;
    outfile<<std::fixed<<std::setprecision(4)<<penalty_sum<<endl;

    outfile<< "\n";


    outfile.close();


    return 0;


}
