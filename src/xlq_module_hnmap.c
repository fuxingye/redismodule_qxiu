#include <stdio.h>
#include "redismodule.h"
#include "xlq_hashmap.h"
#include "xlq_common.h"

/****************************************************
*.module
*.作者：付星烨 方超
*.时间：2021.5.11
*.描述：module hnmap
*****************************************************/

#define GET_OR_CREATE_NMAP(type, hto, key)                               \
    do {                                                                 \
        if (type == REDISMODULE_KEYTYPE_EMPTY) {                         \
            xlq_hashmap_create(16, &hto);                                \
            RedisModule_ModuleTypeSetValue(key, NMapType, hto);          \
        }                                                                \
        else {                                                           \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)       

#define GET_NMAP(type, hto, key)                               \
    do {                                                                 \
        if (type != REDISMODULE_KEYTYPE_EMPTY) {                         \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)           

static RedisModuleType* NMapType;

void NMap_setNMapType(RedisModuleType * _NMapType) {
    NMapType = _NMapType;
}

int NMap_hnsetCmd(RedisModuleCtx * ctx, RedisModuleString * * argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 5) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t fieldLen;
    char* field = RedisModule_StringPtrLen(argv[2], &fieldLen);

    long long score;
    if ((RedisModule_StringToLongLong(argv[3], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    size_t valueLen;
    char* value = RedisModule_StringPtrLen(argv[4], &valueLen);

    struct xlq_hashmap_t* map;
    GET_OR_CREATE_NMAP(type, map, key);

    struct xlq_str* v;
    int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
    if (findRs)
    {
        if (v->m_count2 != valueLen || memcmp(v->m_value2, value, valueLen) != 0)
        {
            char* v1 = xlq_malloc(valueLen);
            memcpy(v1, value, valueLen);
            xlq_free(v->m_value2);
            v->m_value2 = v1;
            v->m_count2 = valueLen;
        }
        v->m_value_8zj = score;
        RedisModule_ReplyWithLongLong(ctx, 0);
    }
    else
    {
        struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
        char* v0 = xlq_malloc(fieldLen);
        memcpy(v0, field, fieldLen);
        char* v1 = xlq_malloc(valueLen);
        memcpy(v1, value, valueLen);

        str0->m_count = fieldLen;
        str0->m_value = v0;
        str0->m_value_8zj = score;
        str0->m_count2 = valueLen;
        str0->m_value2 = v1;

        xlq_hashmap_put(map, str0);
        RedisModule_ReplyWithLongLong(ctx, 1);
    }
    
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int NMap_hnsetReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 5) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t fieldLen;
    char* field = RedisModule_StringPtrLen(argv[2], &fieldLen);

    long long score;
    if ((RedisModule_StringToLongLong(argv[3], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    size_t valueLen;
    char* value = RedisModule_StringPtrLen(argv[4], &valueLen);

    struct xlq_hashmap_t* map;
    GET_OR_CREATE_NMAP(type, map, key);

    struct xlq_str* v;
    int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
    if (findRs)
    {
        RedisModule_ReplyWithArray(ctx, (long) 2);
        RedisModule_ReplyWithLongLong(ctx, (long) v->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, v->m_value2, v->m_count2);
        if (v->m_count2 != valueLen || memcmp(v->m_value2, value, valueLen) != 0)
        {
            char* v1 = xlq_malloc(valueLen);
            memcpy(v1, value, valueLen);

            xlq_free(v->m_value2);
            v->m_value2 = v1;
            v->m_count2 = valueLen;
        }
        v->m_value_8zj = score;
    }
    else
    {
        struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
        char* v0 = xlq_malloc(fieldLen);
        memcpy(v0, field, fieldLen);
        char* v1 = xlq_malloc(valueLen);
        memcpy(v1, value, valueLen);

        str0->m_count = fieldLen;
        str0->m_value = v0;
        str0->m_value_8zj = score;
        str0->m_count2 = valueLen;
        str0->m_value2 = v1;

        xlq_hashmap_put(map, str0);
        RedisModule_ReplyWithArray(ctx, (long) 0);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int NMap_hnincrbyCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t fieldLen;
    char* field = RedisModule_StringPtrLen(argv[2], &fieldLen);

    long long score;
    if ((RedisModule_StringToLongLong(argv[3], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    size_t valueLen;
    char* value = NULL;
    if (argc == 5)
    {
        value = RedisModule_StringPtrLen(argv[4], &valueLen);
    }
    else
    {
        valueLen = 0;
    }

    struct xlq_hashmap_t* map;
    GET_OR_CREATE_NMAP(type, map, key);

    struct xlq_str* v;
    int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
    if (findRs)
    {
        if (valueLen > 0 && (v->m_count2 != valueLen || memcmp(v->m_value2, value, valueLen) != 0))
        {
            char* v1 = xlq_malloc(valueLen);
            memcpy(v1, value, valueLen);
            xlq_free(v->m_value2);
            v->m_value2 = v1;
            v->m_count2 = valueLen;
        }
        v->m_value_8zj += score;
        RedisModule_ReplyWithLongLong(ctx, v->m_value_8zj);
    }
    else
    {
        struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
        char* v0 = xlq_malloc(fieldLen);
        memcpy(v0, field, fieldLen);
        char* v1 = xlq_malloc(valueLen);
        memcpy(v1, value, valueLen);

        str0->m_count = fieldLen;
        str0->m_value = v0;
        str0->m_value_8zj = score;
        str0->m_count2 = valueLen;
        str0->m_value2 = v1;

        xlq_hashmap_put(map, str0);
        RedisModule_ReplyWithLongLong(ctx, score);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int NMap_hnincrbyReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    size_t fieldLen;
    char* field = RedisModule_StringPtrLen(argv[2], &fieldLen);

    long long score;
    if ((RedisModule_StringToLongLong(argv[3], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }

    size_t valueLen;
    char* value = NULL;
    if (argc == 5)
    {
        value = RedisModule_StringPtrLen(argv[4], &valueLen);
    }

    struct xlq_hashmap_t* map;
    GET_OR_CREATE_NMAP(type, map, key);

    struct xlq_str* v;
    int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
    if (findRs)
    {
        RedisModule_ReplyWithArray(ctx, (long) 2);
        RedisModule_ReplyWithLongLong(ctx, (long) v->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, v->m_value2, v->m_count2);
        if (valueLen > 0 && (v->m_count2 != valueLen || memcmp(v->m_value2, value, valueLen) != 0))
        {
            char* v1 = xlq_malloc(valueLen);
            memcpy(v1, value, valueLen);
            xlq_free(v->m_value2);
            v->m_value2 = v1;
            v->m_count2 = valueLen;
        }
        v->m_value_8zj += score;
        RedisModule_ReplyWithLongLong(ctx, v->m_value_8zj);
    }
    else
    {
        struct xlq_str* str0 = xlq_malloc(sizeof(struct xlq_str));
        char* v0 = xlq_malloc(fieldLen);
        memcpy(v0, field, fieldLen);
        char* v1 = xlq_malloc(valueLen);
        memcpy(v1, value, valueLen);

        str0->m_count = fieldLen;
        str0->m_value = v0;
        str0->m_value_8zj = score;
        str0->m_count2 = valueLen;
        str0->m_value2 = v1;

        xlq_hashmap_put(map, str0);
        RedisModule_ReplyWithArray(ctx, (long) 0);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int NMap_hngetCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);

    for (int i = 0; i < argc; i++)
    {
        size_t fieldLen;
        char* field = RedisModule_StringPtrLen(argv[i], &fieldLen);

        struct xlq_str* v;
        int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
        if (findRs)
        {
            xlq_list_str_insert(list, v);
        }
    }
    
    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}

int NMap_hngetallCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    int r = xlq_hashmap_get_all(map, list);
    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}

int NMap_hndelCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }

    int delNum = 0;
    for (int i = 0; i < argc; i++)
    {
        size_t fieldLen;
        char* field = RedisModule_StringPtrLen(argv[i], &fieldLen);

        struct xlq_str* v;
        int r = xlq_hashmap_del2(map, fieldLen, field, &v);
        if (r > 0)
        {
            delNum++;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, delNum);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}


int NMap_hndelReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);

    for (int i = 0; i < argc; i++)
    {
        size_t fieldLen;
        char* field = RedisModule_StringPtrLen(argv[i], &fieldLen);

        struct xlq_str* v;
        int findRs = xlq_hashmap_del2(map, fieldLen, field, &v);
        if (findRs)
        {
            xlq_list_str_insert(list, v);
        }
    }

    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int NMap_hnlenCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }

    RedisModule_ReplyWithLongLong(ctx, map->m_size);
    return REDISMODULE_OK;
}

int NMap_hnkeysCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    int r = xlq_hashmap_get_all(map, list);
    RedisModule_ReplyWithArray(ctx, (long) list->size);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);
    return REDISMODULE_OK;
}

int NMap_hnexistsCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx);

    if (argc != 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != NMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_hashmap_t* map = NULL;
    GET_NMAP(type, map, key);
    if (map == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, 0);
        return REDISMODULE_OK;
    }

    size_t fieldLen;
    char* field = RedisModule_StringPtrLen(argv[2], &fieldLen);

    struct xlq_str* v;
    int findRs = xlq_hashmap_get2(map, fieldLen, field, &v);
    if (findRs)
    {
        RedisModule_ReplyWithLongLong(ctx, 1);
    }

    return REDISMODULE_OK;
}

/* ========================== "nmaptype" type methods ======================= */

void* NMapTypeRdbLoad(RedisModuleIO* rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }
    int64_t size = RedisModule_LoadSigned(rdb);	//num of keys
    struct xlq_hashmap_t* hto;
    xlq_hashmap_create((unsigned int) size, &hto);

    long long scoreOri = 0;
    unsigned int outValue_size;
    char* outValue_value;
    while (size--) {
        long long score = RedisModule_LoadSigned(rdb);

        size_t memberLen = 0;
        char* member = RedisModule_LoadStringBuffer(rdb, &memberLen);
        size_t valueLen = 0;
        char* value = RedisModule_LoadStringBuffer(rdb, &valueLen);

        struct xlq_str* str = xlq_malloc(sizeof(struct xlq_str));
        str->m_value_8zj = score;
        str->m_count = (unsigned int) memberLen;
        str->m_value = member;
        str->m_count2 = (unsigned int) valueLen;
        str->m_value2 = value;

        xlq_hashmap_put(hto, str);
    }
    return hto;
}

void NMapTypeRdbSave(RedisModuleIO* rdb, void* value) {
    struct xlq_hashmap_t* m = value;
    RedisModule_SaveSigned(rdb, m->m_size);	//num of score
    struct xlq_list_str* list;
    for (size_t i = 0; i < m->array_size; i++)
    {
        list = m->arrays + i;
        if (list->size > 0)
        {
            struct xlq_list_node* n = list->head;
            while (n)
            {
                struct xlq_str* str = n->m_list_value;
                RedisModule_SaveSigned(rdb, str->m_value_8zj);
                RedisModule_SaveStringBuffer(rdb, str->m_value, (size_t) str->m_count);
                RedisModule_SaveStringBuffer(rdb, str->m_value2, (size_t) str->m_count2);
                n = n->next;
            }
        }
    }
}

void NMapTypeAofRewrite(RedisModuleIO* rdb, RedisModuleString* key, void* value) {
    struct xlq_hashmap_t* m = value;
    struct xlq_list_str* list;
    for (size_t i = 0; i < m->array_size; i++)
    {
        list = m->arrays + i;
        if (list->size > 0)
        {
            struct xlq_list_node* n = list->head;
            while (n)
            {
                struct xlq_str* str = n->m_list_value;
                RedisModule_EmitAOF(rdb, "xlq.hnset", "sblb", key, str->m_value, (size_t) str->m_count, str->m_value_8zj, str->m_value2, (size_t) str->m_count2);
                n = n->next;
            }
        }
    }
}

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t NMapTypeMemUsage(void* value) {
    return xlq_hashmap_sizeof(value);
}

void NMapTypeFree(void* value) {
    xlq_hashmap_destory(value);
}

void NMapTypeDigest(RedisModuleDigest* md, void* value) {

}