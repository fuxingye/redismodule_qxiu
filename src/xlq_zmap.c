#include "xlq_zmap.h"
#include "xlq_skiplist.h"
#include "xlq_hashmap.h"
#include "xlq_common.h"

/****************************************************
*.zmap
*.作者：付星烨 方超
*.时间：2021.4.20
*.描述：zmap，对zset的增强，多存个value
*****************************************************/
int xlq_zmap_create(struct xlq_zmap_t** _zmap) {
	(*_zmap) = xlq_malloc(sizeof(struct xlq_zmap_t));
	(*_zmap)->length = 0;
	(*_zmap)->m_memberAndValueSize = 0;
	struct xlq_hashmap_t* map;
	xlq_hashmap_create(32, &map);
	(*_zmap)->m_hashmap = map;

	struct xlq_skiplist* skiplist;
	xlq_skiplist_create(&skiplist);
	(*_zmap)->m_skiplist = skiplist;
}

int xlq_zmap_destory(struct xlq_zmap_t* _zmap) {
	if (_zmap->m_hashmap != NULL) {
		xlq_hashmap_destory_nofreestr(_zmap->m_hashmap);
		xlq_skiplist_distory(_zmap->m_skiplist);
	}
	xlq_free(_zmap);
}

size_t xlq_zmap_sizeof(struct xlq_zmap_t* _zmap) {
	return xlq_skiplist_sizeof(_zmap->m_skiplist) + xlq_hashmap_sizeof(_zmap->m_hashmap) + sizeof(struct xlq_zmap_t);
}

long long  xlq_zmap_incrby(struct xlq_zmap_t* _zmap, long long score, unsigned int _member_size, char* _member_value, unsigned int _value_size, char* _value_value
		, unsigned int* _outValue_size, char** _outValue_value) {
	struct xlq_str* v;
	int findRs = xlq_hashmap_get2(_zmap->m_hashmap, _member_size, _member_value, &v);

	if (findRs)
	{
		*_outValue_size = v->m_count2;
		*_outValue_value = v->m_value2;
		if (0 == score)
		{
			if (_value_size > 0 && (v->m_count2 != _value_size || memcmp(v->m_value2, _value_value, _value_size) != 0))
			{
				v->m_count2 = _value_size;

				char* v1 = xlq_malloc(_value_size);
				memcpy(v1, _value_value, _value_size);
				v->m_value2 = v1;
			}
			return v->m_value_8zj;
		}

		struct xlq_str* v_skip;
		xlq_skiplist_del(_zmap->m_skiplist, v->m_value_8zj, v, 0, &v_skip);


		v_skip->m_value_8zj = v_skip->m_value_8zj + score;
		xlq_skiplist_insert(_zmap->m_skiplist, v_skip->m_value_8zj, v_skip);

		_zmap->m_memberAndValueSize += _value_size - v_skip->m_count2;

		if (_value_size > 0 && (v_skip->m_count2 != _value_size || memcmp(v_skip->m_value2, _value_value, _value_size) != 0)) {
			v_skip->m_count2 = _value_size;

			char* v1 = xlq_malloc(_value_size);
			memcpy(v1, _value_value, _value_size);
			v_skip->m_value2 = v1;
		}
		
		return v_skip->m_value_8zj;
	}
	else
	{
		_zmap->m_memberAndValueSize += 5 * 8 + _member_size + _value_size;

		struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
		char* v0 = xlq_malloc(_member_size);
		memcpy(v0, _member_value, _member_size);

		str0->m_count = _member_size;
		str0->m_value = v0;
		str0->m_value_8zj = score;
		str0->m_count2 = _value_size;

		char* v1 = xlq_malloc(_value_size);
		memcpy(v1, _value_value, _value_size);
		str0->m_value2 = v1;

		xlq_hashmap_put(_zmap->m_hashmap, str0);
		xlq_skiplist_insert(_zmap->m_skiplist, score, str0);
		_zmap->length++;
		return score;
	}
}

long long xlq_zmap_add(struct xlq_zmap_t* _zmap, long long score, unsigned int _member_size, char* _member_value, unsigned int _value_size, char* _value_value
		, long long* scoreOri, unsigned int* _outValue_size, char** _outValue_value) {
	struct xlq_str* v;
	int findRs = xlq_hashmap_get2(_zmap->m_hashmap, _member_size, _member_value, &v);

	if (findRs)
	{
		*scoreOri = v->m_value_8zj;
		*_outValue_size = v->m_count2;
		*_outValue_value = v->m_value2;
		if (v->m_value_8zj == score)
		{
			if (v->m_count2 != _value_size || memcmp(v->m_value2, _value_value, _value_size) != 0)
			{
				v->m_count2 = _value_size;

				char* v1 = xlq_malloc(_value_size);
				memcpy(v1, _value_value, _value_size);
				v->m_value2 = v1;
			}
		}
		else {
			struct xlq_str* v_skip;
			xlq_skiplist_del(_zmap->m_skiplist, v->m_value_8zj, v, 0, &v_skip);

			v_skip->m_value_8zj = score;
			xlq_skiplist_insert(_zmap->m_skiplist, v_skip->m_value_8zj, v_skip);

			_zmap->m_memberAndValueSize += _value_size - v_skip->m_count2;

			if (v->m_count2 != _value_size || memcmp(v->m_value2, _value_value, _value_size) != 0) {
				v_skip->m_count2 = _value_size;

				char* v1 = xlq_malloc(_value_size);
				memcpy(v1, _value_value, _value_size);
				v_skip->m_value2 = v1;
			}
		}
		return score;
	}
	else
	{
		_zmap->m_memberAndValueSize += 5 * 8 + _member_size + _value_size;

		struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
		char* v0 = xlq_malloc(_member_size);
		memcpy(v0, _member_value, _member_size);
		
		str0->m_count = _member_size;
		str0->m_value = v0;
		str0->m_value_8zj = score;

		str0->m_count2 = _value_size;

		char* v1 = xlq_malloc(_value_size);
		memcpy(v1, _value_value, _value_size);
		str0->m_value2 = v1;
		
		xlq_hashmap_put(_zmap->m_hashmap, str0);
		xlq_skiplist_insert(_zmap->m_skiplist, score, str0);
		_zmap->length++;
		return score;
	}
}

