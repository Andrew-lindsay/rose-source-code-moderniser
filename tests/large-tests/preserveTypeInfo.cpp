//#include <vector>
#include <stdio.h>

int main(int argc, char *argv[]){

  // && handle rvalues ? e.g int&& a = A();
  
  //pointers don't matter, auto can infer it is a pointer from the type fine no need for 
  int num = 2;
  int* ptr_num = &num;
  int** ptr_ptr_num = &ptr_num;
  printf("num: %d \nptr_num: %d \nptr_ptr_num: %d \n", num, *ptr_num, **ptr_ptr_num);
  
  **ptr_ptr_num = 4;
  
  int& num_ref = num;
  printf("Num_ref: %d\n",num_ref);
  num_ref  = 1337;
  printf("num: %d \nptr_num: %d \nptr_ptr_num: %d \n", num, *ptr_num, **ptr_ptr_num);
  
  const int&  const_num_ref = num;
  const int& num_const = 2;
  volatile const int* ptr_num_const = &num_const;

  volatile const int * const *   prt_ptr_const_num = &ptr_num_const;
  unsigned c = 2;
  
  unsigned long long int super_l = 1; 
   
  //char strH[6] = "hello";
  //printf("%s",strH);
  
  // initalizer list not handled 
  //int a[2] = {1,2};
  //int ar1[2];
  
  //char x;
  //char*  y;
  //char* d[3] = {"a","b","c"} ;
  // std::vector<int> vec_1 = std::vector<int>();
  // vec_1[0] = 1;
  // vec_1[1]  =2 ;
  // for(auto & num : vec_1){
  //   printf("%d\n",num);
  // }
  return 0;
}
