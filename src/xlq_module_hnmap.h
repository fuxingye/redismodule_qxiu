#ifndef __XLQ_MODULE_NMAP__
#define __XLQ_MODULE_NMAP__

#include "redismodule.h"

void NMap_setNMapType(RedisModuleType* _NMapType);

int NMap_hnsetCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnsetReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnincrbyCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnincrbyReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hngetCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hngetallCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hndelCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hndelReOriCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnlenCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnkeysCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

int NMap_hnexistsCmd(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);


void* NMapTypeRdbLoad(RedisModuleIO* rdb, int encver);

void NMapTypeRdbSave(RedisModuleIO* rdb, void* value);

void NMapTypeAofRewrite(RedisModuleIO* rdb, RedisModuleString* key, void* value);

/* The goal of this function is to return the amount of memory used by
 * the HelloType value. */
size_t NMapTypeMemUsage(void* value);

void NMapTypeFree(void* value);

void NMapTypeDigest(RedisModuleDigest* md, void* value);
#endif
