#include "xlq_skiplist.h"
#include "xlq_common.h"
#include <string.h>

#include <math.h>
/****************************************************
*.skiplist
*.作者：付星烨 方超
*.时间：2021.4.20
*.描述：手撕跳表
*****************************************************/
int xlq_skiplist_create(struct xlq_skiplist** _outValue) {
	struct xlq_skiplist* m = (struct xlq_skiplist*) xlq_malloc(sizeof(struct xlq_skiplist));
	m->head = NULL;
	m->tail = NULL;
	m->length = 0;
	m->level = 0;
	*_outValue = m;
	return 1;
}

int xlq_skiplist_distory(struct xlq_skiplist* _list) {

	if (_list->length > 0)
	{
		struct xlq_skiplist_node* currNode = _list->head;
		while (1)
		{
			xlq_list_str_distory(currNode->member, 1);
			struct xlq_skiplist_node* willDelNode = currNode;
			currNode = currNode->next;
			xlq_free(willDelNode);
			if (currNode == NULL)
			{
				break;
			}
		}
	}
	
	xlq_free(_list);
	return 1;
}

size_t xlq_skiplist_sizeof(struct xlq_skiplist* _l) {
	size_t r = sizeof(struct xlq_skiplist);
	if (_l->length == 0)
	{
		return r;
	}
	struct xlq_skiplist_node* n = _l->head;
	while (n != NULL)
	{
		r += sizeof(struct xlq_skiplist_node) * n->levels;
		r += xlq_list_str_sizeof(n->member);
		n = n->next;
	}
	return r;
}

xlq_skiplist_node* create_node(int _level, long long _score, struct xlq_str* _member, char isFirst) {
	struct xlq_skiplist_node * n0 = xlq_malloc(sizeof(struct xlq_skiplist_node) * _level);	//0层 + 拉高的level-1层
	struct xlq_list_str* list;
	int r = xlq_list_str_create(&list);
	xlq_list_str_insert_nx(list, _member);

	for (int i = 0; i < _level; i++)
	{
		struct xlq_skiplist_node* n = (n0 + i);
		n->levels = _level;
		n->currLevel = i;
		n->next = NULL;
		n->prev = NULL;
		n->score = _score;
		n->member = list;
		n->isFirst = isFirst;
	}
	return n0;
}

xlq_skiplist_node* find_node_by_level(xlq_skiplist_node* n0, int _level) {
	return (n0 + _level);
}

int find_randomLevel() {
	int level = 1;
	while ((random() & 0xFFFF) < (ZSKIPLIST_P * 0xFFFF))
		level += 1;
	return (level < ZSKIPLIST_MAXLEVEL) ? level : ZSKIPLIST_MAXLEVEL;
}