int xlq_zmap_delByMember(struct xlq_zmap_t* _zmap, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue) {
	struct xlq_str* v;
	int r = xlq_hashmap_del2(_zmap->m_hashmap, _member_size, _member_value, &v);
	if (r)
	{
		xlq_skiplist_del(_zmap->m_skiplist, v->m_value_8zj, v, 0, _outValue);
		_zmap->length--;
		_zmap->m_memberAndValueSize -= 5 * 8 + v->m_count + v->m_count2;
		return 1;
	}
	return 0;
}

int xlq_zmap_delByScore(struct xlq_zmap_t* _zmap, long long _score, struct xlq_list_str* _outValue) {
	int delNum = xlq_skiplist_del_by_score(_zmap->m_skiplist, _score, 0, _outValue);
	_zmap->length = _zmap->length - delNum;
	struct xlq_list_node* node = _outValue->head;
	while (node)
	{
		struct xlq_str* v;
		xlq_hashmap_del(_zmap->m_hashmap, node->m_list_value, &v);
		_zmap->m_memberAndValueSize -= 5 * 8 + v->m_count + v->m_count2;
		node = node->next;
	}
	return delNum;
}

int xlq_zmap_delRangeByIndex(struct xlq_zmap_t* _zmap, int start, int end, int free_xlq_str, struct xlq_list_str* _outValue) {
	struct xlq_list_str* dels;
	if (free_xlq_str)
	{
		xlq_list_str_create(&dels);
	}
	else
	{
		dels = _outValue;
	}

	xlq_skiplist_remrangeByIndex(_zmap->m_skiplist, start, end, 0, dels);
	int delNum = dels->size;
	if (dels->size > 0)
	{
		struct xlq_list_node* node = dels->head;
		struct xlq_str* tmp;
		while (1)
		{
			xlq_hashmap_del(_zmap->m_hashmap, node->m_list_value, &tmp);
			_zmap->m_memberAndValueSize -= 5 * 8 + tmp->m_count + tmp->m_count2;
			if (node->next == NULL)
			{
				break;
			}
			node = node->next;
		}
	}
	if (free_xlq_str)
	{
		xlq_list_str_distory(dels, 1);
	}
	_zmap->length = _zmap->length - delNum;

	return delNum;
}

int xlq_zmap_delRangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore, int free_xlq_str, struct xlq_list_str* _outValue) {
	struct xlq_list_str* dels;
	if (free_xlq_str)
	{
		xlq_list_str_create(&dels);
	}
	else
	{
		dels = _outValue;
	}
	
	xlq_skiplist_remrangeByScore(_zmap->m_skiplist, minScore, maxScore, 0, dels);
	int delNum = dels->size;
	if (dels->size > 0)
	{
		struct xlq_list_node* node = dels->head;
		struct xlq_str* tmp;
		while (1)
		{
			xlq_hashmap_del(_zmap->m_hashmap, node->m_list_value, &tmp);
			_zmap->m_memberAndValueSize -= 5 * 8 + tmp->m_count + tmp->m_count2;
			if (node->next == NULL)
			{
				break;
			}
			node = node->next;
		}
	}
	if (free_xlq_str)
	{
		xlq_list_str_distory(dels, 1);
	}
	_zmap->length = _zmap->length - delNum;
	return delNum;
}

//原值给你了，别释放
int xlq_zmap_getScoreAndValue(struct xlq_zmap_t* _zmap, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue) {
	return xlq_hashmap_get2(_zmap->m_hashmap, _member_size, _member_value, _outValue);
}

void xlq_zmap_rangeByScore(struct xlq_zmap_t* _zmap, long long minScore, long long maxScore
		, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count) {
	xlq_skiplist_range_call(_zmap->m_skiplist, minScore, maxScore, arg, callback_content, offset, count);
}

void xlq_zmap_revrangeByScore(struct xlq_zmap_t* _zmap, long long maxScore, long long minScore
		, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count) {
	xlq_skiplist_revrange_call(_zmap->m_skiplist, maxScore, minScore, arg, callback_content, offset, count);
}