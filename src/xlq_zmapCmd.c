
#include <math.h>
#include "redismodule.h"
#include "xlq_zmap.h"
#include "xlq_common.h"

void zmaddCmd(void* _zmap, long long score, char* _member, size_t _memberLen, char* _value, size_t _valueLen, long long* scoreOri, unsigned int* _outValue_size, char** _outValue_value) {
	xlq_zmap_add(_zmap, score, _memberLen, _member, _valueLen, _value, scoreOri, _outValue_size, _outValue_value);
}

long long zmincrbyCmd(void* _zmap, long long score, char* _member, size_t _memberLen, char* _value, size_t _valueLen, unsigned int* _outValue_size, char** _outValue_value, int* _valueChg) {
	return xlq_zmap_incrby(_zmap, score, _memberLen, _member, _valueLen, _value, _outValue_size, _outValue_value, _valueChg);
}

void zmrangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore, struct xlq_list_str* _list,
				void (*callback_zmap)(void *, struct xlq_str*), int offset, int count) {
	xlq_zmap_rangeByScore(_zmap, minScore, maxScore, _list, callback_zmap, offset, count);
}

void zmrevrangeByScore(struct xlq_zmap_t* _zmap, long long maxScore, long long minScore, struct xlq_list_str * _list
			, void (*callback_zmap)(void *, struct xlq_str*), int offset, int count) {
	xlq_zmap_revrangeByScore(_zmap, minScore, maxScore, _list, callback_zmap, offset, count);
}

int zm_getScoreAndValue(struct xlq_zmap_t* _zmap, size_t _member_size, char* _member_value, struct xlq_str ** _outValue) {
	return xlq_zmap_getScoreAndValue(_zmap, _member_size, _member_value, _outValue);
}

int zm_delByMember(struct xlq_zmap_t* _zmap, size_t _member_size, char* _member_value, struct xlq_str** _outValue) {
	return xlq_zmap_delByMember(_zmap, _member_size, _member_value, _outValue);
}

/// ///////////////// FOR REDIS MODULE /////////////////////////////////////////////////

void* createZMapTypeObject() {
	struct xlq_zmap_t* o;
	xlq_zmap_create(&o);
	return o;
}

void callbackSave(RedisModuleIO* rdb, struct xlq_str* _str) {
	RedisModule_SaveSigned(rdb, _str->m_value_8zj);
	//RedisModule_SaveSigned(rdb, (int64_t) _str->m_count);
	RedisModule_SaveStringBuffer(rdb, _str->m_value, (size_t) _str->m_count);
	//RedisModule_SaveSigned(rdb, (int64_t) _str->m_count2);
	RedisModule_SaveStringBuffer(rdb, _str->m_value2, (size_t) _str->m_count2);
}

void zmapSaveRdb(struct xlq_zmap_t* _zmap, RedisModuleIO* rdb) {
	RedisModule_SaveSigned(rdb, _zmap->length);	//num of score
	xlq_skiplist_range_call(_zmap->m_skiplist, -9223372036854775808, 9223372036854775807, rdb, callbackSave, 0, 147483647);
}

void* zmapLoadRdb(RedisModuleIO* rdb) {	
	struct xlq_zmap_t* hto = (struct xlq_zmap_t*) createZMapTypeObject();
	int64_t size = RedisModule_LoadSigned(rdb);	//num of score
	long long scoreOri = 0;
	unsigned int outValue_size;
	char* outValue_value;
	while (size--) {
		long long score = RedisModule_LoadSigned(rdb);

		size_t memberLen = 0;
		char* member = RedisModule_LoadStringBuffer(rdb, &memberLen);
		size_t valueLen = 0;
		char* value = RedisModule_LoadStringBuffer(rdb, &valueLen);
		printf("score=%d,member=%s,v=%s", score, member, value);
		zmaddCmd(hto, score, member, memberLen, value, valueLen, &scoreOri, &outValue_size, &outValue_value);
		if (outValue_size > 0)
		{
			xlq_free(outValue_value);
		}
	}
	return hto;
}

size_t zmapUseMem(struct xlq_zmap_t* _zmap) {
	return xlq_zmap_sizeof(_zmap) + _zmap->m_memberAndValueSize;
}

size_t zmapFree(void* _zmap) {
	xlq_zmap_destory(_zmap);
}
