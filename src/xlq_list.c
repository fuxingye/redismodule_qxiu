#include <string.h>
#include "xlq_list.h"
#include "xlq_common.h"
/****************************************************
*.hashmap
*.作者：付星烨 方超
*.时间：2021.4.16
*.描述：list
*****************************************************/

int xlq_str_compare(struct xlq_str* str1, struct xlq_str* str2) {
	if (str1->m_count != str2->m_count)
	{
		return 1;
	}
	return memcmp(str1->m_value, str2->m_value, str1->m_count);
}

int xlq_str_compare2(struct xlq_str* str1, unsigned int _str_count, char* _str_value) {
	if (str1->m_count != _str_count)
	{
		return 1;
	}
	return memcmp(str1->m_value, _str_value, str1->m_count);
}

void xlq_str_free(struct xlq_str* _str) {
	if (_str->m_value != NULL)
	{
		xlq_free(_str->m_value);
	}
	if (_str->m_value2 != NULL)
	{
		xlq_free(_str->m_value2);
	}
	xlq_free(_str);
}

size_t xlq_str_size(struct xlq_str* _str) {
	return 8 * 5 + _str->m_count + _str->m_count2;
}

size_t xlq_list_str_sizeof(struct xlq_list_str* _list) {
	return sizeof(struct xlq_list_node) * _list->size + sizeof(struct xlq_list_str);
}

int xlq_list_str_create(struct xlq_list_str** _outValue) {
	*_outValue = (struct xlq_list_str*) xlq_malloc(sizeof(struct xlq_list_str));
	(*_outValue)->head = NULL;
	(*_outValue)->end = NULL;
	(*_outValue)->size = 0;
	return 1;
}

int xlq_list_str_insert_nx(struct xlq_list_str* _list, struct xlq_str* _str) {

	if (_list->head == NULL)
	{
		struct xlq_list_node* node = (struct xlq_list_node*) xlq_malloc(sizeof(struct xlq_list_node));
		node->m_list_value = _str;
		node->next = NULL;
		node->prev = NULL;
		_list->head = node;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		
		while (1) {
			if (xlq_str_compare(curr->m_list_value, _str) == 0)
			{
				return 0;
			}
			if (curr->next == NULL)
			{
				break;
			}
			curr = curr->next;
		}
		struct xlq_list_node* node = (struct xlq_list_node*) xlq_malloc(sizeof(struct xlq_list_node));
		node->m_list_value = _str;
		node->next = NULL;
		curr->next = node;
		node->prev = curr;
	}
	_list->size++;
	return 1;
}

int xlq_list_str_insert(struct xlq_list_str* _list, struct xlq_str* _str) {

	if (_list->head == NULL)
	{
		struct xlq_list_node* node = (struct xlq_list_node*) xlq_malloc(sizeof(struct xlq_list_node));
		node->m_list_value = _str;
		node->next = NULL;
		node->prev = NULL;
		_list->head = node;
		_list->end = node;
	}
	else {
		struct xlq_list_node* end = _list->end;

		struct xlq_list_node* node = (struct xlq_list_node*) xlq_malloc(sizeof(struct xlq_list_node));
		node->m_list_value = _str;
		node->next = NULL;
		node->prev = end;
		end->next= node;
		_list->end = node;
	}
	_list->size++;
	return 1;
}

int xlq_list_str_insert_all_nx(struct xlq_list_str* _desc, struct xlq_list_str* _src) {

	if (_src->size == 0)
	{
		return 0;
	}
	else {
		struct xlq_list_node* curr = _src->head;
		int num = 0;
		while (1) {
			int r = xlq_list_str_insert_nx(_desc, curr->m_list_value);
			if (r)
			{
				num++;
			}
			if (curr->next == NULL)
			{
				break;
			}
			curr = curr->next;
		}
		return num;
	}
}

int xlq_list_str_insert_all(struct xlq_list_str* _desc, struct xlq_list_str* _src) {

	if (_src->size == 0)
	{
		return 0;
	}
	else {
		struct xlq_list_node* curr = _src->head;
		int num = 0;
		while (1) {
			int r = xlq_list_str_insert(_desc, curr->m_list_value);
			if (r)
			{
				num++;
			}
			if (curr->next == NULL)
			{
				break;
			}
			curr = curr->next;
		}
		return num;
	}
}

