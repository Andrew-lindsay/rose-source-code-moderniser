#include "rose.h"
#include <list>
#include <vector>
#include <string>

int main(int argc, char *argv[]){
  ROSE_INITIALIZE;
  
  std::vector<std::string> argv_plus(argv, argv + sizeof(argv)/ sizeof(argv[0]) );
  argv_plus.insert(++argv_plus.begin(),"-rose:skipfinalCompileStep");
  
  SgProject* project = frontend(argv_plus);
  
  std::list<SgNode*> queryResult;
  return backend(project);
}
