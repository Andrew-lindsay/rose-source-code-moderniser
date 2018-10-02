#include <iostream>
#include <map>
#include <string>

int sqr_int(int  x){
  return x*x;
}

int main(int argc, char *argv[]){
  //created map
  std::map<std::string,int> str_to_int;
  
  // pointer to a map
  
  std::map<std::string,int> var;
 
  int max_num = 10;

  int  max_num_sq = sqr_int(max_num);
  std::cout << "Max loop squared: "<< max_num << std::endl;
  while(max_num){
    std::cout << max_num-- << std::endl;
  }
  
  std::cout << "for test" << std::endl;

  int for_limit;
  for_limit = 10;
  double sum = 0.0;
  
  for(int i = 0; i < for_limit; i++ ){
    sum += i;
  }
  sum /= for_limit;
  
  std::cout <<  sum  << "\n";
  
  return 0;
}