int xlq_skiplist_insert(struct xlq_skiplist* _list, long long _score, struct xlq_str* _member) {

	if (_list->head == NULL)
	{
		xlq_skiplist_node* n = create_node(ZSKIPLIST_MAXLEVEL, _score, _member, 1);
		_list->head = n;
		_list->length++;
		_list->level = 1;
		_list->tail = n;
	}
	else {
		int list_ori_level_index = _list->level - 1;
		struct xlq_skiplist_node* levelsRecord[_list->level];
		int level_highest = _list->level - 1;
		
		struct xlq_skiplist_node* currNode = _list->head;
		currNode = find_node_by_level(currNode, level_highest);
		for (int seLevel = level_highest; seLevel >= 0; seLevel--)
		{
			while (1)
			{

				if (currNode->next == NULL || currNode->next->score > _score) {
					//降层
					levelsRecord[seLevel] = currNode;
					if (seLevel == 0)
					{
						break;
					}
					currNode = currNode - 1;
					break;
				}
				currNode = currNode->next;
			}
		}

		if (currNode->score == _score)
		{
			if (xlq_list_str_insert_nx(currNode->member, _member)) {
				_list->length++;
			}
			else
			{
				return 0;
			}
		}
		else {
			int levelThisNode = 1;
			if (_list->length > 16) {	//长度太小没必要拉高
				levelThisNode = find_randomLevel();
			}
			
			//struct xlq_skiplist_node* insertNode = create_node(levelThisNode, _score, _member, 0);
			struct xlq_skiplist_node* aNode = find_node_by_level(_list->head, 0);
			int insert1Zhu = 0;
			if (aNode->score > _score && aNode->prev == NULL) {	//还是要插在1柱前面的
				insert1Zhu = 1;
			}

			int high_1 = _list->head->levels > _list->level ? _list->level : _list->head->levels;
			if (levelThisNode > _list->level)
			{
				if (levelThisNode - _list->level > 1)	//一次只能升一层
				{
					levelThisNode = _list->level + 1;
				}
			}
			struct xlq_skiplist_node* insertNode = create_node(levelThisNode, _score, _member, 0);

			struct xlq_skiplist_node* insertNode0 = insertNode;

			int pillarBuQi = 0;
			struct xlq_list_str* pillarBuQiMember = NULL;
			if (levelThisNode < _list->level)	//如果发现，本骰子小于最高level
			{
				//struct xlq_skiplist_node* aNode = find_node_by_level(_list->head, 0);
				//if (aNode->score > _score && aNode->prev == NULL)	//并且，还是要插在1柱前面的
				//{
				//	levelThisNode = _list->level;	//这是个笨办法，导致如果当每次插入的分都依次减小时，层数会特别高
				//}
				if (insert1Zhu)	//并且，还是要插在1柱前面的
				{
					pillarBuQi = _list->level - levelThisNode;
					pillarBuQiMember = insertNode->member;	//
				}
			}

			int upLevel = 0;
			if (levelThisNode > _list->level)
			{
				_list->level = levelThisNode;
				upLevel = 1;
			}


			for (int levelWrite = 0; levelWrite < levelThisNode; levelWrite++)
			{
				struct xlq_skiplist_node* aNode;
				if (levelWrite <= list_ori_level_index)
				{
					aNode = levelsRecord[levelWrite];
				}
				else
				{
					aNode = find_node_by_level(_list->head, levelWrite);
					//补齐1柱数据
					struct xlq_skiplist_node* head0 = find_node_by_level(_list->head, 0);
					aNode->currLevel = levelWrite;
					aNode->levels = levelThisNode + 1;
					aNode->member = head0->member;
					aNode->score = head0->score;
				}

				if (aNode->score < _score)
				{
					if (aNode->next != NULL)
					{
						struct xlq_skiplist_node* bNode = aNode->next;
						aNode->next = insertNode;
						insertNode->prev = aNode;
						insertNode->next = bNode;
						bNode->prev = insertNode;
					}
					else
					{
						aNode->next = insertNode;
						insertNode->prev = aNode;
						if (levelWrite == 0)
						{
							_list->tail = insertNode;
						}
					}
				}
				else {
					if (aNode->prev != NULL)
					{
						struct xlq_skiplist_node* bNode = aNode->prev;
						aNode->prev = insertNode;
						insertNode->next = aNode;
						insertNode->prev = bNode;
						bNode->next = insertNode;
					}
					else
					{	//这里麻烦了，撞上1柱了，要交换
						if (levelWrite <= list_ori_level_index) {
							struct xlq_skiplist_node* aNext = aNode->next;
							struct xlq_list_str* aMember = aNode->member;
							long long aScore = aNode->score;
							aNode->score = _score;
							aNode->member = insertNode->member;
							aNode->next = insertNode;

							insertNode->score = aScore;
							insertNode->member = aMember;
							insertNode->prev = aNode;
							if (aNext != NULL)
							{
								insertNode->next = aNext;
								aNext->prev = insertNode;
							}
						} 
						else
						{
							//aNode已经没值了
							aNode->score = _score;
							aNode->member = insertNode->member;
						}

					}
				}
				
				insertNode++;	//上爬一层
			}

			if (pillarBuQi > 0)		//需要补齐1柱,把分和值都刷成新插这个
			{
				int levelWrite = levelThisNode;
				struct xlq_skiplist_node* aNode;
				for (int i = 0; i < pillarBuQi; i++) {
					aNode = find_node_by_level(_list->head, levelWrite);
					aNode->score = _score;
					aNode->member = pillarBuQiMember;
					levelWrite++;
				}
				
			}

			if (upLevel && insert1Zhu)
			{
				insertNode0->levels = high_1;
			}

			_list->length++;
		}
	}

	return 1;
}

