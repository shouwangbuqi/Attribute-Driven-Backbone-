#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <map>
#include <set>
#include <vector>

namespace cluster_approx {

template <typename ValueType, typename IndexType>
class PriorityQueue {
 public:
  PriorityQueue() {}

  bool is_empty() {
    return sorted_set.empty();
  }

  bool get_min(ValueType* value, IndexType* index) {
    if (sorted_set.empty()) {
      return false;
    }
    *value = sorted_set.begin()->first;//get the smallest cluster edge_part value
    *index = sorted_set.begin()->second;//get the smallest cluster ID
    return true;
  }

  bool delete_min(ValueType* value, IndexType* index) {
    if (sorted_set.empty()) {
      return false;
    }
    *value = sorted_set.begin()->first;
    *index = sorted_set.begin()->second;
    sorted_set.erase(sorted_set.begin());
    return true;
  }

  void insert(ValueType value, IndexType index) {
    if (index >= static_cast<int>(index_to_iterator.size())) {
      index_to_iterator.resize(index + 1);
    }
    index_to_iterator[index] = sorted_set.insert(std::make_pair(value, index)).first;
    //index_to_iterator actually stores the min-heap returned prize, vertexID pair from the QUEUE order

    /*
    typename std::vector<typename std::set<std::pair<ValueType, IndexType> >::iterator>::iterator it1;
    for (it1= index_to_iterator.begin(); it1!= index_to_iterator.end(); ++it1){
      double a=it1->operator*().first;
      int b=it1->operator*().second;
      std::cout<< a<<",";
      std::cout<<b;
      std::cout<<"\n";
    }
     */

    /*
    std::set<std::pair<double, int> >::iterator it;
    for (it = sorted_set.begin(); it != sorted_set.end(); it++) {
          std::cout<<"\n";
          double a= it->first;
          int b= it->second;
          std::cout<<a<<",";
          std::cout<<b;
          std::cout<<"\n";
        }
    std::cout<<"---------------";
    std::cout<<"\n";
    */
  }
  // The single element versions (1) return a pair, with its member pair::first set to an iterator pointing to either the newly inserted element or to the equivalent element already in the set.
  // index_to_iterator[index] just stores the vertex prize
  // sorted_set, where the first element of sorted_set is vertex prize, the second element of sorted_set is vertex index, vertex index from the smallest to largest
  void decrease_key(ValueType new_value, IndexType index) {
    sorted_set.erase(index_to_iterator[index]);
    index_to_iterator[index] =
        sorted_set.insert(std::make_pair(new_value, index)).first;
  }

  void delete_element(IndexType index) {
    sorted_set.erase(index_to_iterator[index]);

    std::set<std::pair<double, int> >::iterator it;
    //std::cout<<"delete_element, after deletion: \n";
      /*
    for (it = sorted_set.begin(); it != sorted_set.end(); it++) {

      double a= it->first;
      int b= it->second;
      std::cout<<a<<",";
      std::cout<<b;
      std::cout<<"\n";
    }
    std::cout<<"---------------";
    std::cout<<"\n";
    */
  }


 
 private:
  std::set<std::pair<ValueType, IndexType> > sorted_set;
  std::vector<typename std::set<std::pair<ValueType, IndexType> >::iterator>
      index_to_iterator;
};

}  // namespace cluster_approx

#endif
