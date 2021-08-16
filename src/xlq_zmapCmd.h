#pragma once
#include "redismodule.h"
#include "xlq_zmap.h"

void zmaddCmd(void* _zmap, long long score, char* _member, size_t _memberLen, char* _value, size_t _valueLen, long long* scoreOri, unsigned int* _outValue_size, char** _outValue_value);

long long zmincrbyCmd(void* _zmap, long long score, char* _member, size_t _memberLen, char* _value, size_t _valueLen, unsigned int* _outValue_size, char** _outValue_value, int* _valueChg);

void zmrangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore, struct xlq_list_str* _list,
	void (*callback_zmap)(void*, struct xlq_str*), int offset, int count);

void zmrevrangeByScore(struct xlq_zmap_t* _zmap, long long maxScore, long long minScore, struct xlq_list_str* _list, 
	void (*callback_zmap)(void*, struct xlq_str*), int offset, int count);

int zm_getScoreAndValue(struct xlq_zmap_t* _zmap, size_t _member_size, char* _member_value, struct xlq_str** _outValue);

int zm_delByMember(struct xlq_zmap_t* _zmap, size_t _member_size, char* _member_value, struct xlq_str** _outValue);

/// ///////////////// FOR REDIS MODULE /////////////////////////////////////////////////

void* createZMapTypeObject();

void zmapSaveRdb(struct xlq_zmap_t* _zmap, RedisModuleIO* rdb);

void* zmapLoadRdb(RedisModuleIO* rdb);

size_t zmapUseMem(void* _zmap);

size_t zmapFree(void* _zmap);
