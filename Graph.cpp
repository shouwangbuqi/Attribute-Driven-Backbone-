#include "Graph.h"
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <algorithm>
#include <cmath>

using cluster_approx::Graph;
using std::make_pair;
using std::vector;
using std::iostream;
using std:: ofstream;
using std::ifstream;
using std::string;
using std::cout;
using std::vector;
using std::endl;
using std::pair;


Graph::Graph(const std::string graphFilePath, const std::string attributesFilePath, const std::string idMapFilePath, const std::string penaltyFilePath) {


    loadIdmap(idMapFilePath,userIdMap);

    loadPrizes(penaltyFilePath,prizes,terminals);


    loadAttributes(attributesFilePath, attribute_stat, nodeAttrMap, userIdMap);

    loadGraph(graphFilePath, edges, prizes, terminals, nodeAttrMap,edgeAttributeMap,userIdMap);


}

void Graph::loadIdmap(std::string idMapFilePath, std::map<long,long> &userIdMap){
    std::ifstream test(idMapFilePath);
    if (!test)
    {
        std::cout << "The idMapfile doesn't exist" << std::endl;
        throw std::invalid_argument("The idMapfile doesn't exist");
    }
    printf("start loading userId Map file and build the userIdMap");

    printf("\n");
    int temp_line_number = 0;
    string STRING;
    ifstream infile_map;
    long key = 0;
    infile_map.open (idMapFilePath);

    while(!infile_map.eof()){
        getline(infile_map, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, "\r");
        try{
            key = stol(currentLine[0]);
        }
        catch (const std::exception &e){
            printf("current line is %d",temp_line_number);
            printf("\n");
        }

        //userIdMap is guaranteed with no id duplication, we don't do the fact checking here
        userIdMap.insert(
                std::pair<long, int>(key, temp_line_number));
        temp_line_number++;
    }

    printf("Done building the userIdMap");
    printf("\n");

}

void Graph::loadPrizes(std::string prizesFilePath, std::vector<double> &prizes,std::set<int> &terminals){
    printf("Start loading prizes......");
    printf("\n");

    string STRING;
    ifstream infile;
    infile.open(prizesFilePath);

    long temp_line_number = 0;

    while (!infile.eof()) {

        getline(infile, STRING); // Saves the line in STRING.

        vector<string> currentLine = Graph::split(STRING, " ");


        int node_id = temp_line_number;
        double prize = stod(currentLine[1]);
        prizes.push_back(prize);
        if(prize > 0){
            terminals.insert(node_id);
        }
	temp_line_number++;

    }

    infile.close();
    printf("Done loading prizes......");
    printf("\n");

}

void Graph::loadGraph(std::string graphFilePath, std::vector<std::pair<std::pair<long, long>, long> > &edges,
                      std::vector<double> &prizes, std::set<int> &terminals, std::map<long, std::vector<double>>&nodeAttrMap,
                      std::map<long,std::vector<int>> &edgeAttributeMap,
                      std::map<long,long> &userIdMap ){
    printf("Start loading graph......");
    printf("\n");

    string STRING;
    ifstream infile;
    infile.open(graphFilePath);
    clock_t edge_trans_startTime,edge_trans_endTime;
    edge_trans_startTime = clock();

    for (int i=0; i<attribute_index_number; ++i){
        attribute_index.push_back(i+1);
    }

    long origin_edge_id = 0;

    while (!infile.eof()) {
        getline(infile, STRING); // Saves the line in STRING.
        vector<string> currentLine = split(STRING, " ");
        long origin_sourceId = stol(currentLine[0]);
        long origin_sinkId = stol(currentLine[1]);
        long sourceId = userIdMap[origin_sourceId];
        long sinkId = userIdMap[origin_sinkId];

        std::map<long, std::vector<double>>::iterator it;
        std::map<long, std::vector<double>>::iterator it2;
        it = nodeAttrMap.find(sourceId);
        it2 = nodeAttrMap.find(sinkId);
        if ((it == nodeAttrMap.end()) || (it2 == nodeAttrMap.end())) {
            origin_edge_id++;
            continue;
        }

        //printf("sourceId is %d, sinkId is %d \n", sourceId, sinkId);
        edges.push_back(std::make_pair(std::make_pair(sourceId, sinkId),origin_edge_id));
        //printf("edge id is %d \n",edges.size()-1);
        //printf("origin edge id is %d \n",edges[edges.size()-1].second);
        origin_edge_id++;
    }
    edge_trans_endTime = clock();

    ofstream outfile;
    outfile.open("result_edge_comp.txt");

    outfile <<"------------------------Final result-------------------------------------" << endl;
    outfile << "Total Running Time : " <<(double)(edge_trans_endTime - edge_trans_startTime) / CLOCKS_PER_SEC << "s" << endl;

    infile.close();
    printf("Done loading graph......");
    printf("\n");

}

