#include <algorithm>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <vector>
#include <string>
#include <set>
#include <map>

#ifndef PCSTTEST_GRAPH_H
#define PCSTTEST_GRAPH_H

namespace cluster_approx {

    class Graph {


    public:

        std::vector<std::pair<std::pair<long, long>, long> > edges;
        std::vector<double> prizes;
        std::set<int> terminals;
        std::vector<double> costs;

        struct AttributeInfo{
            double min_value;
            double max_value;
            double mean_value;
        };

        struct Node1Info{
            int id;
            int bi_followers_count;
            std::string city_rank_within_province;
        };

        struct NodeInfo {
            int id;
            int bi_followers_count;
            std::string city_rank_within_province;
            std::string verified;
            int followers_count;
            std::string  province;
            std::string  city_name;
            std::string  province_id;
            int friends_count;
            std::string gender;
            std::string creation_time;
            std::string verified_type;
            int statuses_count;

        };

        struct Edge_Attribute_Info {
            long edge_id;
            std::vector<int> attribute_id;
        };

        std::map<long,std::vector<int>>edgeAttributeMap;

        std::map<long,cluster_approx::Graph::NodeInfo> attributeMap;
        std::map<long,cluster_approx::Graph::Node1Info> attributeMap1;

        std::map<long,long> userIdMap;

        int bi_followers_count_max;
        int followers_count_max;
        int friends_count_max;
        int statuses_count_max;

        int bi_followers_count_min;
        int followers_count_min;
        int friends_count_min;
        int statuses_count_min;

        std::vector<AttributeInfo>attribute_stat;
        std::map<long, std::vector<double>> nodeAttrMap;

        std::vector<NodeInfo> nodes;
        NodeInfo edgeAttributeSelection;

        std::vector<Node1Info> nodes1;
        Node1Info edgeAttributeSelection1;

        std::vector<int> attribute_index;
        std::vector<int> attribute_index_combination;

        //Just for testing
        const int attribute_index_number =8;
        //const int attribute_index_number =12;


        Graph(const std::string graphFilePath, const std::string attributesFilePath, const std::string idMapFilePath, const std::string penaltyFilePath);

        ~Graph();

        void loadIdmap(std::string idMapFilePath, std::map<long,long> &userIdMap);

        /*
        void loadGraph(std::string graphFilePath, std::vector<std::pair<int, int> > &edges,
                       std::vector<double> &prizes, std::set<int> &terminals, std::map<long,NodeInfo> &attributeMap,
                       std::map<long,std::vector<int>> &edgeAttributeMap);

        void loadGraph(std::string graphFilePath, std::vector<std::pair<int, int> > &edges,
                       std::vector<double> &prizes, std::set<int> &terminals, std::map<long,Node1Info> &attributeMap1,
                       std::map<long,std::vector<int>> &edgeAttributeMap)
        ;
        */
        void loadGraph(std::string graphFilePath, std::vector<std::pair<std::pair<long, long>, long> >&edges,
                std::vector<double> &prizes, std::set<int> &terminals, std::map<long, std::vector<double>>&nodeAttrMap,
                std::map<long,std::vector<int>> &edgeAttributeMap,
                std::map<long,long> &userIdMap );

        void loadAttributes(std::string attributesFilePath, std::vector<AttributeInfo> &attribute_stat,
                            std::map<long, std::vector<double>>&nodeAttrMap,
                            std::map<long,long> &userIdMap);

        void loadAttributes(std::string attributeFilePath, std::string idMapFilePath, std::vector<NodeInfo> &nodes, std::map<long,NodeInfo> &attributeMap, std::map<long,long> &userIdMap, int &bi_followers_count_max,
                            int &followers_count_max, int &friends_count_max, int &statuses_count_max, int& bi_followers_count_min, int& followers_count_min,
                            int& friends_count_min, int& statuses_count_min
        );


        void loadAttributes(std::string attributeFilePath, std::string idMapFilePath, std::vector<Node1Info> &nodes1, std::map<long,Node1Info> &attributeMap1, std::map<long,long> &userIdMap, int &bi_followers_count_max,
                            int &bi_followers_count_min);

        void loadPrizes(std::string prizesFilePath, std::vector<double> &prizes,std::set<int> &terminals);

        static double computeEdgeWeight(NodeInfo sourceNode, NodeInfo sinkNode, int &bi_followers_count_max,
                                 int &followers_count_max, int &friends_count_max, int &statuses_count_max,
                                 int &bi_followers_count_min, int &followers_count_min, int &friends_count_min,
                                 int &statuses_count_min,
                                        std::vector<int> attribute_index_combination);
        static double computeEdgeWeight(Node1Info sourceNode, Node1Info sinkNode, int &bi_followers_count_max,
                                        int &bi_followers_count_min,
                                        std::vector<int> attribute_index_combination
        );

        static double computeEdgeWeight(std::vector<double> sourceNode, std::vector<double> sinkNode, std::vector<AttributeInfo> attribute_stat,
                                 std::vector<int> attribute_index_combination);

        static double computeDis(std::string string1, std::string string2);

        static double computeDis(int integer1, int integer2, int& integerMax, int& integerMin);

        static double computeDis(double dou1, double dou2, double& doubleMax, double& doubleMin);

        void combCalculation(int offset, int k, const int sourceId, const int sinkId, std::vector<std::pair<std::pair<long, long>, long> >&edges,  std::map<long,std::vector<int>> &edgeAttributeMap, long origin_edge_id);

        void pretty_print (const std::vector<int> &v, const int sourceId, const int sinkId, std::vector<std::pair<std::pair<long, long>, long> > &edges, std::map<long,std::vector<int>> &edgeAttributeMap, long origin_edge_id);


    private:

        std::vector<std::string> split(std::string str, std::string s);

    };
}


#endif //PCSTTEST_GRAPH_H
