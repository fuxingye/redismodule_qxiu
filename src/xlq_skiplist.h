#ifndef __XLQ_SKIPLIST__
#define __XLQ_SKIPLIST__

#include "xlq_list.h"
#define ZSKIPLIST_MAXLEVEL 64 /* Should be enough for 2^64 elements */
#define ZSKIPLIST_P 0.25      /* Skiplist P = 1/4 */

/* ZSETs use a specialized version of Skiplists */
//struct xlq_skiplist_node_higher {
//    struct xlq_skiplist_node* next;
//    int currLevel;
//};

typedef struct xlq_skiplist_node {
    struct xlq_list_str *member;
    long long score;
    struct xlq_skiplist_node* prev;
    struct xlq_skiplist_node* next;
    short currLevel;
    short levels;   //本节点level数
    unsigned char isFirst;    //是不是第一个节点
} xlq_skiplist_node;

typedef struct xlq_skiplist {
    struct xlq_skiplist_node* head, * tail;
    unsigned int length;
    unsigned char level;    //当前skip最大level
} xlq_skiplist;


int xlq_skiplist_create(struct xlq_skiplist** _outValue);

int xlq_skiplist_distory(struct xlq_skiplist* _list);

size_t xlq_skiplist_sizeof(struct xlq_skiplist* _l);

int xlq_skiplist_insert(struct xlq_skiplist* _list, long long score, struct xlq_str* _member);

//按分，删除一个member一样的，free_xlq_str==1表示free，==0就得传出来了
int xlq_skiplist_del(struct xlq_skiplist* _list, long long _score, struct xlq_str* _member, int free_xlq_str, struct xlq_str** _outValue);

int xlq_skiplist_del_by_score(struct xlq_skiplist* _list, long long _score, int free_xlq_str, struct xlq_list_str* _outValue);

int xlq_skiplist_remrangeByIndex(struct xlq_skiplist* _list, int start, int end, int free_xlq_str, struct xlq_list_str* _outValue);

int xlq_skiplist_remrangeByScore(struct xlq_skiplist* _list, long long minScore, long long maxScore, int free_xlq_str, struct xlq_list_str* _outValue);

void xlq_skiplist_range_print(struct xlq_skiplist* _list, long long scoreBegin, long long scoreEnd, int printLevelMin, int printEveryLevel);

void xlq_skiplist_range_call(struct xlq_skiplist* _list, long long minScore, long long maxScore
    , void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count);

void xlq_skiplist_revrange_call(struct xlq_skiplist* _list, long long maxScore, long long minScore
    , void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count);
#endif