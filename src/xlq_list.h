#ifndef __XLQ_LIST__
#define __XLQ_LIST__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

struct xlq_str {
	long long m_value_8zj;	//other value score
	unsigned int m_count;	//key
	char* m_value;		//key member
	
	unsigned int m_count2;	//other value
	char* m_value2;		//other value
};

int xlq_str_compare(struct xlq_str* str1, struct xlq_str* str2);

size_t xlq_str_size(struct xlq_str* _str);

struct xlq_list_node {
	struct xlq_list_node* prev;
	struct xlq_list_node* next;
	struct xlq_str *m_list_value;
};
struct xlq_list_str {
	struct xlq_list_node* head;
	struct xlq_list_node* end;
	unsigned int size;
};

void xlq_str_free(struct xlq_str * _str);

size_t xlq_list_str_sizeof(struct xlq_list_str* _list);

int xlq_list_str_create(struct xlq_list_str** _outValue);

int xlq_list_str_insert_nx(struct xlq_list_str* _list, struct xlq_str* _str);

int xlq_list_str_insert(struct xlq_list_str* _list, struct xlq_str* _str);

int xlq_list_str_insert_all_nx(struct xlq_list_str* _desc, struct xlq_list_str* _src);

int xlq_list_str_insert_all(struct xlq_list_str* _desc, struct xlq_list_str* _src);

//删除，没找到返回-1，否则返回所在位置，同时把找到的xlq_list_node指出来到_outValue，因为可能可以复用
int xlq_list_str_del(struct xlq_list_str* _list, struct xlq_str* _str, struct xlq_str** _outValue);
//删除，没找到返回-1，否则返回所在位置，同时把找到的xlq_list_node指出来到_outValue，因为可能可以复用
int xlq_list_str_del2(struct xlq_list_str* _list, unsigned int _str_count, char* _str_value, struct xlq_str** _outValue);

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_index_of(struct xlq_list_str* _list, struct xlq_str* _str);

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_find(struct xlq_list_str* _list, struct xlq_str* _str, struct xlq_str** _outValue);

//查找，没找到返回-1，否则返回所在位置
int xlq_list_str_find2(struct xlq_list_str* _list, unsigned int _str_count, char* _str_value, struct xlq_str** _outValue);

void callback_list(struct xlq_str* m_list_value, int _array_size, struct xlq_list_str* arrays);

int xlq_list_foreach(struct xlq_list_str* _list, int _array_size, struct xlq_list_str* arrays, void (*callback_list)(struct xlq_str* m_list_value, int _array_size, struct xlq_list_str* arrays));

//free_xlq_str是否释放存储内容
int xlq_list_str_distory(struct xlq_list_str* _list, int free_xlq_str);

//清空list, 但不freelist; free_xlq_str是否释放存储内容
int xlq_list_str_clean(struct xlq_list_str* _list, int free_xlq_str);

#endif
