/***********************************************************************************************************************************
Key Value Handler
***********************************************************************************************************************************/
#ifndef COMMON_TYPE_KEYVALUE_H
#define COMMON_TYPE_KEYVALUE_H

#include "common/type/variant.h"

/***********************************************************************************************************************************
Key value object
***********************************************************************************************************************************/
typedef struct KeyValue KeyValue;

/***********************************************************************************************************************************
Functions
***********************************************************************************************************************************/
KeyValue *kvNew();
KeyValue *kvDup(const KeyValue *source);
KeyValue *kvAdd(KeyValue *this, const Variant *key, const Variant *value);
const VariantList *kvKeyList(const KeyValue *this);
KeyValue *kvPut(KeyValue *this, const Variant *key, const Variant *value);
KeyValue *kvPutKv(KeyValue *this, const Variant *key);
const Variant *kvGet(const KeyValue *this, const Variant *key);
VariantList *kvGetList(const KeyValue *this, const Variant *key);
void kvFree(KeyValue *this);

#endif
