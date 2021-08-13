#ifndef __XLQ_HASHMAP__
#define __XLQ_HASHMAP__

#include <string.h>
#include "xlq_list.h"

struct xlq_hashmap_t {
	unsigned int m_size;
	unsigned int array_size;
	struct xlq_list_str* arrays;
};

int xlq_hashmap_create(unsigned int initial_size, struct xlq_hashmap_t** _outValue);

int xlq_hashmap_destory(struct xlq_hashmap_t* _m);

int xlq_hashmap_destory_nofreestr(struct xlq_hashmap_t* _m);

size_t xlq_hashmap_sizeof(struct xlq_hashmap_t* _m);

int xlq_hashmap_put(struct xlq_hashmap_t* _m, struct xlq_str* _str);

int xlq_hashmap_get(struct xlq_hashmap_t* _m, struct xlq_str* _str, struct xlq_str** _outValue);

int xlq_hashmap_get2(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue);

int xlq_hashmap_del(struct xlq_hashmap_t* _m, struct xlq_str* _str, struct xlq_str** _outValue);

int xlq_hashmap_del2(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue);

int xlq_hashmap_incr(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, long long _incrValue, unsigned int _newValue_size, char* _newValue, struct xlq_str** _outValue);

int xlq_hashmap_get_all(struct xlq_hashmap_t* _m, struct xlq_list_str* _outValue);

#endif