int xlq_skiplist_del(struct xlq_skiplist* _list, long long _score, struct xlq_str* _member, int free_xlq_str, struct xlq_str** _outValue) {
	if (_list->head == NULL)
	{
		return 0;
	}
	else {
		int list_ori_level_index = _list->level - 1;
		struct xlq_skiplist_node* levelsRecord[_list->level];
		int level_highest = _list->level - 1;

		struct xlq_skiplist_node* currNode = _list->head;
		currNode = find_node_by_level(currNode, level_highest);
		for (int seLevel = level_highest; seLevel >= 0; seLevel--)
		{
			while (1)
			{

				if (currNode->next == NULL || currNode->next->score > _score) {
					//降层
					levelsRecord[seLevel] = currNode;
					if (seLevel == 0)
					{
						break;
					}
					currNode = currNode - 1;
					break;
				}
				currNode = currNode->next;
			}
		}

		if (currNode->score == _score)
		{
			struct xlq_str* dell = NULL;  //TODO 泄漏
			if (xlq_list_str_del(currNode->member, _member, &dell) == -1) {		//分对，值不对
				return 0;
			}
			if (free_xlq_str)
			{
				xlq_str_free(dell);
			}
			else {
				*_outValue = dell;
			}

			if (currNode->member->head == NULL)	// 没了
			{
				xlq_list_str_distory(currNode->member, free_xlq_str);
				if (_list->length == 1)
				{
					xlq_free(currNode);
					_list->head = NULL;
					_list->tail = NULL;
					_list->length--;
					_list->level = 0;
					return 1;
				}

				struct xlq_skiplist_node* node2_0 = currNode->next;
				struct xlq_skiplist_node* willDelNode = currNode;
				int delChg = 0;
				int rangeLevel = willDelNode->levels < _list->level ? willDelNode->levels : _list->level;
				for (int levelDel = 0; levelDel < rangeLevel; levelDel++) {

					if (currNode->prev != NULL)
					{
						if (currNode->next != NULL)
						{
							currNode->prev->next = currNode->next;
							currNode->next->prev = currNode->prev;
						}
						else
						{
							if (currNode->prev->isFirst && levelDel == _list->level - 1 && levelDel > 0)
							{
								//消减一层
								_list->level--;
							}
							currNode->prev->next = NULL;
							if (levelDel == 0)
							{
								_list->tail = currNode->prev;
							}
						}
					}
					else 
					{	//我就是1柱，又得交换
						delChg = 1;
						struct xlq_skiplist_node* node2 = currNode->next;
						currNode->member = node2_0->member;
						currNode->score = node2_0->score;
						if (levelDel <= node2_0->levels - 1)	//你看得懂吗
						{
							if (node2)
							{
								struct xlq_skiplist_node* node3 = node2->next;  //TODO
								currNode->next = node3;
								if (node3 != NULL)
								{
									node3->prev = currNode;
								}
							}
							else
							{
								//消减一层
								//_list->level--;
							}
						}
						if (levelDel == 0)
						{
							_list->tail = currNode;
						}
					}
					currNode++;
				}
				if (delChg)
				{
					xlq_free(node2_0);
				}
				else
				{
					xlq_free(willDelNode);
				}
			}

			_list->length--;
		}
		else {
			return 0;
		}
	}
	return 1;
}