//删除，没找到返回-1，否则返回所在位置，同时把找到的xlq_list_node指出来到_outValue，因为可能可以复用
int xlq_list_str_del(struct xlq_list_str* _list, struct xlq_str* _str, struct xlq_str** _outValue) {

	if (_list->head == NULL)
	{
		return -1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		int rs = -1;
		int index = 0;

		do {
			if (xlq_str_compare(curr->m_list_value, _str) == 0)
			{
				rs = index;
				if (curr->next != NULL)
				{
					if (curr->prev != NULL)
					{
						curr->prev->next = curr->next;
						curr->next->prev = curr->prev;
					}
					else
					{
						_list->head = curr->next;
						curr->next->prev = NULL;
					}
				}
				else
				{
					if (curr->prev != NULL)
					{
						curr->prev->next = NULL;
					}
					else
					{
						_list->head = NULL;
					}
				}
				(*_outValue) = curr->m_list_value;
				xlq_free(curr);
				break;
			}
			curr = curr->next;
			index++;
		} while (curr != NULL);
		if (rs >= 0)
		{
			_list->size--;
		}
		return rs;
	}
}

//删除，没找到返回-1，否则返回所在位置，同时把找到的xlq_list_node指出来到_outValue，因为可能可以复用
int xlq_list_str_del2(struct xlq_list_str* _list, unsigned int _str_count, char* _str_value, struct xlq_str** _outValue) {

	if (_list->head == NULL)
	{
		return -1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		int rs = -1;
		int index = 0;

		do {
			if (xlq_str_compare2(curr->m_list_value, _str_count, _str_value) == 0)
			{
				rs = index;
				if (curr->next != NULL)
				{
					if (curr->prev != NULL)
					{
						curr->prev->next = curr->next;
						curr->next->prev = curr->prev;
					}
					else
					{
						_list->head = curr->next;
						curr->next->prev = NULL;
					}
				}
				else
				{
					if (curr->prev != NULL)
					{
						curr->prev->next = NULL;
					}
					else
					{
						_list->head = NULL;
					}
				}
				(*_outValue) = curr->m_list_value;
				xlq_free(curr);
				break;
			}
			curr = curr->next;
			index++;
		} while (curr != NULL);
		if (rs >= 0)
		{
			_list->size--;
		}
		return rs;
	}
}

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_index_of(struct xlq_list_str* _list, struct xlq_str* _str) {

	if (_list->head == NULL)
	{
		return -1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		int rs = -1;
		int index = 0;

		do {
			if (xlq_str_compare(curr->m_list_value, _str) == 0)
			{
				rs = index;
				break;
			}
			curr = curr->next;
			index++;
		} while (curr != NULL);
		return rs;
	}
}

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_find(struct xlq_list_str* _list, struct xlq_str* _str, struct xlq_str** _outValue) {
	if (_list->head == NULL)
	{
		return -1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		int rs = -1;
		int index = 0;

		do {
			if (xlq_str_compare(curr->m_list_value, _str) == 0)
			{
				rs = index;
				*_outValue = curr->m_list_value;
				break;
			}
			curr = curr->next;
			index++;
		} while (curr != NULL);
		return rs;
	}
}

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_find2(struct xlq_list_str* _list, unsigned int _str_count, char* _str_value, struct xlq_str** _outValue) {
	if (_list->head == NULL)
	{
		return -1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		int rs = -1;
		int index = 0;

		do {
			if (xlq_str_compare2(curr->m_list_value, _str_count, _str_value) == 0)
			{
				rs = index;
				*_outValue = curr->m_list_value;
				break;
			}
			curr = curr->next;
			index++;
		} while (curr != NULL);
		return rs;
	}
}

int xlq_list_str_distory(struct xlq_list_str* _list, int free_xlq_str) {
	if (_list->head == NULL)
	{
		xlq_free(_list);
		return 1;
	}
	else {
		struct xlq_list_node* curr = _list->head;

		while (curr != NULL)
		{
			struct xlq_list_node* willDel = curr;
			curr = curr->next;

			if (free_xlq_str)
			{
				xlq_str_free(willDel->m_list_value);
			}
			xlq_free(willDel);
		}
		xlq_free(_list);
		return 1;
	}
}

int xlq_list_str_clean(struct xlq_list_str* _list, int free_xlq_str) {
	if (_list->head == NULL)
	{
		return 1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		struct xlq_list_node* currLast = NULL;
		int rs = -1;

		while (1)
		{
			if (curr->next == NULL)
			{
				if (free_xlq_str)
				{
					xlq_str_free(curr->m_list_value);
				}
				xlq_free(curr);
				break;
			}
			else {
				curr = curr->next;
				if (free_xlq_str)
				{
					xlq_str_free(curr->prev->m_list_value);
				}
				xlq_free(curr->prev);
			}
		}
		_list->size = 0;
		return 1;
	}
}

int xlq_list_foreach(struct xlq_list_str* _list, int _array_size, struct xlq_list_str* arrays, void (*callback_list)(struct xlq_str* m_list_value, int _array_size, struct xlq_list_str* arrays)) {
	if (_list->head == NULL)
	{
		return 1;
	}
	else {
		struct xlq_list_node* curr = _list->head;
		do {
			callback_list(curr->m_list_value, _array_size, arrays);
			curr = curr->next;
		} while (curr != NULL);
		return 1;
	}
}