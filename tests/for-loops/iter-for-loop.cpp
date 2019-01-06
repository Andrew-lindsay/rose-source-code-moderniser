
#include <vector>

int main(){
	std::vector<int> vec;
	vec.push_back(1);
	vec.push_back(2);
	int sum = 0;
	
	for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); iter++ ){
		sum += *iter;
	}
	
}
