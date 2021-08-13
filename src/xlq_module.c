#include <stdio.h>
#include "xlq_zmapCmd.h"
#include "redismodule.h"
#include "xlq_zmap.h"
#include "xlq_common.h"
#include "xlq_module_hnmap.h"
#include "xlq_module_looparray.h"

/****************************************************
*.module
*.作者：付星烨 方超
*.时间：2021.5.6
*.描述：module
*****************************************************/
#define CREATE_CMD(name, tgt, attr)                                                                \
    do {                                                                                           \
        if (RedisModule_CreateCommand(ctx, name, tgt, attr, 1, 1, 1) != REDISMODULE_OK) {          \
            return REDISMODULE_ERR;                                                                \
        }                                                                                          \
    } while (0)
#define CREATE_WRCMD(name, tgt) CREATE_CMD(name, tgt, "write deny-oom")
#define CREATE_ROCMD(name, tgt) CREATE_CMD(name, tgt, "readonly fast")

#define GET_OR_CREATE_ZMAP(type, hto, key)                               \
    do {                                                                 \
        if (type == REDISMODULE_KEYTYPE_EMPTY) {                         \
            hto = createZMapTypeObject();                                \
            RedisModule_ModuleTypeSetValue(key, ZMapType, hto);          \
        }                                                                \
        else {                                                           \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)       

#define GET_ZMAP(type, hto, key)                               \
    do {                                                                 \
        if (type != REDISMODULE_KEYTYPE_EMPTY) {                         \
            hto = RedisModule_ModuleTypeGetValue(key);                   \
        }                                                                \
    } while (0)           

void callback_zmap(RedisModuleCtx * ctx, int withScore, int withValue, long long _score, const char* _member, size_t _memberLen, const char* _value, size_t _valueLen) {
    printf("score:%.05f\n", _score);
    printf("member:%s\n", _member);
    printf("value:%s\n", _value);
    RedisModule_ReplyWithStringBuffer(ctx, _member, _memberLen);
    if (withScore)
    {
        RedisModule_ReplyWithLongLong(ctx, _score);
    }
    if (withValue)
    {
        RedisModule_ReplyWithStringBuffer(ctx, _value, _valueLen);
    }
}
void callback_count(RedisModuleCtx* ctx, int withScore, int withValue, int count) {
    int bei = 1 + withScore + withValue;
    RedisModule_ReplyWithArray(ctx, (long) count * bei);
}
void callback_zmap_savelist(struct xlq_list_str* _list, struct xlq_str* _str) {
    xlq_list_str_insert(_list, _str);
}


static RedisModuleType* ZMapType;

/* ========================== "zmaptype" type methods ======================= */

void* ZMapTypeRdbLoad(RedisModuleIO* rdb, int encver) {
    if (encver != 0) {
        /* RedisModule_Log("warning","Can't load data with version %d", encver);*/
        return NULL;
    }
    return zmapLoadRdb(rdb);
}

void ZMapTypeRdbSave(RedisModuleIO* rdb, void* value) {
    zmapSaveRdb(value, rdb);
}
struct aofBox {
    RedisModuleIO* rdb;
    RedisModuleString* key;
};
void callbackAof(struct aofBox* box, struct xlq_str* _str) {
    RedisModule_EmitAOF(box->rdb, "xlq.zmadd", "slbb", box->key, _str->m_value_8zj, _str->m_value, (size_t) _str->m_count, _str->m_value2, (size_t) _str->m_count2);
}
void ZMapTypeAofRewrite(RedisModuleIO* rdb, RedisModuleString* key, void* value) {
    struct xlq_zmap_t* zmap = value;
    struct aofBox box;
    box.key = key;
    box.rdb = rdb;
    xlq_skiplist_range_call(zmap->m_skiplist, -9223372036854775808, 9223372036854775807, &rdb, callbackAof, 0, 2147483647);
}

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t ZMapTypeMemUsage(void* value) {
    return zmapUseMem(value);
}

void ZMapTypeFree(void* value) {
    zmapFree(value);
}

void ZMapTypeDigest(RedisModuleDigest* md, void* value) {

}

/* ========================= "zmap" type commands ======================= */

/* HELLOTYPE.INSERT key value */
int ZMap_zmaddCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 5) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    long long score;
    if ((RedisModule_StringToLongLong(argv[2], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: score must be a long long");
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[3], &memberLen);
    if (memberLen == 0)
    {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: member cannot empty!!");
    }
    size_t valueLen;
    char* value = RedisModule_StringPtrLen(argv[4], &valueLen);
    if (valueLen == 0)
    {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: value cannot empty!!");
    }

    struct xlq_zmap_t* hto;
    GET_OR_CREATE_ZMAP(type, hto, key);
    /*if (type == REDISMODULE_KEYTYPE_EMPTY) {
            hto = createZMapTypeObject();                                
            RedisModule_ModuleTypeSetValue(key, ZMapType, hto);          
    }                                                                
    else {
            hto = RedisModule_ModuleTypeGetValue(key);                   
    }*/

    long long scoreOri = 0;
    unsigned int srcValue_size = 0;
    char* srcValue_value = NULL;
    zmaddCmd(hto, score, member, memberLen, value, valueLen, &scoreOri, &srcValue_size, &srcValue_value);
    if (valueLen > 0 && srcValue_size > 0)  //说明有替换发生
    {
        xlq_free(srcValue_value);
    }
    RedisModule_ReplyWithLongLong(ctx, 1);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmaddReturnOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc != 5) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    long long score;
    if ((RedisModule_StringToLongLong(argv[2], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[3], &memberLen);
    if (memberLen == 0)
    {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: member cannot empty!!");
    }
    size_t valueLen;
    char* value = RedisModule_StringPtrLen(argv[4], &valueLen);
    if (valueLen == 0)
    {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: value cannot empty!!");
    }

    struct xlq_zmap_t* hto;
    GET_OR_CREATE_ZMAP(type, hto, key);

    long long scoreOri = 0;
    unsigned int srcValue_size = 0;
    char* srcValue_value = NULL;
    zmaddCmd(hto, score, member, memberLen, value, valueLen, &scoreOri, &srcValue_size, &srcValue_value);

    RedisModule_ReplyWithArray(ctx, (long) 2);
    RedisModule_ReplyWithLongLong(ctx, scoreOri);
    RedisModule_ReplyWithStringBuffer(ctx, srcValue_value, srcValue_size);

    if (valueLen > 0 && srcValue_size > 0)  //说明有替换发生
    {
        xlq_free(srcValue_value);
    }
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmincrByCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);  
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    double score;
    if ((RedisModule_StringToDouble(argv[2], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[3], &memberLen);

    size_t valueLen = 0;
    char* value = { 0 };
    if (argc == 5)
    {
        value = RedisModule_StringPtrLen(argv[4], &valueLen);
    }

    void* hto;
    GET_OR_CREATE_ZMAP(type, hto, key);

    unsigned int srcValue_size = 0;
    char* srcValue_value = NULL;

    long long scoreNow = zmincrbyCmd(hto, score, member, memberLen, value, valueLen, &srcValue_size, &srcValue_value);

    RedisModule_ReplyWithLongLong(ctx, scoreNow);

    if (valueLen > 0 && srcValue_size > 0)  //说明有替换发生
    {
        xlq_free(srcValue_value);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmincrByReturnOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    double score;
    if ((RedisModule_StringToDouble(argv[2], &score) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a long long");
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[3], &memberLen);

    size_t valueLen = 0;
    char* value = {0};
    if (argc == 5)
    {
        value = RedisModule_StringPtrLen(argv[4], &valueLen);
    }

    void* hto;
    GET_OR_CREATE_ZMAP(type, hto, key);

    unsigned int srcValue_size;
    char* srcValue_value;

    long long scoreNow = zmincrbyCmd(hto, score, member, memberLen, value, valueLen, &srcValue_size, &srcValue_value);

    RedisModule_ReplyWithArray(ctx, (long) 2);
    RedisModule_ReplyWithLongLong(ctx, scoreNow);
    RedisModule_ReplyWithStringBuffer(ctx, srcValue_value, srcValue_size);

    if (valueLen > 0 && srcValue_size > 0)
    {
        xlq_free(srcValue_value);
    }

    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

char* withScoresTmp = "withscores";
char* withValueTmp = "withvalues";
char* limitTmp = "limit";

int ZMap_zmrangeByScoreCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx); 
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long minScore;
    if ((RedisModule_StringToLongLong(argv[2], &minScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: param2 is minScore, must be a long long");
    }

    long long maxScore;
    if ((RedisModule_StringToLongLong(argv[3], &maxScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: param3 is maxScore, must be a long long");
    }

    int withScore = 0;
    int withValue = 0;

    if (argc > 4)
    {
        size_t withXXX1Len;
        char* withXXX1 = RedisModule_StringPtrLen(argv[4], &withXXX1Len);
        if (strcmp(withScoresTmp, withXXX1) == 0) {
            withScore = 1;
        }
        if (strcmp(withValueTmp, withXXX1) == 0) {
            withValue = 1;
        }
        if (withScore == 0 && withValue == 0)
        {
            return RedisModule_ReplyWithError(ctx, "ERR invalid value: param4 must be withscores or withvalues");
        }
    }
    if (argc > 5)
    {
        size_t withXXX2Len;
        char* withXXX2 = RedisModule_StringPtrLen(argv[5], &withXXX2Len);
        if (strcmp(withScoresTmp, withXXX2) == 0) {
            withScore = 1;
        }
        if (strcmp(withValueTmp, withXXX2) == 0) {
            withValue = 1;
        }
        if (withScore == 0 && withValue == 0)
        {
            return RedisModule_ReplyWithError(ctx, "ERR invalid value: param5 must be withscores or withvalues");
        }
    }

    long long offsetLL = 0;
    long long countLL = 0x7fffffff;
    if (argc > 6)
    {
        size_t limitLen;
        char* limit2 = RedisModule_StringPtrLen(argv[6], &limitLen);
        if (strcmp(limitTmp, limit2) == 0) {
            if (argc < 9)
            {
                return RedisModule_ReplyWithError(ctx, "ERR invalid value: now that param6 is limit, you must write offset and count!!");
            }
            
            RedisModule_StringToLongLong(argv[7], &offsetLL);
            RedisModule_StringToLongLong(argv[8], &countLL);
        }
    }
    
    int offset = offsetLL;
    int count = countLL;

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    zmrangeByScore(zmap, minScore, maxScore, list, callback_zmap_savelist, offset, count);

    int bei = 1 + withScore + withValue;
    RedisModule_ReplyWithArray(ctx, (long) list->size * bei);

    struct xlq_list_node* node = list->head;
    while (1)
    {
        if (node == NULL)
        {
            break;
        }
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        if (withScore)
        {
            RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        }
        if (withValue)
        {
            RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        }
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}


int ZMap_zmrevrangeByScoreCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long minScore;
    if ((RedisModule_StringToLongLong(argv[2], &minScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: param2 is minScore, must be a long long");
    }

    long long maxScore;
    if ((RedisModule_StringToLongLong(argv[3], &maxScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: param3 is maxScore, must be a long long");
    }

    int withScore = 0;
    int withValue = 0;

    if (argc > 4)
    {
        size_t withXXX1Len;
        char* withXXX1 = RedisModule_StringPtrLen(argv[4], &withXXX1Len);
        if (strcmp(withScoresTmp, withXXX1) == 0) {
            withScore = 1;
        }
        if (strcmp(withValueTmp, withXXX1) == 0) {
            withValue = 1;
        }
        if (withScore == 0 && withValue == 0)
        {
            return RedisModule_ReplyWithError(ctx, "ERR invalid value: param4 must be withscores or withvalues");
        }
    }
    if (argc > 5)
    {
        size_t withXXX2Len;
        char* withXXX2 = RedisModule_StringPtrLen(argv[5], &withXXX2Len);
        if (strcmp(withScoresTmp, withXXX2) == 0) {
            withScore = 1;
        }
        if (strcmp(withValueTmp, withXXX2) == 0) {
            withValue = 1;
        }
        if (withScore == 0 && withValue == 0)
        {
            return RedisModule_ReplyWithError(ctx, "ERR invalid value: param5 must be withscores or withvalues");
        }
    }

    long long offsetLL = 0;
    long long countLL = 0x7fffffff;
    if (argc > 6)
    {
        size_t limitLen;
        char* limit2 = RedisModule_StringPtrLen(argv[6], &limitLen);
        if (strcmp(limitTmp, limit2) == 0) {
            if (argc < 9)
            {
                return RedisModule_ReplyWithError(ctx, "ERR invalid value: now that param6 is limit, you must write offset and count!!");
            }

            RedisModule_StringToLongLong(argv[7], &offsetLL);
            RedisModule_StringToLongLong(argv[8], &countLL);
        }
    }

    int offset = offsetLL;
    int count = countLL;

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    zmrevrangeByScore(zmap, minScore, maxScore, list, callback_zmap_savelist, offset, count);

    int bei = 1 + withScore + withValue;
    RedisModule_ReplyWithArray(ctx, (long) list->size * bei);

    struct xlq_list_node* node = list->head;
    while (1)
    {
        if (node == NULL)
        {
            break;
        }
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        if (withScore)
        {
            RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        }
        if (withValue)
        {
            RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        }
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}

int ZMap_zmscoreCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[2], &memberLen);

    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithNull(ctx);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    struct xlq_str* outValue;
    int r = zm_getScoreAndValue(zmap, memberLen, member, &outValue);
    if (r)
    {
        RedisModule_ReplyWithLongLong(ctx, outValue->m_value_8zj);
    }
    else {
        RedisModule_ReplyWithNull(ctx);
    }
    
    return REDISMODULE_OK;
}

int ZMap_zminfoCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 3) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    size_t memberLen;
    char* member = RedisModule_StringPtrLen(argv[2], &memberLen);

    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    struct xlq_str* outValue;
    int r = zm_getScoreAndValue(zmap, memberLen, member, &outValue);
    if (r)
    {
        RedisModule_ReplyWithArray(ctx, (long) 2);
        RedisModule_ReplyWithLongLong(ctx, outValue->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, outValue->m_value2, outValue->m_count2);
    }
    else {
        RedisModule_ReplyWithArray(ctx, (long) 0);
    }

    return REDISMODULE_OK;
}

int ZMap_zmcardCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1],
        REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }

    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);

    if (zmap != NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, zmap->length);
    }
    else {
        RedisModule_ReplyWithLongLong(ctx, 0);
    }

    return REDISMODULE_OK;
}

int ZMap_zmremCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    int delNum = 0;
    for (int i = 2; i < argc; i++)
    {
        size_t memberLen;
        char* memberDel = RedisModule_StringPtrLen(argv[i], &memberLen);
        struct xlq_str* delV;
        if (zm_delByMember(zmap, memberLen, memberDel, &delV)) {
            xlq_free(delV);
            delNum++;
        }
    }

    RedisModule_ReplyWithLongLong(ctx, delNum);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmremReturnOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);

    int delNum = 0;
    for (int i = 2; i < argc; i++)
    {
        size_t memberLen;
        char* memberDel = RedisModule_StringPtrLen(argv[i], &memberLen);
        struct xlq_str* delV;
        if (zm_delByMember(zmap, memberLen, memberDel, &delV)) {
            xlq_list_str_insert(list, delV);
            delNum++;
        }
    }
    
    //RedisModule_ReplyWithLongLong(ctx, delNum);

    if (delNum > 0)
    {
        RedisModule_ReplyWithArray(ctx, (long) delNum * 3);
        struct xlq_list_node* node = list->head;
        while (1)
        {
            struct xlq_str* delV = node->m_list_value;
            RedisModule_ReplyWithStringBuffer(ctx, delV->m_value, delV->m_count);
            RedisModule_ReplyWithLongLong(ctx, delV->m_value_8zj);
            RedisModule_ReplyWithStringBuffer(ctx, delV->m_value2, delV->m_count2);
            if (node->next == NULL)
            {
                break;
            }
            node = node->next;
        }
    }
    else
    {
        RedisModule_ReplyWithArray(ctx, (long) 0);
    }
    
    xlq_list_str_distory(list, 1);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmremByScoreCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    int delNum = 0;
    long long score;
    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    for (int i = 2; i < argc; i++)
    {
        if ((RedisModule_StringToLongLong(argv[i], &score) == REDISMODULE_OK)) {
            delNum += xlq_zmap_delByScore(zmap, score, list);
        }
    }
    xlq_list_str_distory(list, 0);

    RedisModule_ReplyWithLongLong(ctx, delNum);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}


int ZMap_zmremByScoreReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    int delNum = 0;
    long long score;
    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    for (int i = 2; i < argc; i++)
    {
        if ((RedisModule_StringToLongLong(argv[i], &score) == REDISMODULE_OK)) {
            delNum += xlq_zmap_delByScore(zmap, score, list);
        }
    }

    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);

    struct xlq_list_node* node = list->head;
    while (1)
    {
        if (node == NULL)
        {
            break;
        }
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

int ZMap_zmremRangeByScoreCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long minScore;
    if ((RedisModule_StringToLongLong(argv[2], &minScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: minScore must be a long long");
    }
    long long maxScore;
    if ((RedisModule_StringToLongLong(argv[3], &maxScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: maxScore must be a long long");
    }

    int delNum = xlq_zmap_delRangeByScore(zmap, minScore, maxScore, 1, NULL);
    RedisModule_ReplyWithLongLong(ctx, (long) delNum);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmremRangeByScoreReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long minScore;
    if ((RedisModule_StringToLongLong(argv[2], &minScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: minScore must be a long long");
    }
    long long maxScore;
    if ((RedisModule_StringToLongLong(argv[3], &maxScore) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: maxScore must be a long long");
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    int delNum = xlq_zmap_delRangeByScore(zmap, minScore, maxScore, 0, list);

    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);

    struct xlq_list_node* node = list->head;
    while (1)
    {
        if (node == NULL)
        {
            break;
        }
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


int ZMap_zmremRangeByIndexCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long startLL;
    if ((RedisModule_StringToLongLong(argv[2], &startLL) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: start must be a long long");
    }
    long long endLL;
    if ((RedisModule_StringToLongLong(argv[3], &endLL) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: end must be a long long");
    }

    int start = (int) startLL;
    int end = (int) endLL;

    int delNum = xlq_zmap_delRangeByIndex(zmap, start, end, 1, NULL);
    RedisModule_ReplyWithLongLong(ctx, (long) delNum);
    RedisModule_ReplicateVerbatim(ctx);
    return REDISMODULE_OK;
}

int ZMap_zmremRangeByIndexReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 4) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    void* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithLongLong(ctx, (long) 0);
        RedisModule_ReplicateVerbatim(ctx);
        return REDISMODULE_OK;
    }

    long long startLL;
    if ((RedisModule_StringToLongLong(argv[2], &startLL) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: start must be a long long");
    }
    long long endLL;
    if ((RedisModule_StringToLongLong(argv[3], &endLL) != REDISMODULE_OK)) {
        return RedisModule_ReplyWithError(ctx, "ERR invalid value: end must be a long long");
    }

    int start = (int) startLL;
    int end = (int) endLL;

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    int delNum = xlq_zmap_delRangeByIndex(zmap, start, end, 0, list);

    RedisModule_ReplyWithArray(ctx, (long) list->size * 3);

    struct xlq_list_node* node = list->head;
    while (1)
    {
        if (node == NULL)
        {
            break;
        }
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

/// ////////////////以下是辅助功能

int ZMap_zmrangeHashmapCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }

    struct xlq_list_str* list;
    xlq_list_str_create(&list);
    int r = xlq_hashmap_get_all(zmap->m_hashmap, list);
    RedisModule_ReplyWithArray(ctx, (long) list->size * 4);
    struct xlq_list_node* node = list->head;
    while (node)
    {
        struct xlq_str* s = node->m_list_value;
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value, s->m_count);
        RedisModule_ReplyWithLongLong(ctx, s->m_value_8zj);
        RedisModule_ReplyWithStringBuffer(ctx, s->m_value2, s->m_count2);
        RedisModule_ReplyWithLongLong(ctx, r);
        node = node->next;
    }
    xlq_list_str_distory(list, 0);

    return REDISMODULE_OK;
}

void ZMap_zmapinfoCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

    if (argc < 2) return RedisModule_WrongArity(ctx);   //incr You don't have to add value, can only inct score
    RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
    int type = RedisModule_KeyType(key);
    if (type != REDISMODULE_KEYTYPE_EMPTY &&
        RedisModule_ModuleTypeGetType(key) != ZMapType)
    {
        return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
    }
    struct xlq_zmap_t* zmap = NULL;
    GET_ZMAP(type, zmap, key);
    if (zmap == NULL)
    {
        RedisModule_ReplyWithArray(ctx, 0);
        return REDISMODULE_OK;
    }
    RedisModule_ReplyWithArray(ctx, (long) 3);
    RedisModule_ReplyWithLongLong(ctx, (long) zmap->length);
    RedisModule_ReplyWithLongLong(ctx, (long) zmap->m_hashmap->m_size);
    RedisModule_ReplyWithLongLong(ctx, (long) zmap->m_skiplist->length);
    return REDISMODULE_OK;
}

void ZMap_meminfoCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    RedisModule_AutoMemory(ctx); /* Use automatic memory management. */
    RedisModule_ReplyWithArray(ctx, (long) 3);
    int no = 0;
    int addNum = 0;
    int delNum = 0;
    xlq_get_meminfo(&no, &addNum, &delNum);
    RedisModule_ReplyWithLongLong(ctx, (long) no);
    RedisModule_ReplyWithLongLong(ctx, (long) addNum);
    RedisModule_ReplyWithLongLong(ctx, (long) delNum);
    return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
    REDISMODULE_NOT_USED(argv);
    REDISMODULE_NOT_USED(argc);

    if (RedisModule_Init(ctx, "zmaptype", 1, REDISMODULE_APIVER_1)
        == REDISMODULE_ERR) return REDISMODULE_ERR;

    RedisModuleTypeMethods tm = {
        .version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = ZMapTypeRdbLoad,
        .rdb_save = ZMapTypeRdbSave,
        .aof_rewrite = ZMapTypeAofRewrite,
        .mem_usage = ZMapTypeMemUsage,
        .free = ZMapTypeFree,
//        .digest = ZMapTypeDigest
    };

    ZMapType = RedisModule_CreateDataType(ctx, "xzmaptype", 0, &tm);
    if (ZMapType == NULL) return REDISMODULE_ERR;

    CREATE_WRCMD("xlq.zmadd", ZMap_zmaddCmd);
    CREATE_WRCMD("xlq.zmaddReOri", ZMap_zmaddReturnOriCmd);
    CREATE_WRCMD("xlq.zmincrby", ZMap_zmincrByCmd);
    CREATE_WRCMD("xlq.zmincrbyReOri", ZMap_zmincrByReturnOriCmd);
    CREATE_ROCMD("xlq.zmrangebyscore", ZMap_zmrangeByScoreCmd);
    CREATE_ROCMD("xlq.zmrevrangebyscore", ZMap_zmrevrangeByScoreCmd);

    CREATE_ROCMD("xlq.zmscore", ZMap_zmscoreCmd);
    CREATE_ROCMD("xlq.zminfo", ZMap_zminfoCmd);
    CREATE_ROCMD("xlq.zmcard", ZMap_zmcardCmd);

    CREATE_WRCMD("xlq.zmrem", ZMap_zmremCmd);
    CREATE_WRCMD("xlq.zmremReOri", ZMap_zmremReturnOriCmd);
    CREATE_WRCMD("xlq.zmremByScore", ZMap_zmremByScoreCmd);
    CREATE_WRCMD("xlq.zmremByScoreReOri", ZMap_zmremByScoreReOriCmd);
    CREATE_WRCMD("xlq.zmremRangeByScore", ZMap_zmremRangeByScoreCmd);
    CREATE_WRCMD("xlq.zmremRangeByScoreReOri", ZMap_zmremRangeByScoreReOriCmd);
    CREATE_WRCMD("xlq.zmremRangeByIndex", ZMap_zmremRangeByIndexCmd);
    CREATE_WRCMD("xlq.zmremRangeByIndexReOri", ZMap_zmremRangeByIndexReOriCmd);

    //--------------------------------------------------------
    RedisModuleTypeMethods hm = {
        .version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = NMapTypeRdbLoad,
        .rdb_save = NMapTypeRdbSave,
        .aof_rewrite = NMapTypeAofRewrite,
        .mem_usage = NMapTypeMemUsage,
        .free = NMapTypeFree,
        //        .digest = ZMapTypeDigest
    };

    void * NMapType = RedisModule_CreateDataType(ctx, "xnmaptype", 0, &hm);
    if (NMapType == NULL) return REDISMODULE_ERR;
    NMap_setNMapType(NMapType);

    CREATE_WRCMD("xlq.hnset", NMap_hnsetCmd);
    CREATE_WRCMD("xlq.hnsetReOri", NMap_hnsetReOriCmd);
    CREATE_WRCMD("xlq.hnincrby", NMap_hnincrbyCmd);
    CREATE_WRCMD("xlq.hnincrbyReOri", NMap_hnincrbyReOriCmd);
    CREATE_ROCMD("xlq.hnget", NMap_hngetCmd);
    CREATE_ROCMD("xlq.hngetall", NMap_hngetallCmd);
    CREATE_WRCMD("xlq.hndel", NMap_hndelCmd);
    CREATE_WRCMD("xlq.hndelReOri", NMap_hndelReOriCmd);

    CREATE_ROCMD("xlq.hnlen", NMap_hnlenCmd);
    CREATE_ROCMD("xlq.hnkeys", NMap_hnkeysCmd);
    CREATE_ROCMD("xlq.hnexists", NMap_hnexistsCmd);

    //--------------------------------------------------------


    //--------------------------------------------------------
    RedisModuleTypeMethods loopm = {
        .version = REDISMODULE_TYPE_METHOD_VERSION,
        .rdb_load = LoopArrTypeRdbLoad,
        .rdb_save = LoopArrTypeRdbSave,
        .aof_rewrite = LoopArrTypeAofRewrite,
        .mem_usage = LoopArrTypeMemUsage,
        .free = LoopArrTypeFree,
        //        .digest = ZMapTypeDigest
    };

    void* LoopArrType = RedisModule_CreateDataType(ctx, "xlooptype", 0, &loopm);
    if (LoopArrType == NULL) return REDISMODULE_ERR;
    LoopArr_setLooparrayType(LoopArrType);

    CREATE_WRCMD("xlq.loopstrCreate", LoopArr_loopstrCreateCmd);
    CREATE_WRCMD("xlq.loopstrCreateOrResize", LoopArr_loopstrCreateOrResizeCmd);
    CREATE_WRCMD("xlq.loopstrInsert", LoopArr_loopstrInsertCmd);
    CREATE_ROCMD("xlq.loopstrRevrange", LoopArr_loopstrRevrangeCmd);
    CREATE_ROCMD("xlq.loopstrInfo", LoopArr_loopstrInfoCmd);

    //--------------------------------------------------------

    CREATE_ROCMD("xlq.zmrangeHashmap", ZMap_zmrangeHashmapCmd);
    CREATE_ROCMD("xlq.meminfo", ZMap_meminfoCmd);
    CREATE_ROCMD("xlq.zmapinfo", ZMap_zmapinfoCmd);

    return REDISMODULE_OK;
}
