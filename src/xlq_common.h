#ifndef __XLQ_COMMON__
#define __XLQ_COMMON__

#include <stdlib.h>
#include <stdio.h>
#include "redismodule.h"

struct xlq_str_simple {
	unsigned int m_count;	//key
	char* m_value;		//key member
};

void* xlq_malloc(size_t _size);

void* xlq_calloc(size_t __nmemb, size_t __size);

void xlq_free(void* p);

void xlq_get_meminfo(int* _no, int* _addNum, int* _delNum);

void xlq_print_memNO();



#endif