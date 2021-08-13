#ifndef __XLQ_LOOP_ARRAY__
#define __XLQ_LOOP_ARRAY__

struct xlq_looparray_t {
	unsigned short block_size;
	unsigned short array_size;
	unsigned short index_0;
	unsigned short index_next;
	//void* data;  ºóÃæ¸ú×Å
};

struct xlq_looparray_t* xlq_looparray_str_create(unsigned short _array_size);

void xlq_looparray_str_destory(struct xlq_looparray_t* _looparr, int freeStr);

size_t xlq_looparray_str_sizeof(struct xlq_looparray_t* _looparr);

int xlq_looparray_str_insert(struct xlq_looparray_t* _looparr, unsigned int _member_size, char* member_value);

int xlq_looparray_str_member_size(struct xlq_looparray_t* _looparr);

int xlq_looparray_str_range(struct xlq_looparray_t* _looparr, int start, int count, struct xlq_list_str* list);

struct xlq_looparray_t* xlq_looparray_str_resize(struct xlq_looparray_t* _looparr_src, unsigned short _new_array_size);

#endif