void Graph::loadAttributes(std::string attributeFilePath, std::vector<AttributeInfo>&attribute_stat,
                           std::map<long, std::vector<double>>&nodeAttrMap,
                           std::map<long,long> &userIdMap){

    std::ifstream test2(attributeFilePath);
    if (!test2)
    {
        std::cout << "The attributeFile doesn't exist" << std::endl;
    }

    printf("Start loading attributes......");
    printf("\n");

    string STRING;
    ifstream infile;
    infile.open (attributeFilePath);

    for (int i = 0; i< attribute_index_number; i++){
        AttributeInfo singleAttribute;
        singleAttribute.min_value = INT_MAX*1.0;
        singleAttribute.max_value = 0.0;
        singleAttribute.mean_value = 0.0;
        attribute_stat.push_back(singleAttribute);
    }

    long lineNumber =0;

    while(!infile.eof()) {

        getline(infile, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, " ");
        std::vector<double> attribute_values;
        long node_id = stol(currentLine[0]);
        //printf("node_id is %ld \n", node_id);´
        long mapped_id = userIdMap[node_id];
        //printf("mapped_node_id is %ld \n",mapped_id);
        for (int i = 1; i <= attribute_index_number; i++) {
            double attribute_value = stod(currentLine[i]);
            //printf("attribute_value is %f \n", attribute_value);
            if (attribute_value < attribute_stat[i - 1].min_value) {
                attribute_stat[i - 1].min_value = attribute_value;
            }
            if (attribute_value > attribute_stat[i - 1].max_value) {
                attribute_stat[i - 1].max_value = attribute_value;
            }
            attribute_values.push_back(attribute_value);
        }

        if (nodeAttrMap.count(mapped_id) == 0) {
            lineNumber++;
            nodeAttrMap.insert(std::pair<long, std::vector<double>>(mapped_id, attribute_values));
        } else {
            lineNumber++;
            continue;
        }
    }

    infile.close();
    printf("Done loading attributes......");
    printf("\n");

    /*
    int i =0;
    std::vector<AttributeInfo>::iterator it2;
    for (it2=attribute_stat.begin(); it2!=attribute_stat.end();++it2){
        printf("attribute_stat i is %d, min_val is %f, max_val is %f \n",i, (*it2).min_value, (*it2).max_value);
        i++;
    }
    */
}







std::vector<std::string> Graph::split(std::string str, std::string s) {

        vector<string> vStr;
        boost::split(vStr, str, boost::is_any_of(s), boost::token_compress_on);
        return vStr;

}














void Graph::pretty_print(const vector<int>& v, const int sourceId, const int sinkId,std::vector<std::pair<std::pair<long, long>, long> > &edges,
                         std::map<long,std::vector<int>> &edgeAttributeMap, long origin_edge_id
) {

    /*
    static int count = 0;
    cout << "combination no " << (++count) << ": [ ";
    for (int i = 0; i < v.size(); ++i) { cout << v[i] << " "; }
    cout << "] " << endl;
    */


    printf("sourceId is %d, sinkId is %d \n", sourceId, sinkId);
    edges.push_back(std::make_pair(std::make_pair(sourceId, sinkId),origin_edge_id));
    printf("edge id is %d \n",edges.size()-1);
    printf("origin edge id is %d \n",edges[edges.size()-1].second);
    edgeAttributeMap.insert(
            std::pair<long, std::vector<int>>(edges.size()-1, v));

}

void Graph::combCalculation(int offset, int k, const int sourceId, const int sinkId,std::vector<std::pair<std::pair<long, long>, long> > &edges,
                            std::map<long,std::vector<int>> &edgeAttributeMap, long origin_edge_id
) {
    if (k == 0) {
        pretty_print(attribute_index_combination, sourceId, sinkId,edges,edgeAttributeMap,origin_edge_id);
        return;
    }
    for (int i = offset; i <= attribute_index.size() - k; ++i) {
        attribute_index_combination.push_back(attribute_index[i]);
        Graph::combCalculation(i+1, k-1, sourceId, sinkId,edges,edgeAttributeMap, origin_edge_id);
        attribute_index_combination.pop_back();
    }
}







