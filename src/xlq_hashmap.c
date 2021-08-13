#pragma once
#include <string.h>
#include "xlq_hashmap.h"
#include "xlq_common.h"
#include "xlq_list.h"
/****************************************************
*.hashmap
*.作者：付星烨 方超
*.时间：2021.4.16
*.描述：hashmap for zmap
*****************************************************/

unsigned int _hashCode(unsigned int _keySize, char* _key) {
	unsigned int h = 0, i = 0;
	for (i = 0; i < _keySize; i++) {
		h = 31 * h + (unsigned int)(*(_key + i));
	}
	return h;
}
unsigned int _hash(unsigned int hashCode) {
	if (hashCode == 0)
	{
		return 0;
	}
	unsigned int h = hashCode;
	return h ^ (h >> 16);
}

unsigned int MAXIMUM_CAPACITY = 1073741824;
unsigned int xlq_tableSizeFor(int cap) {
	unsigned int rs = 4;
	while (1)
	{
		if (rs >= cap) {
			break;
		}
		rs = rs * 2;
	}
	if (rs > MAXIMUM_CAPACITY)
	{
		return MAXIMUM_CAPACITY;
	}
	return rs;
}

void callback_list(struct xlq_str* _str, int _array_size, struct xlq_list_str* arrays) {
	unsigned int hash = _hash(_hashCode(_str->m_count, _str->m_value));
	int index = (_array_size - 1) & hash;
	xlq_list_str_insert_nx((arrays + index), _str);
}

size_t xlq_hashmap_sizeof(struct xlq_hashmap_t* _m) {
	size_t r = sizeof(struct xlq_hashmap_t);
	struct xlq_list_str * list = _m->arrays;
	for (int i = 0; i < _m->array_size; i++)
	{
		r += xlq_list_str_sizeof(list);
		list++;
	}
	return r;
}

int xlq_hashmap_create(unsigned int initial_size, struct xlq_hashmap_t** _outValue) {
	//
	struct xlq_hashmap_t* m = (struct xlq_hashmap_t*) xlq_malloc(sizeof(struct xlq_hashmap_t));
	m->array_size = xlq_tableSizeFor(initial_size);
	m->arrays = (struct xlq_list_str*) xlq_calloc(1, sizeof(struct xlq_list_str) * m->array_size);

	m->m_size = 0;
	*_outValue = m;
	return 1;
}

int xlq_hashmap_destory(struct xlq_hashmap_t* _m) {
	struct xlq_list_str* arrNode = _m->arrays;
	for (int i = 0; i < _m->array_size; i++)
	{
		xlq_list_str_clean(arrNode, 1);
		arrNode++;
	}
	xlq_free(_m->arrays);
	xlq_free(_m);
}

int xlq_hashmap_destory_nofreestr(struct xlq_hashmap_t* _m) {
	struct xlq_list_str* arrNode = _m->arrays;
	for (int i = 0; i < _m->array_size; i++)
	{
		xlq_list_str_clean(arrNode,0);
		arrNode++;
	}
	xlq_free(_m->arrays);
	xlq_free(_m);
}

int xlq_hashmap_put(struct xlq_hashmap_t* _m, struct xlq_str* _str) {
	unsigned int hash = _hash(_hashCode(_str->m_count, _str->m_value));
	int index = (_m->array_size - 1) & hash;
	_m->m_size++;
	if (_m->m_size > _m->array_size * 0.75)	//扩容
	{
		int array_size_tmp = _m->array_size * 2;
		struct xlq_list_str* arrays_tmp = (struct xlq_list_str*) xlq_calloc(1, sizeof(struct xlq_list_str) * array_size_tmp);

		for (unsigned int i = 0; i < _m->array_size; i++)
		{
			//struct xlq_list_str arrNode = _m->arrays[i];
			struct xlq_list_str* arrNode = &(_m->arrays[i]);
			xlq_list_foreach(arrNode, array_size_tmp, arrays_tmp, callback_list);
			xlq_list_str_clean(arrNode, 0);
		}
		xlq_free(_m->arrays);

		_m->arrays = arrays_tmp;
		_m->array_size = array_size_tmp;

	}
	if (xlq_list_str_insert_nx((_m->arrays + index), _str) == 0) {
		_m->m_size--;	//key已存在
	}

	return 1;
}

int xlq_hashmap_get(struct xlq_hashmap_t* _m, struct xlq_str* _str, struct xlq_str** _outValue) {
	unsigned int hash = _hash(_hashCode(_str->m_count, _str->m_value));
	int index = (_m->array_size - 1) & hash;
	int r = xlq_list_str_find((_m->arrays + index), _str, _outValue);
	return r >= 0;
}

int xlq_hashmap_get2(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue) {
	unsigned int hash = _hash(_hashCode(_member_size, _member_value));
	int index = (_m->array_size - 1) & hash;
	int r = xlq_list_str_find2((_m->arrays + index), _member_size, _member_value, _outValue);
	return r >= 0;
}

int xlq_hashmap_del(struct xlq_hashmap_t* _m, struct xlq_str* _str, struct xlq_str** _outValue) {
	unsigned int hash = _hash(_hashCode(_str->m_count, _str->m_value));
	int index = (_m->array_size - 1) & hash;
	int r = xlq_list_str_del((_m->arrays + index), _str, _outValue);
	if (r >= 0)
	{
		_m->m_size--;
	}
	return r >= 0;
}

int xlq_hashmap_del2(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, struct xlq_str** _outValue) {
	unsigned int hash = _hash(_hashCode(_member_size, _member_value));
	int index = (_m->array_size - 1) & hash;
	int r = xlq_list_str_del2((_m->arrays + index), _member_size, _member_value, _outValue);
	if (r >= 0)
	{
		_m->m_size--;
	}
	return r >= 0;
}

int xlq_hashmap_incr(struct xlq_hashmap_t* _m, unsigned int _member_size, char* _member_value, long long _incrValue, unsigned int _newValue_size, char* _newValue, struct xlq_str** _outValue) {
	unsigned int hash = _hash(_hashCode(_member_size, _member_value));
	int index = (_m->array_size - 1) & hash;
	int r = xlq_list_str_find2((_m->arrays + index), _member_size, _member_value, _outValue);
	if (r >= 0)
	{
		(*_outValue)->m_value_8zj += _incrValue;
		if (_newValue_size > 0)
		{
			(*_outValue)->m_count2 = _newValue_size;
			xlq_free((*_outValue)->m_value2);
			(*_outValue)->m_value2 = _newValue;
		}
		return 1;
	}
	return 0;
}

int xlq_hashmap_get_all(struct xlq_hashmap_t* _m, struct xlq_list_str* _outValue) {
	struct xlq_list_str* list;
	int r = 0;
	for (size_t i = 0; i < _m->array_size; i++)
	{
		list = _m->arrays + i;
		if (list->size > 0)
		{
			xlq_list_str_insert_all_nx(_outValue, list);
			r = i;
		}
	}
	return r;
}