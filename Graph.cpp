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
    /*
    loadAttributes(attributesFilePath, idMapFilePath, nodes, attributeMap, userIdMap, bi_followers_count_max,
                   followers_count_max, friends_count_max, statuses_count_max, bi_followers_count_min,
                   followers_count_min,  friends_count_min, statuses_count_min
    );
    */
    /*
    loadAttributes(attributesFilePath, idMapFilePath, nodes1, attributeMap1, userIdMap, bi_followers_count_max, bi_followers_count_min
    );
    */
    loadGraph(graphFilePath, edges, prizes, terminals, nodeAttrMap,edgeAttributeMap,userIdMap);


}

void Graph::loadIdmap(std::string idMapFilePath, std::map<long,long> &userIdMap){
    std::ifstream test(idMapFilePath);
    if (!test)
    {
        std::cout << "The idMapfile doesn't exist" << std::endl;
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
            //printf("key is %d", key);
            //key = stol(currentLine[0]); //workable in the laptop
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

        /*
        for(int k=1; k<=attribute_index_number;++k){
            combCalculation(0,k,sourceId,sinkId,edges,edgeAttributeMap);
        }
        */

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


/*
void Graph::loadGraph(std::string graphFilePath, std::vector<std::pair<int, int> > &edges,
                          std::vector<double> &prizes, std::set<int> &terminals, std::map<long,NodeInfo> &attributeMap,
                      std::map<long,std::vector<int>> &edgeAttributeMap) {

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

        while (!infile.eof()) {

            getline(infile, STRING); // Saves the line in STRING.
            vector<string> currentLine = split(STRING, " ");


            int sourceId = stoi(currentLine[0]);
            int sinkId = stoi(currentLine[1]);

            //discard the edge if the mapped ID has no attributes
            std::map<long, Graph::NodeInfo>::iterator it;
            std::map<long, Graph::NodeInfo>::iterator it2;
            it = attributeMap.find(sourceId);
            it2 = attributeMap.find(sinkId);
            if((it== attributeMap.end())||(it2 == attributeMap.end())){
                continue;
            }


            for(int k=1; k<=attribute_index_number;++k){
                combCalculation(0,k,sourceId,sinkId,edges,edgeAttributeMap);
            }


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
*/

/*
void Graph::loadGraph(std::string graphFilePath, std::vector<std::pair<int, int> > &edges,
                      std::vector<double> &prizes, std::set<int> &terminals, std::map<long,Node1Info> &attributeMap1,
                      std::map<long,std::vector<int>> &edgeAttributeMap
) {

    printf("Start loading graph......");
    printf("\n");

    string STRING;
    ifstream infile;
    infile.open(graphFilePath);

    for (int i=0; i<attribute_index_number; ++i){
        attribute_index.push_back(i+1);
    }

    while (!infile.eof()) {

        getline(infile, STRING); // Saves the line in STRING.
        vector<string> currentLine = split(STRING, " ");


        int sourceId = stoi(currentLine[0]);
        int sinkId = stoi(currentLine[1]);

        //discard the edge if the mapped ID has no attributes
        std::map<long, Graph::Node1Info>::iterator it;
        std::map<long, Graph::Node1Info>::iterator it2;
        it = attributeMap1.find(sourceId);
        it2 = attributeMap1.find(sinkId);
        if((it== attributeMap1.end())||(it2 == attributeMap1.end())){
            continue;
        }



        for(int k=1; k<=attribute_index_number;++k){
            combCalculation(0,k,sourceId,sinkId,edges,edgeAttributeMap);
        }



    }


    infile.close();
    printf("Done loading graph......");
    printf("\n");
}
*/


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
        //printf("node_id is %ld \n", node_id);
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





void Graph::loadAttributes(std::string attributeFilePath, std::string idMapFilePath, std::vector<Graph::NodeInfo> &nodes,std::map<long,Graph::NodeInfo> &attributeMap,
                           std::map<long,long> &userIdMap, int &bi_followers_count_max,
                           int &followers_count_max, int &friends_count_max, int &statuses_count_max,
                           int& bi_followers_count_min, int& followers_count_min,
                           int& friends_count_min, int& statuses_count_min
){
    printf("start loading userId Map file and build the userIdMap");
    printf("\n");
    int temp_line_number = 0;
    string STRING;
    ifstream infile_map;
    long key;
    infile_map.open (idMapFilePath);

    while(!infile_map.eof()){
        getline(infile_map, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, "\r");
        key = stol(currentLine[0]);
        //userIdMap is guaranteed with no id duplication, we don't do the fact checking here
        userIdMap.insert(
                std::pair<long, int>(key, temp_line_number));
        temp_line_number++;
    }
    printf("Done building the userIdMap");
    printf("\n");

    printf("Start loading attributes......");
    printf("\n");

    //string STRING;
    ifstream infile;


    infile.open (attributeFilePath);
    bi_followers_count_max = 0;
    followers_count_max = 0;
    friends_count_max = 0;
    statuses_count_max = 0;
    bi_followers_count_min =INT_MAX;
    followers_count_min =INT_MAX;
    friends_count_min =INT_MAX;
    statuses_count_min =INT_MAX;

    long lineNumber =0;

    while(!infile.eof()) {

        getline(infile, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, " ");
        Graph::NodeInfo nodeInfo;

        nodeInfo.id = stol(currentLine[0]);
        std::map<long, long>::iterator it_idmap;
        it_idmap = userIdMap.find(nodeInfo.id);
        if(it_idmap == userIdMap.end()){
            lineNumber++;
            perror(
                    "Error: nodeInfo.id is not found in userIdMap\n");
        }else{

            int mapped_id = userIdMap[nodeInfo.id];
            //printf("Adding Profile for node %d", nodeInfo.id);
            //printf("\n");
            nodeInfo.bi_followers_count = stoi(currentLine[1]);
            if(nodeInfo.bi_followers_count > bi_followers_count_max){
                bi_followers_count_max = nodeInfo.bi_followers_count;
            }
            if(nodeInfo.bi_followers_count < bi_followers_count_min){
                bi_followers_count_min = nodeInfo.bi_followers_count;
            }
            nodeInfo.city_rank_within_province = currentLine[2];
            nodeInfo.verified = currentLine[3];
            //printf("nodeInfo.verified %s", nodeInfo.verified.c_str());
            // printf("\n");
            nodeInfo.followers_count = stoi(currentLine[4]);
            if(nodeInfo.followers_count > followers_count_max){
                followers_count_max = nodeInfo.followers_count;
            }
            if(nodeInfo.followers_count < followers_count_min ){
                followers_count_min = nodeInfo.followers_count;
            }
            nodeInfo.province = currentLine[5];
            //printf("nodeInfo.province %s", nodeInfo.province.c_str());
            //printf("\n");
            nodeInfo.city_name = currentLine[6];
            nodeInfo.province_id = currentLine[7];
            nodeInfo.friends_count = stoi(currentLine[8]);
            if(nodeInfo.friends_count > friends_count_max){
                friends_count_max = nodeInfo.friends_count;
            }
            if(nodeInfo.friends_count < friends_count_min){
                friends_count_min = nodeInfo.friends_count;
            }
            nodeInfo.gender = currentLine[9];
            nodeInfo.creation_time = currentLine[10];
            nodeInfo.verified_type = currentLine[11];
            nodeInfo.statuses_count = stoi(currentLine[12]);
            if(nodeInfo.statuses_count> statuses_count_max){
                statuses_count_max = nodeInfo.statuses_count;
            }
            if(nodeInfo.statuses_count< statuses_count_min){
                statuses_count_min = nodeInfo.statuses_count;
            }


            if(attributeMap.count(mapped_id)==0){
                lineNumber++;
                attributeMap.insert(
                        std::pair<long, cluster_approx::Graph::NodeInfo>(mapped_id, nodeInfo));
            }else{
                lineNumber++;
                //printf("Line Number is %d", lineNumber);
                //printf("Already processed profile for Node %d", mapped_id);
                //printf("\n");
                continue;
            }



        }
    }
    //printf("bi_followers_count_max is %d", bi_followers_count_max);
    //printf("\n");
    //printf("followers_count_max is %d", followers_count_max);
    //printf("\n");
    //printf("friends_count_max is %d", friends_count_max);
    //printf("\n");
    //printf("statuses_count_max is %d", statuses_count_max);
    //printf("\n");


    infile.close();
    printf("Done loading attributes......");
    printf("\n");



}

std::vector<std::string> Graph::split(std::string str, std::string s) {

        vector<string> vStr;
        boost::split(vStr, str, boost::is_any_of(s), boost::token_compress_on);
        return vStr;

}
void Graph::loadAttributes(std::string attributeFilePath, std::string idMapFilePath, std::vector<Graph::Node1Info> &nodes1,std::map<long,Graph::Node1Info> &attributeMap1,
                           std::map<long,long> &userIdMap, int &bi_followers_count_max, int &bi_followers_count_min
){
    printf("start loading userId Map file and build the userIdMap");
    printf("\n");
    int temp_line_number = 0;
    string STRING;
    ifstream infile_map;
    long key;
    infile_map.open (idMapFilePath);

    while(!infile_map.eof()){
        getline(infile_map, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, "\r");
        key = stol(currentLine[0]);
        //userIdMap is guaranteed with no id duplication, we don't do the fact checking here
        userIdMap.insert(
                std::pair<long, int>(key, temp_line_number));
        temp_line_number++;
    }
    printf("Done building the userIdMap");
    printf("\n");

    printf("Start loading attributes......");
    printf("\n");

    //string STRING;
    ifstream infile;


    infile.open (attributeFilePath);
    bi_followers_count_max = 0;
    bi_followers_count_min =INT_MAX;

    long lineNumber =0;

    while(!infile.eof()) {

        getline(infile, STRING); // Saves the line in STRING.
        vector<string> currentLine = Graph::split(STRING, " ");
        Graph::Node1Info nodeInfo1;

        nodeInfo1.id = stol(currentLine[0]);
        std::map<long, long>::iterator it_idmap;
        it_idmap = userIdMap.find(nodeInfo1.id);
        if(it_idmap == userIdMap.end()){
            lineNumber++;
            perror(
                    "Error: nodeInfo.id is not found in userIdMap\n");
        }else{

            int mapped_id = userIdMap[nodeInfo1.id];
            //printf("Adding Profile for node %d", nodeInfo1.id);
            //printf("\n");
            nodeInfo1.bi_followers_count = stoi(currentLine[1]);
            if(nodeInfo1.bi_followers_count > bi_followers_count_max){
                bi_followers_count_max = nodeInfo1.bi_followers_count;
            }
            if(nodeInfo1.bi_followers_count < bi_followers_count_min){
                bi_followers_count_min = nodeInfo1.bi_followers_count;
            }
            nodeInfo1.city_rank_within_province = currentLine[2];

            if(attributeMap1.count(mapped_id)==0){
                lineNumber++;
                attributeMap1.insert(
                        std::pair<long, cluster_approx::Graph::Node1Info>(mapped_id, nodeInfo1));
            }else{
                lineNumber++;
                //printf("Line Number is %d", lineNumber);
                //printf("Already processed profile for Node %d", mapped_id);
                //printf("\n");
                continue;
            }



        }
    }
    //printf("bi_followers_count_max is %d", bi_followers_count_max);
    //printf("\n");
    //printf("followers_count_max is %d", followers_count_max);
    //printf("\n");
    //printf("friends_count_max is %d", friends_count_max);
    //printf("\n");
    //printf("statuses_count_max is %d", statuses_count_max);
    //printf("\n");


    infile.close();
    printf("Done loading attributes......");
    printf("\n");


}


double Graph::computeEdgeWeight(Graph::NodeInfo sourceNode, Graph::NodeInfo sinkNode, int &bi_followers_count_max,
                                int &followers_count_max, int &friends_count_max, int &statuses_count_max,
                                int &bi_followers_count_min, int &followers_count_min, int &friends_count_min,
                                int &statuses_count_min,
                                std::vector<int> attribute_index_combination
) {

    double  dis[13]={};
    std::vector<int>::iterator it;
    for(int i=0;i<attribute_index_combination.size();i++){
        if(attribute_index_combination.at(i)==1){
            dis[1] = Graph::computeDis(sourceNode.bi_followers_count,sinkNode.bi_followers_count, bi_followers_count_max, bi_followers_count_min);
        }
        if(attribute_index_combination.at(i)==2) {
            dis[2] = Graph::computeDis(sourceNode.city_rank_within_province,sinkNode.city_rank_within_province);
        }
        if(attribute_index_combination.at(i)==3) {
            dis[3] = Graph::computeDis(sourceNode.verified, sinkNode.verified);
        }
        if(attribute_index_combination.at(i)==4){
            dis[4] = Graph::computeDis(sourceNode.followers_count,sinkNode.followers_count, followers_count_max,followers_count_min);
        }
        if(attribute_index_combination.at(i)==5){
            dis[5] = Graph::computeDis(sourceNode.province, sinkNode.province);
        }
        if(attribute_index_combination.at(i)==6){
            dis[6] = Graph::computeDis(sourceNode.city_name, sinkNode.city_name);
        }
        if(attribute_index_combination.at(i)==7){
            dis[7] = Graph::computeDis(sourceNode.province_id, sinkNode.province_id);
        }
        if(attribute_index_combination.at(i)==8){
            dis[8] = Graph::computeDis(sourceNode.friends_count, sinkNode.friends_count, friends_count_max,friends_count_min);
        }
        if(attribute_index_combination.at(i)==9){
            dis[9] = Graph::computeDis(sourceNode.gender, sinkNode.gender);
        }
        if(attribute_index_combination.at(i)==10){
            dis[10] = Graph::computeDis(sourceNode.creation_time, sinkNode.creation_time);
        }
        if(attribute_index_combination.at(i)==11){
            dis[11] = Graph::computeDis(sourceNode.verified_type, sinkNode.verified_type);
        }
        if(attribute_index_combination.at(i)==12) {
            dis[12] = Graph::computeDis(sourceNode.statuses_count, sinkNode.statuses_count, statuses_count_max,
                                        statuses_count_min);
        }
    }

    double weighted_value = 0;
    for (int i=1; i<=12;i++){
        weighted_value +=dis[i];
    }
    double weight = (weighted_value)/(0.0+ attribute_index_combination.size());
    return weight;



}

double Graph::computeEdgeWeight(Graph::Node1Info sourceNode, Graph::Node1Info sinkNode, int &bi_followers_count_max,
                                int &bi_followers_count_min,
                                std::vector<int> attribute_index_combination
) {

    double  dis[2] = {};
    std::vector<int>::iterator it;
    for(int i=0;i<attribute_index_combination.size();i++){
        if(attribute_index_combination.at(i)==1){
            dis[0] = Graph::computeDis(sourceNode.bi_followers_count,sinkNode.bi_followers_count, bi_followers_count_max, bi_followers_count_min);
        }
        if(attribute_index_combination.at(i)==2){
            dis[1] = Graph::computeDis(sourceNode.city_rank_within_province,sinkNode.city_rank_within_province);
        }

    }

    double weighted_value = 0;
    for (int i=0; i<2;i++){
        weighted_value +=dis[i];
    }
    double weight = (weighted_value)/(0.0+ attribute_index_combination.size());
    return weight;


}

double Graph::computeEdgeWeight(std::vector<double> sourceNode, std::vector<double> sinkNode, std::vector<AttributeInfo> attribute_stat,
                                std::vector<int> attribute_index_combination
) {
    std::vector<double> dis_value;
    std::vector<int>::iterator it;
    for(int i=0;i<attribute_index_combination.size();i++){

        double temp_dis = Graph::computeDis(sourceNode[attribute_index_combination[i]-1],sinkNode[attribute_index_combination[i]-1], attribute_stat[attribute_index_combination[i]-1].max_value,attribute_stat[attribute_index_combination[i]-1].min_value);
        dis_value.push_back(temp_dis);

    }
    double weighted_value = 0;
    std::vector<double>::iterator it2;
    for (it2=dis_value.begin(); it2!=dis_value.end();++it2){
        weighted_value +=*it2;
    }
    double weight = (weighted_value)/(0.0+ attribute_index_combination.size());
    printf("the edge weight is %f \n",weight);
    return weight;


}




double Graph::computeDis(std::string string1, std::string string2){
    if(string1==string2){
        return 0.0;
    }else{
        return 1.0;
    }
}

double Graph::computeDis(int integer1, int integer2, int& integerMax, int& integerMin){
    double weight = abs(integer1 - integer2)*1.0/abs(integerMax-integerMin);
    return weight;
}

double Graph::computeDis(double dou1, double dou2, double& doubleMax, double& doubleMin){
    double weight = fabs((dou1 - dou2)/(doubleMax-doubleMin));
    return weight;
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