int xlq_skiplist_del_by_score(struct xlq_skiplist* _list, long long _score, int free_xlq_str, struct xlq_list_str* _outValue) {
	if (_list->head == NULL)
	{
		return 0;
	}
	else {
		int list_ori_level_index = _list->level - 1;
		struct xlq_skiplist_node* levelsRecord[_list->level];
		int level_highest = _list->level - 1;

		struct xlq_skiplist_node* currNode = _list->head;
		currNode = find_node_by_level(currNode, level_highest);
		for (int seLevel = level_highest; seLevel >= 0; seLevel--)
		{
			while (1)
			{

				if (currNode->next == NULL || currNode->next->score > _score) {
					//降层
					levelsRecord[seLevel] = currNode;
					if (seLevel == 0)
					{
						break;
					}
					currNode = currNode - 1;
					break;
				}
				currNode = currNode->next;
			}
		}

		if (currNode->score == _score)
		{
			int delNum = currNode->member->size;
			if (delNum > 0 && free_xlq_str == 0)
			{
				xlq_list_str_insert_all(_outValue, currNode->member);
			}

			xlq_list_str_distory(currNode->member, free_xlq_str);
			if (_list->length == delNum)
			{
				xlq_free(currNode);
				_list->head = NULL;
				_list->tail = NULL;
				_list->length--;
				_list->level = 0;
				return delNum;
			}

			struct xlq_skiplist_node* node2_0 = currNode->next;
			struct xlq_skiplist_node* willDelNode = currNode;
			int delChg = 0;
			int rangeLevel = willDelNode->levels < _list->level ? willDelNode->levels : _list->level;
			for (int levelDel = 0; levelDel < rangeLevel; levelDel++) {

				if (currNode->prev != NULL)
				{
					if (currNode->next != NULL)
					{
						currNode->prev->next = currNode->next;
						currNode->next->prev = currNode->prev;
					}
					else
					{
						if (currNode->prev->isFirst && levelDel == _list->level - 1 && levelDel > 0)
						{
							//消减一层
							_list->level--;
						}
						currNode->prev->next = NULL;
						if (levelDel == 0)
						{
							_list->tail = currNode->prev;
						}
					}
				}
				else
				{	//我就是1柱，又得交换
					delChg = 1;
					struct xlq_skiplist_node* node2 = currNode->next;
					currNode->member = node2_0->member;
					currNode->score = node2_0->score;
					if (levelDel <= node2_0->levels - 1)	//你看得懂吗
					{
						if (node2)
						{
							struct xlq_skiplist_node* node3 = node2->next;  //TODO
							currNode->next = node3;
							if (node3 != NULL)
							{
								node3->prev = currNode;
							}
						}
						else
						{
							//消减一层
							//_list->level--;
						}
					}
					if (levelDel == 0)
					{
						_list->tail = currNode;
					}
				}
				currNode++;
			}
			if (delChg)
			{
				xlq_free(node2_0);
			}
			else
			{
				xlq_free(willDelNode);
			}

			_list->length -= delNum;
			return delNum;
		}
		else {
			return 0;
		}
	}
	return 0;
}

struct xlq_skiplist_node* xlq_skiplist_findNodeByIndex(struct xlq_skiplist* _list, int index) {
	if (_list->length == 0) {
		return NULL;
	}

	struct xlq_skiplist_node* currNode = _list->head;
	int currIndex = 0;
	while (1)
	{
		if (currIndex == index)
		{
			return currNode;
		}
		if (currNode->next == NULL)
		{
			break;
		}
		currNode = currNode->next;
		currIndex++;
	}
	return NULL;
}

