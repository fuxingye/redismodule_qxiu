#include "xlq_common.h"

#include <stdio.h>
#include "redismodule.h"

int no = 0;
int addNum = 0;
int delNum = 0;

void* xlq_malloc(size_t _size) {
	no++;
	addNum++;
	return RedisModule_Alloc(_size);
}

void* xlq_calloc(size_t __nmemb, size_t __size) {
	no++;
	addNum++;
	return RedisModule_Calloc(__nmemb, __size);
}

void xlq_free(void* p) {
	no--;
	delNum++;
	RedisModule_Free(p);
}

void xlq_get_meminfo(int* _no, int* _addNum, int* _delNum) {
	*_no = no;
	*_addNum = addNum;
	*_delNum = delNum;
}

void xlq_print_memNO() {
	printf("\ncurrNum=%d\n", no);
	printf("\naddNum=%d\n", addNum);
	printf("\ndelNum=%d\n", delNum);
}
