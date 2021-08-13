#ifndef __XLQ_MODULE_LOOP_ARRAY__
#define __XLQ_MODULE_LOOP_ARRAY__

#include "redismodule.h"

void LoopArr_setLooparrayType(void* _looparrayType);

int LoopArr_loopstrCreateCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int LoopArr_loopstrCreateOrResizeCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int LoopArr_loopstrInsertCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int LoopArr_loopstrInfoCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int LoopArr_loopstrRevrangeCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

/* ========================== "LoopArrtype" type methods ======================= */

void* LoopArrTypeRdbLoad(RedisModuleIO* rdb, int encver);

void LoopArrTypeRdbSave(RedisModuleIO* rdb, void* value);

void LoopArrTypeAofRewrite(RedisModuleIO* rdb, RedisModuleString* key, void* value);

size_t LoopArrTypeMemUsage(void* value);

void LoopArrTypeFree(void* value);
#endif