int xlq_skiplist_remrangeByIndex(struct xlq_skiplist* _list, int start, int end, int free_xlq_str, struct xlq_list_str* _outValue) {
	if (_list->length == 0) {
		return 0;
	}

	struct xlq_skiplist_node* currNode = _list->head;
	if (_list->length == 1 && start == 0) {
		xlq_list_str_distory(currNode->member, 1);
		_list->head = NULL;
		_list->tail = NULL;
		_list->length--;
		_list->level = 0;
		return 1;
	}

	if (start < 0)
	{
		start = 0;
	}
	if (end < 0)
	{
		end = 0;
	}

	if (start == 0 && end == 0)
	{
		if (free_xlq_str)
		{
			xlq_skiplist_del(_list, currNode->score, currNode->member, 1, NULL);
		}
		else
		{
			struct xlq_str* d;
			if (xlq_skiplist_del(_list, currNode->score, currNode->member, 0, &d)) {
				xlq_list_str_insert(_outValue, d);
			}
		}
		
		return 1;
	}
	int startIndex = start == 0 ? 1 : start;
	int delNum = 0;
	currNode = xlq_skiplist_findNodeByIndex(_list, startIndex);
	if (currNode != NULL)
	{
		struct xlq_skiplist_node* willDelNode = NULL;
		for (int i = startIndex; i <= end; i++)
		{
			if (currNode == NULL)
			{
				break;
			}
			willDelNode = currNode;
			currNode = currNode->next;
			//xlq_skiplist_del(_list, willDelNode->score, willDelNode->member, 1, NULL);

			struct xlq_skiplist_node* willDelNodeHigh = find_node_by_level(willDelNode, willDelNode->levels - 1);
			if (willDelNodeHigh->prev && willDelNodeHigh->prev->isFirst && willDelNodeHigh->next == NULL)
			{
				//涉及降层，写不明白了
				if (free_xlq_str)
				{
					delNum += xlq_skiplist_del_by_score(_list, willDelNode->score, 1, NULL);
				}
				else {
					delNum += xlq_skiplist_del_by_score(_list, willDelNode->score, 0, _outValue);
				}
			}
			else
			{

				delNum += willDelNode->member->size;
				_list->length -= willDelNode->member->size;
				struct xlq_skiplist_node* willDelNode0 = willDelNode;
				for (int i = 0; i < willDelNode0->levels; i++)
				{
					if (i == 0)
					{
						if (free_xlq_str)
						{
							xlq_list_str_distory(willDelNode->member, 1);
						}
						else {
							xlq_list_str_insert_all(_outValue, willDelNode->member);
							xlq_list_str_distory(willDelNode->member, 0);
						}
					}

					if (willDelNode->next != NULL)
					{
						if (willDelNode->prev)	//没想明白这里为啥是空的
						{
							willDelNode->prev->next = willDelNode->next;
						}
						willDelNode->next->prev = willDelNode->prev;
					}
					else
					{
						if (willDelNode->prev)	//没想明白这里为啥是空的
						{
							willDelNode->prev->next = NULL;
						}
					}
					willDelNode++;
				}
				xlq_free(willDelNode0);
			}
		}
	}
	
	if (start == 0)
	{
		if (free_xlq_str)
		{
			delNum += xlq_skiplist_del_by_score(_list, _list->head->score, 1, NULL);
		}
		else {
			delNum += xlq_skiplist_del_by_score(_list, _list->head->score, 0, _outValue);
		}
	}
	return delNum;
}


