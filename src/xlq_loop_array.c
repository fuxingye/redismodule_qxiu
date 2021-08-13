#include "xlq_common.h"
#include "xlq_list.h"
#include "xlq_loop_array.h"
#include <string.h>
/****************************************************
*.hashmap
*.作者：付星烨 方超
*.时间：2021.5.16
*.描述：looparray
*****************************************************/

struct xlq_looparray_t* xlq_looparray_str_create(unsigned short _array_size) {

	struct xlq_looparray_t* arr = xlq_malloc(sizeof(struct xlq_looparray_t) + sizeof(struct xlq_str_simple) * _array_size);
	arr->array_size = _array_size;
	arr->block_size = 0;	//无意义
	arr->index_0 = 1;
	arr->index_next = 1;
	struct xlq_str_simple* d = ((char*)arr) + 8;
	d->m_count = 0;
	return arr;
}

void xlq_looparray_str_destory(struct xlq_looparray_t* _looparr, int freeStr) {
	struct xlq_str_simple* currNode = ((char*) _looparr) + 8;
	int sameMark = _looparr->index_0 == _looparr->index_next;
	if (currNode->m_count == 0 && sameMark)		// 空的
	{
		xlq_free(_looparr);
		return;
	}
	currNode += _looparr->index_0;
	int index = _looparr->index_0;

	for (int i = 0; i < _looparr->array_size; i++)
	{
		if (freeStr)
		{
			xlq_free(currNode->m_value);
		}
		
		index++;
		if (index > _looparr->array_size - 1)
		{
			index = 0;
			currNode = ((char*) _looparr) + 8;
		}
		else
		{
			currNode++;
		}

		if (i > 0 && index == _looparr->index_next)
		{
			break;
		}
	}
	xlq_free(_looparr);
}

size_t xlq_looparray_str_sizeof(struct xlq_looparray_t* _looparr) {
	if (_looparr == NULL)
	{
		return 0;
	}
	struct xlq_str_simple* currNode = ((char*) _looparr) + 8;
	int sameMark = _looparr->index_0 == _looparr->index_next;

	size_t rs = sizeof(struct xlq_looparray_t) + sizeof(struct xlq_str_simple) * _looparr->array_size;
	if (currNode->m_count == 0 && sameMark)		// 空的
	{
		return rs;
	}

	currNode += _looparr->index_0;
	int index = _looparr->index_0;

	for (int i = 0; i < _looparr->array_size; i++)
	{
		rs += currNode->m_count;

		index++;
		if (index > _looparr->array_size - 1)
		{
			index = 0;
			currNode = ((char*) _looparr) + 8;
		}
		else
		{
			currNode++;
		}

		if (i > 0 && index == _looparr->index_next)
		{
			break;
		}
	}
	return rs;
}

int xlq_looparray_str_insert(struct xlq_looparray_t* _looparr, unsigned int _member_size, char* member_value) {
	char* v1 = xlq_malloc(_member_size);
	memcpy(v1, member_value, _member_size);

	struct xlq_str_simple* d = ((char*) _looparr) + 8;
	if (d->m_count != 0)		// 满的
	{
		d += _looparr->index_next;
		d->m_count = _member_size;
		xlq_free(d->m_value);
		d->m_value = v1;

		_looparr->index_next++;
		if (_looparr->index_next > _looparr->array_size - 1)
		{
			_looparr->index_next = 0;
		}
		_looparr->index_0++;
		if (_looparr->index_0 > _looparr->array_size - 1)
		{
			_looparr->index_0 = 0;
		}
	}
	else
	{
		d += _looparr->index_next;
		d->m_count = _member_size;
		d->m_value = v1;

		_looparr->index_next++;
		if (_looparr->index_next > _looparr->array_size - 1)
		{
			_looparr->index_next = 0;
		}
	}
}

