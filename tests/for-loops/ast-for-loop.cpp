#include <stdio.h>
#include <vector>

int main(){
	int a[] = {0,1,2,3,4,5};
	std::vector<int> vec(a, a+6);
	
	for(int i = 0; i < 6; i++){
		printf("%d\n", a[i]);
	}

	for(std::vector<int>::iterator iter = vec.begin(); iter != vec.end(); iter++){
		printf("%d\n", *iter);
	}

	for(int i = 0; i < vec.size(); i++){
		printf("%d", vec[i]);
	}	
}

