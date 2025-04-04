/**
 * Copyright 2024 TotalJustice.
 * SPDX-License-Identifier: MIT
 */
#ifndef ARGS_H
#define ARGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>

/*
 * TODO:
 * - handle combined args, like -abc -defg
*/

typedef enum ArgsValueType {
  // key only, no value needed
  ArgsValueType_NONE,
  // STR saves ptr to the raw argv value
  ArgsValueType_STR,
  // INT will also handle hex values!
  ArgsValueType_INT,
  // can be used for floats also.
  ArgsValueType_DOUBLE,
  // BOOL will handle 1,0,true,false
  ArgsValueType_BOOL,
} ArgsValueType;

typedef enum ArgsResult {
  // unknown key was found.
  // check argv[index] for bad key string
  ArgsResult_UNKNOWN_KEY = -4,
  // got a value missmatch (ie, bool instead of str)
  // check data.value.s for bad value string
  ArgsResult_BAD_VALUE = -3,
  // value wanted, but didn't get anything
  // check meta[data.meta_index].key for key
  ArgsResult_MISSING_VALUE = -2,
  // generic error...
  ArgsResult_ERROR = -1,
  // all good!
  ArgsResult_OK = 0,
  // returned when finished looping through argv.
  ArgsResult_DONE = 1,
  // trailing value found at the end of args.
  ArgsResult_EXTRA_ARGS = 2,
} ArgsResult;

typedef struct ArgsMeta {
  const char* key;         // the name of the key
  int id;                  // the ID for this key, optional.
  enum ArgsValueType type; // value type
  char single;             // if not 0, enables single args, such as -v
} ArgsMeta;

typedef union ArgsValue {
  const char* s;
  long long i;
  double d;
  bool b;
} ArgsValue;

typedef struct ArgsData {
  size_t meta_index;     // index into the meta
  union ArgsValue value; // union of value types
} ArgsData;

enum ArgsResult args_parse(
  // to skip argv[0] (program name), set the index to 1.
  int* index, int argc, char* const* const argv,
  // the keys that you want to search.
  const struct ArgsMeta* metas, size_t meta_count,
  // the parsed data out.
  struct ArgsData* data_out
);

#ifdef __cplusplus
}
#endif

#endif // ARGS_H
