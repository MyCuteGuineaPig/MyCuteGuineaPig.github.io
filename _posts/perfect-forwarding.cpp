#include <cstdio>
#include <vector>
#include <iostream>

using namespace std;

class object{
public:
  vector<int>_v;
  void set(const std::vector<int> & v) { 
      cout<<" lvalue "<<endl;
      _v = v;   
  }
  void set(std::vector<int> && v) { 
    cout<<" move "<<endl;
    _v = std::move(v); 
  }
  
  //Forwarding references only work with templates
  template<class T> //必须用template
  void perfectSet(T && t){
    set(std::forward<T>(t));   //argument t is always l-value
  }
};




int main()
{
    object v;
  	v.perfectSet<vector<int>>({1,2,3,4,5}); //call lvalue
  	vector<int> t = {1,2,3};
  	v.perfectSet(t);  //call rvalue
}