int xlq_skiplist_remrangeByScore(struct xlq_skiplist* _list, long long minScore, long long maxScore, int free_xlq_str, struct xlq_list_str* _outValue) {
	if (_list->length == 0) {
		return 0;
	}

	int level_highest = _list->level - 1;

	struct xlq_skiplist_node* currNode = _list->head;
	currNode = find_node_by_level(currNode, level_highest);
	for (int seLevel = level_highest; seLevel >= 0; seLevel--)
	{
		while (1)
		{
			if (currNode->next == NULL || currNode->next->score >= minScore) {
				//降层
				if (seLevel == 0)
				{
					break;
				}
				currNode = currNode - 1;
				break;
			}
			currNode = currNode->next;
		}
	}

	int hasFirst = 0;
	if (currNode->prev == NULL && currNode->score >= minScore && currNode->score <= maxScore)
	{
		hasFirst = 1;
		currNode = currNode->next;
	}

	int delNum = 0;
	int delLengthNum = 0;
	
	while (1)
	{
		if (currNode == NULL || currNode->score > maxScore)
		{
			break;
		}
		if (currNode->score < minScore)
		{
			currNode = currNode->next;
			continue;
		}
		struct xlq_skiplist_node* willDelNode = NULL;
		willDelNode = currNode;
		currNode = currNode->next;

		struct xlq_skiplist_node* willDelNodeHigh = find_node_by_level(willDelNode, willDelNode->levels - 1);
		if (willDelNodeHigh->prev && willDelNodeHigh->prev->isFirst && willDelNodeHigh->next == NULL)  //TODO 没想明白willDelNodeHigh->prev为啥是空的
		{
			//涉及降层，写不明白了
			if (free_xlq_str)
			{
				delNum += xlq_skiplist_del_by_score(_list, willDelNode->score, 1, NULL);
			}
			else {
				delNum += xlq_skiplist_del_by_score(_list, willDelNode->score, 0, _outValue);
			}
		}
		else
		{
			delNum += willDelNode->member->size;
			_list->length -= willDelNode->member->size;
			struct xlq_skiplist_node* willDelNode0 = willDelNode;
			for (int i = 0; i < willDelNode0->levels; i++)
			{
				if (i == 0)
				{
					if (free_xlq_str)
					{
						xlq_list_str_distory(willDelNode->member, 1);
					}
					else {
						xlq_list_str_insert_all(_outValue, willDelNode->member);
						xlq_list_str_distory(willDelNode->member, 0);
					}
				}
				if (willDelNode->next != NULL)
				{
					if (willDelNode->prev)	//没想明白这里为啥是空的
					{
						willDelNode->prev->next = willDelNode->next;
					}
					willDelNode->next->prev = willDelNode->prev;
				}
				else
				{
					if (willDelNode->prev)	//没想明白这里为啥是空的
					{
						willDelNode->prev->next = NULL;
					}
				}
				willDelNode++;
			}
			xlq_free(willDelNode0);
		}
	}

	if (hasFirst == 1)
	{
		if (free_xlq_str)
		{
			delNum += xlq_skiplist_del_by_score(_list, _list->head->score, 1, NULL);
		}
		else {
			delNum += xlq_skiplist_del_by_score(_list, _list->head->score, 0, _outValue);
		}
	}
	
	return delNum;
}

