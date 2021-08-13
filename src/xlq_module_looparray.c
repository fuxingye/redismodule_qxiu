#include <stdio.h>
#include "redismodule.h"
#include "xlq_common.h"
#include "xlq_list.h"
#include "xlq_loop_array.h"

/****************************************************
*.module
*.作者：付星烨 方超
*.时间：2021.5.11
*.描述：module looparray
*****************************************************/

#define GET_OR_CREATE_LOOPARRAY(type, hto, key, size)                    \
    do {                                                                 \
        if (type == REDISMODULE_KEYTYPE_EMPTY) {                         \
            hto = xlq_looparray_str_create(size);                              \
            RedisModule_ModuleTypeSetValue(key, LooparrayType, hto);     \
        }                                                                \
        else {                                                           \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)       

#define GET_LOOPARRAY(type, hto, key)                               \
    do {                                                                 \
        if (type != REDISMODULE_KEYTYPE_EMPTY) {                         \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)           

static RedisModuleType* LooparrayType;

void LoopArr_setLooparrayType(void* _looparrayType) {
    LooparrayType = _looparrayType;
}

int LoopArr_loopstrCreateCmd(RedisModuleCtx * ctx, RedisModuleString * * argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != LooparrayType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    long long size;
    if ((RedisModule_StringToLongLong(argv[2], &size) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    if (size <= 0) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: size must larger than 0");
    }

    struct xlq_looparray_t* loop;
    GET_OR_CREATE_LOOPARRAY(type, loop, key, size);

    RedisModule_ReplyWithLongLong(ctx, (long) 1);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int LoopArr_loopstrCreateOrResizeCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != LooparrayType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    long long size;
    if ((RedisModule_StringToLongLong(argv[2], &size) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    if (size <= 0) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: size must larger than 0");
    }

    if (type == REDISMODULE_KEYTYPE_EMPTY) {
        struct xlq_looparray_t* loop = xlq_looparray_str_create(size);
        RedisModule_ModuleTypeSetValue(key, LooparrayType, loop);  
        RedisModule_ReplyWithLongLong(ctx, (long) 1);
    }                                                                
    else {
        struct xlq_looparray_t* loop = RedisModule_ModuleTypeGetValue(key);
        if (loop->array_size != size)
        {
            loop = xlq_looparray_str_resize(loop, size);
            RedisModule_ModuleTypeSetValue(key, LooparrayType, loop);
            RedisModule_ReplyWithLongLong(ctx, (long) 1);
        }
        else
        {
            RedisModule_ReplyWithLongLong(ctx, (long) 0);
        }
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int LoopArr_loopstrInsertCmd(RedisModuleCtx * ctx, RedisModuleString * * argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != LooparrayType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_looparray_t* loop = NULL;
    GET_LOOPARRAY(type, loop, key);
    if (loop == NULL)
    {
        return RedisModule_ReplyWithError(ctx, "ERR looparray not create");
    }

    size_t valueLen;
    char* value = RedisModule_StringPtrLen(argv[2], &valueLen);
    xlq_looparray_str_insert(loop, valueLen, value);
    RedisModule_ReplyWithLongLong(ctx, (long) 1);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int LoopArr_loopstrInfoCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != LooparrayType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_looparray_t* loop = NULL;
    GET_LOOPARRAY(type, loop, key);
    if (loop == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        return REDISMODULE_OK;
    }

    int size = xlq_looparray_str_member_size(loop);
    RedisModule_ReplyWithArray(ctx, (long) 4);
    RedisModule_ReplyWithLongLong(ctx, (long) loop->array_size);
    RedisModule_ReplyWithLongLong(ctx, (long) size);
    RedisModule_ReplyWithLongLong(ctx, (long) loop->index_0);
    RedisModule_ReplyWithLongLong(ctx, (long) loop->index_next);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int LoopArr_loopstrRevrangeCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 4) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != LooparrayType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_looparray_t* loop = NULL;
    GET_LOOPARRAY(type, loop, key);
    if (loop == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        return REDISMODULE_OK;
    }

    long long start;
    if ((RedisModule_StringToLongLong(argv[2], &start) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid start: must be a long long");
    }

    long long count;
    if ((RedisModule_StringToLongLong(argv[3], &count) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid count: must be a long long");
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    xlq_looparray_str_range(loop, start, count, list);
    
    RedisModule_ReplyWithArray(ctx, (long) list->size);
    struct xlq_list_node* node = list->end;
    int index = 0;
    while (node)
    {
        struct xlq_str_simple* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        node = node->prev;
        index++;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}

/* ========================== "LoopArrtype" type methods ======================= */

void* LoopArrTypeRdbLoad(RedisModuleIO* rdb, int encver) {
    if (encver != 0) {
        return NULL;
    }
    int64_t size = RedisModule_LoadSigned(rdb);	//num of keys
    struct xlq_looparray_t* hto = xlq_looparray_str_create((unsigned short) size);

    while (size--) {
        size_t memberLen = 0;
        char* member = RedisModule_LoadStringBuffer(rdb, &memberLen);
        xlq_looparray_str_insert(hto, memberLen, member);
    }
    return hto;
}

void LoopArrTypeRdbSave(RedisModuleIO* rdb, void* value) {
    struct xlq_looparray_t* m = value;
    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    xlq_looparray_str_range(m, 0, 1000000000, list);
    RedisModule_SaveSigned(rdb, list->size);	//num of score
    
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str_simple* s = node->m_list_value;
        RedisModule_SaveStringBuffer(rdb, s->m_value, s->m_count);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);
}

void LoopArrTypeAofRewrite(RedisModuleIO* rdb, RedisModuleString* key, void* value) {
    struct xlq_looparray_t* m = value;
    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    xlq_looparray_str_range(m, 0, 1000000000, list);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str_simple* s = node->m_list_value;
        RedisModule_EmitAOF(rdb, "xlq.loopstrInsert", "sb", key, s->m_value, (size_t) s->m_count);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

}

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t LoopArrTypeMemUsage(void* value) {
    return xlq_looparray_str_sizeof(value);
}

void LoopArrTypeFree(void* value) {
    xlq_looparray_str_destory(value, 1);
}
