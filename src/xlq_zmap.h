#ifndef __XLQ_ZMAP__
#define __XLQ_ZMAP__

#include "xlq_skiplist.h"
#include "xlq_hashmap.h"

struct xlq_zmap_t {
	struct xlq_skiplist* m_skiplist;
	struct xlq_hashmap_t* m_hashmap;
	int length;
	size_t m_memberAndValueSize;
};

int xlq_zmap_create(struct xlq_zmap_t** _zmap);

int xlq_zmap_destory(struct xlq_zmap_t* _zmap);

size_t xlq_zmap_sizeof(struct xlq_zmap_t* _zmap);

long long  xlq_zmap_incrby(struct xlq_zmap_t* _zmap, long long score, unsigned int _member_size, char* _member_value, unsigned int _value_size, char* _value_value
	, unsigned int* _outValue_size, char** _outValue_value, int* _valueChg);

long long xlq_zmap_add(struct xlq_zmap_t* _zmap, long long score, unsigned int _member_size, char* _member_value, unsigned int _value_size, char* _value_value
	, long long* scoreOri, unsigned int* _outValue_size, char** _outValue_value);

int xlq_zmap_delByMember(struct xlq_zmap_t* _zmap, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue);

int xlq_zmap_delByScore(struct xlq_zmap_t* _zmap, long long _score, struct xlq_list_str* _outValue);

int xlq_zmap_delRangeByIndex(struct xlq_zmap_t* _zmap, int start, int end, int free_xlq_str, struct xlq_list_str* _outValue);

int xlq_zmap_delRangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore, int free_xlq_str, struct xlq_list_str* _outValue);

//原值给你了，别释放
int xlq_zmap_getScoreAndValue(struct xlq_zmap_t* _zmap, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue);

void xlq_zmap_rangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore
	, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count);

void xlq_zmap_revrangeByScore(struct xlq_zmap_t* _zmap, long long maxScore, long long minScore
	, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count);

#endif