/**
 * Copyright 2024 TotalJustice.
 * SPDX-License-Identifier: MIT
 */
#include "args.h"

#include <stddef.h> // size_t, NULL
#include <stdlib.h> // stroll
#include <limits.h> // int limits
#include <float.h> // double limits
#include <string.h> // strlen, strcmp

struct ArgsIsKeyResult {
  // will be NULL if no key found
  const char* str;
  // length of the key, not NULL terminated
  size_t len;
  // is the key a single char
  bool is_single;
  // is the next char '=', used for seeing is value is part of key str
  bool equals_next;
  // is this an ending dilem "--"
  bool end_dilem;
};

// strlen, but for keys (checks for '\0' and '=')
static size_t keylen(const char* str) {
  size_t len = 0;

  while (str[len] != '\0' && str[len] != '=') {
    len++;
  }

  return len;
}


// similar to strncmp, but checks that both strings are the same size
// meaning, it'll check up to [stra_len], then check that [strb] ends
// in '\0', meaning we found a match
static int custom_strncmp(const char* stra, const char* strb, unsigned stra_len) {
  for (unsigned i = 0; i < stra_len; ++i) {
    if (stra[i] != strb[i]) {
      return stra[i] - strb[i];
    }
  }

  if (strb[stra_len] == '\0') {
    return 0;
  }

  return 1;
}

static struct ArgsIsKeyResult args_is_key_internal(const char* str) {
  struct ArgsIsKeyResult result = {0};
  size_t offset = 1;

  // check that we have a string
  if (!str) {
    goto fail;
  }

  // check that this is a key.
  if (str[0] != '-') {
    goto fail;
  }
  // check if this is a double key "--key"
  else if (str[1] == '-') {
    offset++;
    // check for ending dilem "--"
    if (str[2] == '\0') {
      result.end_dilem = true;
    }
  }
  // check that the single "-" has a key ie "-k"
  else if (str[1] == '\0') {
    goto fail;
  }
  // check that the single dilem is 1 character long, "-key" is invalid.
  // however, "-k=value" is valid.
  else if (str[2] != '\0' && str[2] != '=') {
    goto fail;
  }

  result.str = str + offset;
  result.len = keylen(result.str);
  result.is_single = offset == 1;
  result.equals_next = result.str[result.len] == '=';

  return result;

fail:
  result.str = NULL;
  return result;
}

static bool get_value_int(const char* str, long long* out) {
  int base = 10;
  int off = 0;

  if (str[0] == '-' || str[0] == '+') {
    off = 1;
  }

  // this allows support for hex and base 10
  if (str[off] == '0' && (str[off + 1] == 'x' || str[off + 1] == 'X')) {
    base = 16;
  }

  char* end_str = NULL;
  *out = strtoll(str, &end_str, base);

  if (str == end_str || end_str == NULL || *end_str != '\0') {
    return false;
  }

  // potential false positive, would need to check errno
  if (*out == LLONG_MAX || *out == -LLONG_MAX) {
    return false;
  }

  return true;
}

static bool get_value_double(const char* str, double* out) {
  char* end_str = NULL;
  *out = strtod(str, &end_str);

  if (str == end_str || end_str == NULL || *end_str != '\0') {
    return false;
  }

  // potential false positive, would need to check errno
  if (*out == DBL_MAX || *out == -DBL_MAX) {
    return false;
  }

  return true;
}

static bool get_value_bool(const char* str, bool* out) {
  // booleans can either be "true","false" or "1","0"
  if (!strcmp(str, "1")) {
    *out = true;
    return true;
  }
  else if (!strcmp(str, "0")) {
    *out = false;
    return true;
  }
  else if (!strcmp(str, "true")) {
    *out = true;
    return true;
  }
  else if (!strcmp(str, "false")) {
    *out = false;
    return true;
  }

  return false;
}

enum ArgsResult args_parse(
  int* index, int argc, char* const* const argv,
  const struct ArgsMeta* metas, size_t meta_count,
  struct ArgsData* data_out
) {
  // validate params.
  if (!index || argc <= 0 || !argv || !metas || !meta_count || !data_out) {
    return ArgsResult_ERROR;
  }

  int i = *index;

  // check if we are finished.
  if (i >= argc) {
    return ArgsResult_DONE;
  }

  // check if args without a key are valid.
  const struct ArgsIsKeyResult result = args_is_key_internal(argv[i]);

  if (result.str == NULL) {
    // check if this is the last entry.
    if (i + 1 == argc) {
      return ArgsResult_EXTRA_ARGS;
    }
    else {
      return ArgsResult_ERROR;
    }
  }
  else if (result.end_dilem) {
    *index = i + 1;
    return ArgsResult_DONE;
  }

  for (size_t j = 0; j < meta_count; j++) {
    if (
      // check if single is wanted and found.
      (result.is_single && metas[j].single && result.str[0] == metas[j].single) ||
      // check if key is wanted and found.
      (metas[j].key && !custom_strncmp(result.str, metas[j].key, result.len))
    ) {
      // found a key match!
      data_out->meta_index = j;

      const char* value_str = NULL;

      // if the key was [key=], then the value is part
      // of the same argv[] entry.
      if (result.equals_next) {
        value_str = result.str + result.len + 1;
      }
      // otherwise, the next argv[] is the value (if any)
      else {
        if (i + 1 < argc) {
          const struct ArgsIsKeyResult next_key = args_is_key_internal(argv[i + 1]);

          // if NULL, next argv is not a key, likely a value
          if (next_key.str == NULL) {
            value_str = argv[i + 1];
            i++;
          }
        }
      }

      // check if a value is required and that we have one.
      if (!value_str && metas[j].type != ArgsValueType_NONE) {
        return ArgsResult_MISSING_VALUE;
      }

      switch (metas[j].type) {
        case ArgsValueType_NONE:
          break;

        case ArgsValueType_STR:
          data_out->value.s = value_str;
          break;

        case ArgsValueType_INT:
          if (!get_value_int(
            value_str, &data_out->value.i)
          ) {
            data_out->value.s = value_str;
            return ArgsResult_BAD_VALUE;
          }
          break;

        case ArgsValueType_DOUBLE:
          if (!get_value_double(
            value_str, &data_out->value.d)
          ) {
            data_out->value.s = value_str;
            return ArgsResult_BAD_VALUE;
          }
          break;

        case ArgsValueType_BOOL:
          if (!get_value_bool(
            value_str, &data_out->value.b)
          ) {
            data_out->value.s = value_str;
            return ArgsResult_BAD_VALUE;
          }
          break;
      }

      *index = i + 1;
      return ArgsResult_OK;
    }
  }

  return ArgsResult_UNKNOWN_KEY;
}