void xlq_skiplist_range_print(struct xlq_skiplist* _list, long long scoreBegin, long long scoreEnd, int printLevelMin, int printEveryLevel) {
	if (_list->length == 0) {
		return;
	}

	int level_highest = _list->level - 1;

	struct xlq_skiplist_node* currNode = _list->head;
	currNode = find_node_by_level(currNode, level_highest);
	for (int seLevel = level_highest; seLevel >= 0; seLevel--)
	{
		while (1)
		{
			if (currNode->next == NULL || currNode->next->score >= scoreBegin) {
				//降层
				if (seLevel == 0)
				{
					break;
				}
				currNode = currNode - 1;
				break;
			}
			currNode = currNode->next;
		}
	}

	printf("skiplistlevel=%d, skiplistlen=%d\n", _list->level, _list->length);
	int index = -1;
	while (1)
	{
		index++;
		if (currNode->score > scoreEnd)
		{
			break;
		}
		struct xlq_skiplist_node* seeEveryLevel = currNode;

		int rangeMax = currNode->levels;
		if (rangeMax < printLevelMin)
		{
			if (currNode->next == NULL)
			{
				break;
			}
			currNode = currNode->next;
			continue;
		}

		printf("-------------------------------------------\n");
		printf("score=%d, levels=%d, index=%d\n", currNode->score, currNode->levels, index);

		
		rangeMax = seeEveryLevel->levels;
		if (rangeMax > _list->level)
		{
			rangeMax = _list->level;
		}
		for (size_t i = 0; i < rangeMax; i++)
		{
			struct xlq_list_node* cc = seeEveryLevel->member->head;
			while (1)
			{
				//printf("currLevel=%d, v=%s, member=%X, this=%X, left=%X, right=%X, \t", seeEveryLevel->currLevel, cc->m_list_value->m_value, seeEveryLevel->member, seeEveryLevel, seeEveryLevel->prev, seeEveryLevel->next);
				printf("currLevel=%d, v=%s,  \n", seeEveryLevel->currLevel, cc->m_list_value->m_value);
				if (cc->next == NULL)
				{
					break;
				}
				cc = cc->next;
			}
			printf("\n");
			if (printEveryLevel == 0)
			{
				break;
			}
			seeEveryLevel ++;
		}
		
		if (currNode->next == NULL)
		{
			break;
		}
		currNode = currNode->next;
	}
	printf("\n\n\n");
}

void xlq_skiplist_range_call(struct xlq_skiplist* _list, long long minScore, long long maxScore
		, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count) {
	if (_list->length == 0) {
		return;
	}

	int level_highest = _list->level - 1;

	struct xlq_skiplist_node* currNode = _list->head;
	currNode = find_node_by_level(currNode, level_highest);
	for (int seLevel = level_highest; seLevel >= 0; seLevel--)
	{
		while (1)
		{
			if (currNode->next == NULL || currNode->next->score >= minScore) {
				//降层
				if (seLevel == 0)
				{
					break;
				}
				currNode = currNode - 1;
				break;
			}
			currNode = currNode->next;
		}
	}

	int index = -1;
	int useNum = 0;
	while (1)
	{
		index++;
		if (currNode->score > maxScore)
		{
			break;
		}

		struct xlq_list_node* cc = currNode->member->head;
		while (1)
		{
			if (index >= offset)
			{
				callback_content(arg, cc->m_list_value);
				useNum++;
				if (useNum >= count)
				{
					goto outloop;
				}
			}
			
			if (cc->next == NULL)
			{
				break;
			}
			cc = cc->next;
		}

		if (currNode->next == NULL)
		{
			break;
		}
		currNode = currNode->next;
	}
outloop:return 0;
}

void xlq_skiplist_revrange_call(struct xlq_skiplist* _list, long long maxScore, long long minScore
	, void* arg, void (*callback_content)(void*, struct xlq_str*), int offset, int count) {
	if (_list->length == 0) {
		return;
	}

	int level_highest = _list->level - 1;

	struct xlq_skiplist_node* currNode = _list->head;
	currNode = find_node_by_level(currNode, level_highest);
	for (int seLevel = level_highest; seLevel >= 0; seLevel--)
	{
		while (1)
		{
			if (currNode->next == NULL || currNode->next->score >= maxScore) {
				//降层
				if (seLevel == 0)
				{
					break;
				}
				currNode = currNode - 1;
				break;
			}
			currNode = currNode->next;
		}
	}

	int index = -1;
	int useNum = 0;
	while (1)
	{
		index++;
		if (currNode->score < minScore)
		{
			break;
		}
		
		struct xlq_list_node* cc = currNode->member->head;
		while (1)
		{
			if (index >= offset)
			{
				callback_content(arg, cc->m_list_value);
				useNum++;
				if (useNum >= count)
				{
					goto outloop;
				}
			}
			
			if (cc->next == NULL)
			{
				break;
			}
			cc = cc->next;
		}
		
		if (currNode->prev == NULL)
		{
			break;
		}
		currNode = currNode->prev;
	}
outloop:return 0;
}