int xlq_looparray_str_member_size(struct xlq_looparray_t* _looparr) {

	struct xlq_str_simple* startNode = ((char*) _looparr) + 8;
	if (startNode->m_count == 0) {
		if (_looparr->index_next == _looparr->index_0)
		{
			return 0;
		}
		if (_looparr->index_next > _looparr->index_0)
		{
			return _looparr->index_next - _looparr->index_0;
		}
		return _looparr->array_size - 1;
	}
	return _looparr->array_size;

	/*if (_looparr->index_next > _looparr->index_0)
	{
		return _looparr->index_next - _looparr->index_0;
	}
	if (_looparr->index_next < _looparr->index_0)
	{
		return _looparr->array_size - 1;
	}
	struct xlq_str_simple* startNode = ((char*) _looparr) + 8;
	if (startNode->m_count == 0) {
		return 0;
	}
	return _looparr->array_size;*/
}

int xlq_looparray_str_range(struct xlq_looparray_t* _looparr, int start, int count, struct xlq_list_str * list) {

	struct xlq_str_simple* startNode = ((char*) _looparr) + 8;
	int sameMark = _looparr->index_0 == _looparr->index_next;
	if (startNode->m_count == 0 && sameMark)		// 空的
	{
		return;
	}

	int maxIndex = _looparr->array_size - 1;
	if (start > maxIndex)
	{
		return;
	}

	struct xlq_str_simple* currNode = startNode;
	
	int index = _looparr->index_0 + start;

	int cha = index - maxIndex;
	if (cha > 0)
	{
		currNode = startNode + cha - 1;
		index = 0 + cha - 1;
	}
	else
	{
		currNode += _looparr->index_0 + start;
	}

	int realCount = xlq_looparray_str_member_size(_looparr) - start;
	if (realCount > count)
	{
		realCount = count;
	}

	for (int i = 0; i < realCount; i++)
	{
		xlq_list_str_insert(list, currNode);
		index++;
		if (index > maxIndex)
		{
			index = 0;
			currNode = ((char*) _looparr) + 8;
		}
		else
		{
			currNode++;
		}

		if (index == _looparr->index_next)
		{
			break;
		}
	}
}

struct xlq_looparray_t* xlq_looparray_str_resize(struct xlq_looparray_t* _looparr_src, unsigned short _new_array_size) {
	struct xlq_looparray_t* arr_new = xlq_looparray_str_create(_new_array_size);

	struct xlq_str_simple* startNode = ((char*) _looparr_src) + 8;
	int sameMark = _looparr_src->index_0 == _looparr_src->index_next;
	if (startNode->m_count == 0 && sameMark)		// 空的
	{
		xlq_free(_looparr_src);
		return arr_new;
	}

	struct xlq_str_simple* currNode = startNode;

	int index = _looparr_src->index_0;
	currNode += _looparr_src->index_0;

	int maxIndex = _looparr_src->array_size - 1;

	int realCount = xlq_looparray_str_member_size(_looparr_src);

	for (int i = 0; i < realCount; i++)
	{
		xlq_looparray_str_insert(arr_new, currNode->m_count, currNode->m_value);
		index++;
		if (index > maxIndex)
		{
			index = 0;
			currNode = ((char*) _looparr_src) + 8;
		}
		else
		{
			currNode++;
		}

		if (index == _looparr_src->index_next)
		{
			break;
		}
	}
	xlq_free(_looparr_src);
	return arr_new;
}

//int xlq_looparray_str_revrange(struct xlq_looparray_t* _looparr, int start, int count, void* ctx, void (*callback_content)(void*, void*)) {
//
//	struct xlq_str_simple* currNode = _looparr->data;
//	int sameMark = _looparr->index_0 == _looparr->index_next;
//	if (currNode->m_count == 0 && sameMark)		// 空的
//	{
//		return;
//	}
//
//	if (_looparr->index_next > _looparr->index_0)
//	{
//		int realCount = _looparr->index_next - _looparr->index_0;
//		if (realCount > count)
//		{
//			realCount = count;
//		}
//		int index = _looparr->index_0 + start + realCount - 1;
//		currNode += _looparr->index_0 + start + realCount - 1;
//		for (int i = 0; i < realCount; i++)
//		{
//			callback_content(ctx, currNode);
//			index--;
//			currNode--;
//		}
//	}
//	else
//	{
//		int index = _looparr->index_0 + start + count - 1;
//		currNode += _looparr->index_0 + start + count - 1;
//		if (index > _looparr->array_size - 1) {
//			index = index - _looparr->array_size - 1;
//			currNode = currNode - _looparr->array_size;
//		}
//
//	}
//
//}