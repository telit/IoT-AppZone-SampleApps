/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*
 Copyright (c) 2009 Dave Gamble

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

/*!
 @file
     azx_cjson.c

 @brief
     JSON objects manipulation

 @details
     JSON objects manipulation implementation

 @version
     1.0.0

 @notes
     Porting on m2mb of the AZX_CJSON_T library.

 @author Fabio Pintus
 @author Norman Argiolas

 @date
     04/02/2020
 */

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>
#include <ctype.h>

#include "m2mb_types.h"
#include "m2mb_os_api.h"

#include "azx_cjson.h"

#define DBL_EPSILON_1 2.2204460492503131E-16

static const char *global_ep;

const char *azx_cjson_getErrorPtr(void)
{
  return global_ep;
}

static int cJSON_strcasecmp(const char *s1, const char *s2)
{
  if (!s1)
    return (s1 == s2) ? 0 : 1;
  if (!s2)
    return 1;
  for (; tolower(*s1) == tolower(*s2); ++s1, ++s2)
    if (*s1 == 0)
      return 0;
  return tolower(*(const unsigned char *) s1)
      - tolower(*(const unsigned char *) s2);
}

#define cJSON_malloc m2mb_os_malloc
#define cJSON_free m2mb_os_free

static char* cJSON_strdup(const char* str)
{
  size_t len;
  char* copy;

  len = strlen(str) + 1;
  if (!(copy = (char*) cJSON_malloc(len)))
    return 0;
  memcpy(copy, str, len);
  return copy;
}

/* Internal constructor. */
static AZX_CJSON_T *cJSON_New_Item(void)
{
 AZX_CJSON_T* node = (AZX_CJSON_T*) cJSON_malloc(sizeof(AZX_CJSON_T));
  if (node)
    memset(node, 0, sizeof(AZX_CJSON_T));
  return node;
}

/* Delete a AZX_CJSON_T structure. */
void azx_cjson_delete(AZX_CJSON_T *c)
{
  AZX_CJSON_T *next;
  while (c)
  {
    next = c->next;
    if (!(c->type & AZX_CJSON_IS_REFERENCE) && c->child)
      azx_cjson_delete(c->child);
    if (!(c->type & AZX_CJSON_IS_REFERENCE) && c->valuestring)
      cJSON_free(c->valuestring);
    if (!(c->type & AZX_CJSON_STRING_IS_CONST) && c->string)
      cJSON_free(c->string);
    cJSON_free(c);
    c = next;
  }
}

/* Parse the input text to generate a number, and populate the result into item. */
static const char *parse_number(AZX_CJSON_T *item, const char *num)
{
  double n = 0;
  int sign = 1, scale = 0, subscale = 0, signsubscale = 1;

  if (*num == '-')
    sign = -1, num++; /* Has sign? */
  if (*num == '0')
    num++; /* is zero */
  if (*num >= '1' && *num <= '9')
    do
    {
      n = (n * 10.0) + (*num - '0');
      num++;
    } while (*num >= '0' && *num <= '9'); /* Number? */
  if (*num == '.' && num[1] >= '0' && num[1] <= '9')
  {
    num++;
    do
    {
      n = (n * 10.0) + (*num - '0');
      scale--;
      num++;
    } while (*num >= '0' && *num <= '9');
  } /* Fractional part? */

  if (*num == 'e' || *num == 'E') /* Exponent? */
  {
    num++;
    if (*num == '+')
      num++;
    else if (*num == '-')
      signsubscale = -1, num++; /* With sign? */
    while (*num >= '0' && *num <= '9')
    {
      subscale = (subscale * 10) + (*num - '0'); /* Number? */
      num++;
    }
  }

  // TODO  this is locking the m2m app. find why
  // n=sign*n*pow(10.0,(scale+subscale*signsubscale));  /* number = +/- number.fraction * 10^+/- exponent */
  int k;
  if (scale + subscale * signsubscale < 0)
  {
    for (k = 0; k > scale + subscale * signsubscale; k--)
      n = n / 10.0;
  } else
  {
    for (k = 0; k < scale + subscale * signsubscale; k++)
      n = n * 10.0;
  }

  n = n * sign;

  item->valuedouble = n;
  item->valueint = (int) n;
  item->type = AZX_CJSON_NUMBER;
  return num;
}

static int pow2gt(int x)
{
  --x;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  return x + 1;
}

typedef struct {
  char *buffer;
  int length;
  int offset;
} printbuffer;

static char* ensure(printbuffer *p, int needed)
{
  char *newbuffer;
  int newsize;
  if (!p || !p->buffer)
    return 0;
  needed += p->offset;
  if (needed <= p->length)
    return p->buffer + p->offset;

  newsize = pow2gt(needed);
  newbuffer = (char*) cJSON_malloc(newsize);
  if (!newbuffer)
  {
    cJSON_free(p->buffer);
    p->length = 0, p->buffer = 0;
    return 0;
  }
  if (newbuffer)
    memcpy(newbuffer, p->buffer, p->length);
  cJSON_free(p->buffer);
  p->length = newsize;
  p->buffer = newbuffer;
  return newbuffer + p->offset;
}

static int update(printbuffer *p)
{
  char *str;
  if (!p || !p->buffer)
    return 0;
  str = p->buffer + p->offset;
  return p->offset + strlen(str);
}

/* Render the number nicely from the given item into a string. */
static char *print_number(AZX_CJSON_T *item, printbuffer *p)
{
  char *str = 0;
  double d = item->valuedouble;
  if (d == 0)
  {
    if (p)
      str = ensure(p, 2);
    else
      str = (char*) cJSON_malloc(2); /* special case for 0. */
    if (str)
      strcpy(str, "0");
  } else if (fabs(((double) item->valueint) - d) <= DBL_EPSILON_1
      && d <= INT_MAX && d >= INT_MIN)
  {
    if (p)
      str = ensure(p, 21);
    else
      str = (char*) cJSON_malloc(21); /* 2^64+1 can be represented in 21 chars. */
    if (str)
      sprintf(str, "%d", item->valueint);
  } else
  {
    if (p)
      str = ensure(p, 64);
    else
      str = (char*) cJSON_malloc(64); /* This is a nice tradeoff. */
    if (str)
    {
      if (d * 0 != 0)
        sprintf(str, "null"); /* This checks for NaN and Infinity */
      else if (fabs(floor(d) - d) <= DBL_EPSILON_1 && fabs(d) < 1.0e60)
        sprintf(str, "%.0f", d);
      else if (fabs(d) < 1.0e-6 || fabs(d) > 1.0e9)
        sprintf(str, "%e", d);
      else
        sprintf(str, "%f", d);
    }
  }
  return str;
}

static unsigned parse_hex4(const char *str)
{
  unsigned h = 0;
  if (*str >= '0' && *str <= '9')
    h += (*str) - '0';
  else if (*str >= 'A' && *str <= 'F')
    h += 10 + (*str) - 'A';
  else if (*str >= 'a' && *str <= 'f')
    h += 10 + (*str) - 'a';
  else
    return 0;
  h = h << 4;
  str++;
  if (*str >= '0' && *str <= '9')
    h += (*str) - '0';
  else if (*str >= 'A' && *str <= 'F')
    h += 10 + (*str) - 'A';
  else if (*str >= 'a' && *str <= 'f')
    h += 10 + (*str) - 'a';
  else
    return 0;
  h = h << 4;
  str++;
  if (*str >= '0' && *str <= '9')
    h += (*str) - '0';
  else if (*str >= 'A' && *str <= 'F')
    h += 10 + (*str) - 'A';
  else if (*str >= 'a' && *str <= 'f')
    h += 10 + (*str) - 'a';
  else
    return 0;
  h = h << 4;
  str++;
  if (*str >= '0' && *str <= '9')
    h += (*str) - '0';
  else if (*str >= 'A' && *str <= 'F')
    h += 10 + (*str) - 'A';
  else if (*str >= 'a' && *str <= 'f')
    h += 10 + (*str) - 'a';
  else
    return 0;
  return h;
}

/* Parse the input text into an unescaped cstring, and populate item. */
static const unsigned char firstByteMark[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0,
    0xF8, 0xFC };
static const char *parse_string(AZX_CJSON_T *item, const char *str,
    const char **ep)
{
  const char *ptr = str + 1, *end_ptr = str + 1;
  char *ptr2;
  char *out;
  int len = 0;
  unsigned uc, uc2;
  if (*str != '\"')
  {
    *ep = str;
    return 0;
  } /* not a string! */

  while (*end_ptr != '\"' && *end_ptr && ++len)
    if (*end_ptr++ == '\\')
      end_ptr++; /* Skip escaped quotes. */

  out = (char*) cJSON_malloc(len + 1); /* This is how long we need for the string, roughly. */
  if (!out)
    return 0;
  item->valuestring = out; /* assign here so out will be deleted during azx_cjson_delete() later */
  item->type = AZX_CJSON_STRING;

  ptr = str + 1;
  ptr2 = out;
  while (ptr < end_ptr)
  {
    if (*ptr != '\\')
      *ptr2++ = *ptr++;
    else
    {
      ptr++;
      switch (*ptr) {
      case 'b':
        *ptr2++ = '\b';
        break;
      case 'f':
        *ptr2++ = '\f';
        break;
      case 'n':
        *ptr2++ = '\n';
        break;
      case 'r':
        *ptr2++ = '\r';
        break;
      case 't':
        *ptr2++ = '\t';
        break;
      case 'u': /* transcode utf16 to utf8. */
        uc = parse_hex4(ptr + 1);
        ptr += 4; /* get the unicode char. */
        if (ptr >= end_ptr)
        {
          *ep = str;
          return 0;
        } /* invalid */

        if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0)
        {
          *ep = str;
          return 0;
        } /* check for invalid.   */

        if (uc >= 0xD800 && uc <= 0xDBFF) /* UTF16 surrogate pairs. */
        {
          if (ptr + 6 > end_ptr)
          {
            *ep = str;
            return 0;
          } /* invalid */
          if (ptr[1] != '\\' || ptr[2] != 'u')
          {
            *ep = str;
            return 0;
          } /* missing second-half of surrogate.    */
          uc2 = parse_hex4(ptr + 3);
          ptr += 6;
          if (uc2 < 0xDC00 || uc2 > 0xDFFF)
          {
            *ep = str;
            return 0;
          } /* invalid second-half of surrogate.    */
          uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
        }

        len = 4;
        if (uc < 0x80)
          len = 1;
        else if (uc < 0x800)
          len = 2;
        else if (uc < 0x10000)
          len = 3;
        ptr2 += len;

        switch (len) {
        case 4:
          *--ptr2 = ((uc | 0x80) & 0xBF);
          uc >>= 6;
          break;
        case 3:
          *--ptr2 = ((uc | 0x80) & 0xBF);
          uc >>= 6;
          break;
        case 2:
          *--ptr2 = ((uc | 0x80) & 0xBF);
          uc >>= 6;
          break;
        case 1:
          *--ptr2 = (uc | firstByteMark[len]);
          break;
        }
        ptr2 += len;
        break;
      default:
        *ptr2++ = *ptr;
        break;
      }
      ptr++;
    }
  }
  *ptr2 = 0;
  if (*ptr == '\"')
    ptr++;
  return ptr;
}

/* Render the cstring provided to an escaped version that can be printed. */
static char *print_string_ptr(const char *str, printbuffer *p)
{
  const char *ptr;
  char *ptr2, *out;
  int len = 0, flag = 0;
  unsigned char token;

  if (!str)
  {
    if (p)
      out = ensure(p, 3);
    else
      out = (char*) cJSON_malloc(3);
    if (!out)
      return 0;
    strcpy(out, "\"\"");
    return out;
  }

  for (ptr = str; *ptr; ptr++)
    flag |=
        ((*ptr > 0 && *ptr < 32) || (*ptr == '\"') || (*ptr == '\\')) ? 1 : 0;
  if (!flag)
  {
    len = ptr - str;
    if (p)
      out = ensure(p, len + 3);
    else
      out = (char*) cJSON_malloc(len + 3);
    if (!out)
      return 0;
    ptr2 = out;
    *ptr2++ = '\"';
    strcpy(ptr2, str);
    ptr2[len] = '\"';
    ptr2[len + 1] = 0;
    return out;
  }

  ptr = str;
  while ((token = *ptr) && ++len)
  {
    if (strchr("\"\\\b\f\n\r\t", token))
      len++;
    else if (token < 32)
      len += 5;
    ptr++;
  }

  if (p)
    out = ensure(p, len + 3);
  else
    out = (char*) cJSON_malloc(len + 3);
  if (!out)
    return 0;

  ptr2 = out;
  ptr = str;
  *ptr2++ = '\"';
  while (*ptr)
  {
    if ((unsigned char) *ptr > 31 && *ptr != '\"' && *ptr != '\\')
      *ptr2++ = *ptr++;
    else
    {
      *ptr2++ = '\\';
      switch (token = *ptr++) {
      case '\\':
        *ptr2++ = '\\';
        break;
      case '\"':
        *ptr2++ = '\"';
        break;
      case '\b':
        *ptr2++ = 'b';
        break;
      case '\f':
        *ptr2++ = 'f';
        break;
      case '\n':
        *ptr2++ = 'n';
        break;
      case '\r':
        *ptr2++ = 'r';
        break;
      case '\t':
        *ptr2++ = 't';
        break;
      default:
        sprintf(ptr2, "u%04x", token);
        ptr2 += 5;
        break; /* escape and print */
      }
    }
  }
  *ptr2++ = '\"';
  *ptr2++ = 0;
  return out;
}
/* Invote print_string_ptr (which is useful) on an item. */
static char *print_string(AZX_CJSON_T *item, printbuffer *p)
{
  return print_string_ptr(item->valuestring, p);
}

/* Predeclare these prototypes. */
static const char *parse_value(AZX_CJSON_T *item, const char *value,
    const char **ep);
static char *print_value(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p);
static const char *parse_array(AZX_CJSON_T *item, const char *value,
    const char **ep);
static char *print_array(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p);
static const char *parse_object(AZX_CJSON_T *item, const char *value,
    const char **ep);
static char *print_object(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p);

/* Utility to jump whitespace and cr/lf */
static const char *skip(const char *in)
{
  while (in && *in && (unsigned char) *in <= 32)
    in++;
  return in;
}

/* Parse an object - create a new root, and populate. */
AZX_CJSON_T *azx_cjson_parseWithOpts(const char *value,
    const char **return_parse_end, int require_null_terminated)
{
  const char *end = 0, **ep = return_parse_end ? return_parse_end : &global_ep;
  AZX_CJSON_T *c = cJSON_New_Item();
  *ep = 0;
  if (!c)
    return 0; /* memory fail */
  end = parse_value(c, skip(value), ep);
  if (!end)
  {
    azx_cjson_delete(c);
    return 0;
  } /* parse failure. ep is set. */
  // PrintToUart("done\r\n");

  /* if we require null-terminated JSON without appended garbage, skip and then check for a null terminator */
  if (require_null_terminated)
  {
    end = skip(end);
    if (*end)
    {
      azx_cjson_delete(c);
      *ep = end;
      return 0;
    }
  }
  if (return_parse_end)
    *return_parse_end = end;
  return c;
}
/* Default options for azx_cjson_parse */
AZX_CJSON_T *azx_cjson_parse(const char *value)
{
  return azx_cjson_parseWithOpts(value, 0, 0);
}

/* Render a AZX_CJSON_T item/entity/structure to text. */
char *azx_cjson_print(AZX_CJSON_T *item)
{
  return print_value(item, 0, 1, 0);
}
char *azx_cjson_printUnformatted(AZX_CJSON_T *item)
{
  return print_value(item, 0, 0, 0);
}

char *azx_cjson_printBuffered(AZX_CJSON_T *item, int prebuffer, int fmt)
{
  printbuffer p;
  p.buffer = (char*) cJSON_malloc(prebuffer);
  p.length = prebuffer;
  p.offset = 0;
  return print_value(item, 0, fmt, &p);
}

/* Parser core - when encountering text, process appropriately. */
static const char *parse_value(AZX_CJSON_T *item, const char *value,
    const char **ep)
{
  if (!value)
    return 0; /* Fail on null. */
  if (!strncmp(value, "null", 4))
  {
    item->type = AZX_CJSON_NULL;
    return value + 4;
  }
  if (!strncmp(value, "false", 5))
  {
    item->type = AZX_CJSON_FALSE;
    return value + 5;
  }
  if (!strncmp(value, "true", 4))
  {
    item->type = AZX_CJSON_TRUE;
    item->valueint = 1;
    return value + 4;
  }
  if (*value == '\"')
  {
    return parse_string(item, value, ep);
  }
  if (*value == '-' || (*value >= '0' && *value <= '9'))
  {
    return parse_number(item, value);
  }
  if (*value == '[')
  {
    return parse_array(item, value, ep);
  }
  if (*value == '{')
  {
    return parse_object(item, value, ep);
  }

  *ep = value;
  return 0; /* failure. */
}

/* Render a value to text. */
static char *print_value(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p)
{
  char *out = 0;
  if (!item)
    return 0;
  if (p)
  {
    switch ((item->type) & 255) {
    case AZX_CJSON_NULL:
    {
      out = ensure(p, 5);
      if (out)
        strcpy(out, "null");
      break;
    }
    case AZX_CJSON_FALSE:
    {
      out = ensure(p, 6);
      if (out)
        strcpy(out, "false");
      break;
    }
    case AZX_CJSON_TRUE:
    {
      out = ensure(p, 5);
      if (out)
        strcpy(out, "true");
      break;
    }
    case AZX_CJSON_NUMBER:
      out = print_number(item, p);
      break;
    case AZX_CJSON_STRING:
      out = print_string(item, p);
      break;
    case AZX_CJSON_ARRAY:
      out = print_array(item, depth, fmt, p);
      break;
    case AZX_CJSON_OBJECT:
      out = print_object(item, depth, fmt, p);
      break;
    }
  } else
  {
    switch ((item->type) & 255) {
    case AZX_CJSON_NULL:
      out = cJSON_strdup("null");
      break;
    case AZX_CJSON_FALSE:
      out = cJSON_strdup("false");
      break;
    case AZX_CJSON_TRUE:
      out = cJSON_strdup("true");
      break;
    case AZX_CJSON_NUMBER:
      out = print_number(item, 0);
      break;
    case AZX_CJSON_STRING:
      out = print_string(item, 0);
      break;
    case AZX_CJSON_ARRAY:
      out = print_array(item, depth, fmt, 0);
      break;
    case AZX_CJSON_OBJECT:
      out = print_object(item, depth, fmt, 0);
      break;
    }
  }
  return out;
}

/* Build an array from input text. */
static const char *parse_array(AZX_CJSON_T *item, const char *value,
    const char **ep)
{
  AZX_CJSON_T *child;
  if (*value != '[')
  {
    *ep = value;
    return 0;
  } /* not an array! */

  item->type = AZX_CJSON_ARRAY;
  value = skip(value + 1);
  if (*value == ']')
    return value + 1; /* empty array. */

  item->child = child = cJSON_New_Item();
  if (!item->child)
    return 0; /* memory fail */
  value = skip(parse_value(child, skip(value), ep)); /* skip any spacing, get the value. */
  if (!value)
    return 0;

  while (*value == ',')
  {
    AZX_CJSON_T *new_item;
    if (!(new_item = cJSON_New_Item()))
      return 0; /* memory fail */
    child->next = new_item;
    new_item->prev = child;
    child = new_item;
    value = skip(parse_value(child, skip(value + 1), ep));
    if (!value)
      return 0; /* memory fail */
  }

  if (*value == ']')
    return value + 1; /* end of array */
  *ep = value;
  return 0; /* malformed. */
}

/* Render an array to text */
static char *print_array(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p)
{
  char **entries;
  char *out = 0, *ptr, *ret;
  int len = 5;
  AZX_CJSON_T *child = item->child;
  int numentries = 0, i = 0, fail = 0;
  size_t tmplen = 0;

  /* How many entries in the array? */
  while (child)
    numentries++, child = child->next;
  /* Explicitly handle numentries==0 */
  if (!numentries)
  {
    if (p)
      out = ensure(p, 3);
    else
      out = (char*) cJSON_malloc(3);
    if (out)
      strcpy(out, "[]");
    return out;
  }

  if (p)
  {
    /* Compose the output array. */
    i = p->offset;
    ptr = ensure(p, 1);
    if (!ptr)
      return 0;
    *ptr = '[';
    p->offset++;
    child = item->child;
    while (child && !fail)
    {
      print_value(child, depth + 1, fmt, p);
      p->offset = update(p);
      if (child->next)
      {
        len = fmt ? 2 : 1;
        ptr = ensure(p, len + 1);
        if (!ptr)
          return 0;
        *ptr++ = ',';
        if (fmt)
          *ptr++ = ' ';
        *ptr = 0;
        p->offset += len;
      }
      child = child->next;
    }
    ptr = ensure(p, 2);
    if (!ptr)
      return 0;
    *ptr++ = ']';
    *ptr = 0;
    out = (p->buffer) + i;
  } else
  {
    /* Allocate an array to hold the values for each */
    entries = (char**) cJSON_malloc(numentries * sizeof(char*));
    if (!entries)
      return 0;
    memset(entries, 0, numentries * sizeof(char*));
    /* Retrieve all the results: */
    child = item->child;
    while (child && !fail)
    {
      ret = print_value(child, depth + 1, fmt, 0);
      entries[i++] = ret;
      if (ret)
        len += strlen(ret) + 2 + (fmt ? 1 : 0);
      else
        fail = 1;
      child = child->next;
    }

    /* If we didn't fail, try to malloc the output string */
    if (!fail)
      out = (char*) cJSON_malloc(len);
    /* If that fails, we fail. */
    if (!out)
      fail = 1;

    /* Handle failure. */
    if (fail)
    {
      for (i = 0; i < numentries; i++)
        if (entries[i])
          cJSON_free(entries[i]);
      cJSON_free(entries);
      return 0;
    }

    /* Compose the output array. */
    *out = '[';
    ptr = out + 1;
    *ptr = 0;
    for (i = 0; i < numentries; i++)
    {
      tmplen = strlen(entries[i]);
      memcpy(ptr, entries[i], tmplen);
      ptr += tmplen;
      if (i != numentries - 1)
      {
        *ptr++ = ',';
        if (fmt)
          *ptr++ = ' ';
        *ptr = 0;
      }
      cJSON_free(entries[i]);
    }
    cJSON_free(entries);
    *ptr++ = ']';
    *ptr++ = 0;
  }
  return out;
}

/* Build an object from the text. */
static const char *parse_object(AZX_CJSON_T *item, const char *value,
    const char **ep)
{
  AZX_CJSON_T *child;
  if (*value != '{')
  {
    *ep = value;
    return 0;
  } /* not an object! */

  item->type = AZX_CJSON_OBJECT;
  value = skip(value + 1);
  if (*value == '}')
    return value + 1; /* empty array. */

  item->child = child = cJSON_New_Item();
  if (!item->child)
    return 0;
  value = skip(parse_string(child, skip(value), ep));
  if (!value)
    return 0;
  child->string = child->valuestring;
  child->valuestring = 0;
  if (*value != ':')
  {
    *ep = value;
    return 0;
  } /* fail! */
  value = skip(parse_value(child, skip(value + 1), ep)); /* skip any spacing, get the value. */
  if (!value)
    return 0;

  while (*value == ',')
  {
    AZX_CJSON_T *new_item;
    if (!(new_item = cJSON_New_Item()))
      return 0; /* memory fail */
    child->next = new_item;
    new_item->prev = child;
    child = new_item;
    value = skip(parse_string(child, skip(value + 1), ep));
    if (!value)
      return 0;
    child->string = child->valuestring;
    child->valuestring = 0;
    if (*value != ':')
    {
      *ep = value;
      return 0;
    } /* fail! */
    value = skip(parse_value(child, skip(value + 1), ep)); /* skip any spacing, get the value. */
    if (!value)
      return 0;
  }

  if (*value == '}')
    return value + 1; /* end of array */
  *ep = value;
  return 0; /* malformed. */
}

/* Render an object to text. */
static char *print_object(AZX_CJSON_T *item, int depth, int fmt, printbuffer *p)
{
  char **entries = 0, **names = 0;
  char *out = 0, *ptr, *ret, *str;
  int len = 7, i = 0, j;
  AZX_CJSON_T *child = item->child;
  int numentries = 0, fail = 0;
  size_t tmplen = 0;
  /* Count the number of entries. */
  while (child)
    numentries++, child = child->next;
  /* Explicitly handle empty object case */
  if (!numentries)
  {
    if (p)
      out = ensure(p, fmt ? depth + 4 : 3);
    else
      out = (char*) cJSON_malloc(fmt ? depth + 4 : 3);
    if (!out)
      return 0;
    ptr = out;
    *ptr++ = '{';
    if (fmt)
    {
      *ptr++ = '\n';
      for (i = 0; i < depth; i++)
        *ptr++ = '\t';
    }
    *ptr++ = '}';
    *ptr++ = 0;
    return out;
  }
  if (p)
  {
    /* Compose the output: */
    i = p->offset;
    len = fmt ? 2 : 1;
    ptr = ensure(p, len + 1);
    if (!ptr)
      return 0;
    *ptr++ = '{';
    if (fmt)
      *ptr++ = '\n';
    *ptr = 0;
    p->offset += len;
    child = item->child;
    depth++;
    while (child)
    {
      if (fmt)
      {
        ptr = ensure(p, depth);
        if (!ptr)
          return 0;
        for (j = 0; j < depth; j++)
          *ptr++ = '\t';
        p->offset += depth;
      }
      print_string_ptr(child->string, p);
      p->offset = update(p);

      len = fmt ? 2 : 1;
      ptr = ensure(p, len);
      if (!ptr)
        return 0;
      *ptr++ = ':';
      if (fmt)
        *ptr++ = '\t';
      p->offset += len;

      print_value(child, depth, fmt, p);
      p->offset = update(p);

      len = (fmt ? 1 : 0) + (child->next ? 1 : 0);
      ptr = ensure(p, len + 1);
      if (!ptr)
        return 0;
      if (child->next)
        *ptr++ = ',';
      if (fmt)
        *ptr++ = '\n';
      *ptr = 0;
      p->offset += len;
      child = child->next;
    }
    ptr = ensure(p, fmt ? (depth + 1) : 2);
    if (!ptr)
      return 0;
    if (fmt)
      for (i = 0; i < depth - 1; i++)
        *ptr++ = '\t';
    *ptr++ = '}';
    *ptr = 0;
    out = (p->buffer) + i;
  } else
  {
    /* Allocate space for the names and the objects */
    entries = (char**) cJSON_malloc(numentries * sizeof(char*));
    if (!entries)
      return 0;
    names = (char**) cJSON_malloc(numentries * sizeof(char*));
    if (!names)
    {
      cJSON_free(entries);
      return 0;
    }
    memset(entries, 0, sizeof(char*) * numentries);
    memset(names, 0, sizeof(char*) * numentries);

    /* Collect all the results into our arrays: */
    child = item->child;
    depth++;
    if (fmt)
      len += depth;
    while (child && !fail)
    {
      names[i] = str = print_string_ptr(child->string, 0);
      entries[i++] = ret = print_value(child, depth, fmt, 0);
      if (str && ret)
        len += strlen(ret) + strlen(str) + 2 + (fmt ? 2 + depth : 0);
      else
        fail = 1;
      child = child->next;
    }

    /* Try to allocate the output string */
    if (!fail)
      out = (char*) cJSON_malloc(len);
    if (!out)
      fail = 1;

    /* Handle failure */
    if (fail)
    {
      for (i = 0; i < numentries; i++)
      {
        if (names[i])
          cJSON_free(names[i]);
        if (entries[i])
          cJSON_free(entries[i]);
      }
      cJSON_free(names);
      cJSON_free(entries);
      return 0;
    }

    /* Compose the output: */
    *out = '{';
    ptr = out + 1;
    if (fmt)
      *ptr++ = '\n';
    *ptr = 0;
    for (i = 0; i < numentries; i++)
    {
      if (fmt)
        for (j = 0; j < depth; j++)
          *ptr++ = '\t';
      tmplen = strlen(names[i]);
      memcpy(ptr, names[i], tmplen);
      ptr += tmplen;
      *ptr++ = ':';
      if (fmt)
        *ptr++ = '\t';
      strcpy(ptr, entries[i]);
      ptr += strlen(entries[i]);
      if (i != numentries - 1)
        *ptr++ = ',';
      if (fmt)
        *ptr++ = '\n';
      *ptr = 0;
      cJSON_free(names[i]);
      cJSON_free(entries[i]);
    }

    cJSON_free(names);
    cJSON_free(entries);
    if (fmt)
      for (i = 0; i < depth - 1; i++)
        *ptr++ = '\t';
    *ptr++ = '}';
    *ptr++ = 0;
  }
  return out;
}

/* Get Array size/item / object item. */
int azx_cjson_getArraySize(AZX_CJSON_T *array)
{
  AZX_CJSON_T *c = array->child;
  int i = 0;
  while (c)
    i++, c = c->next;
  return i;
}
AZX_CJSON_T *azx_cjson_getArrayItem(AZX_CJSON_T *array, int item)
{
  AZX_CJSON_T *c = array ? array->child : 0;
  while (c && item > 0)
    item--, c = c->next;
  return c;
}
AZX_CJSON_T *azx_cjson_getObjectItem(AZX_CJSON_T *object, const char *string)
{
  AZX_CJSON_T *c = object ? object->child : 0;
  while (c && cJSON_strcasecmp(c->string, string))
    c = c->next;
  return c;
}
int azx_cjson_hasObjectItem(AZX_CJSON_T *object, const char *string)
{
  return azx_cjson_getObjectItem(object, string) ? 1 : 0;
}

/* Utility for array list handling. */
static void suffix_object(AZX_CJSON_T *prev, AZX_CJSON_T *item)
{
  prev->next = item;
  item->prev = prev;
}
/* Utility for handling references. */
static AZX_CJSON_T *create_reference(AZX_CJSON_T *item)
{
  AZX_CJSON_T *ref = cJSON_New_Item();
  if (!ref)
    return 0;
  memcpy(ref, item, sizeof(AZX_CJSON_T));
  ref->string = 0;
  ref->type |= AZX_CJSON_IS_REFERENCE;
  ref->next = ref->prev = 0;
  return ref;
}

/* Add item to array/object. */
void azx_cjson_addItemToArray(AZX_CJSON_T *array, AZX_CJSON_T *item)
{
  AZX_CJSON_T *c = array->child;
  if (!item)
    return;
  if (!c)
  {
    array->child = item;
  } else
  {
    while (c && c->next)
      c = c->next;
    suffix_object(c, item);
  }
}
void azx_cjson_addItemToObject(AZX_CJSON_T *object, const char *string,
    AZX_CJSON_T *item)
{
  if (!item)
    return;
  if (item->string)
    cJSON_free(item->string);
  item->string = cJSON_strdup(string);
  azx_cjson_addItemToArray(object, item);
}
void azx_cjson_addItemToObjectCS(AZX_CJSON_T *object, const char *string,
    AZX_CJSON_T *item)
{
  if (!item)
    return;
  if (!(item->type & AZX_CJSON_STRING_IS_CONST) && item->string)
    cJSON_free(item->string);
  item->string = (char*) string;
  item->type |= AZX_CJSON_STRING_IS_CONST;
  azx_cjson_addItemToArray(object, item);
}
void azx_cjson_addItemReferenceToArray(AZX_CJSON_T *array, AZX_CJSON_T *item)
{
  azx_cjson_addItemToArray(array, create_reference(item));
}
void azx_cjson_addItemReferenceToObject(AZX_CJSON_T *object, const char *string, AZX_CJSON_T *item)
{
  azx_cjson_addItemToObject(object, string, create_reference(item));
}

AZX_CJSON_T *azx_cjson_detachItemFromArray(AZX_CJSON_T *array, int which)
{
  AZX_CJSON_T *c = array->child;
  while (c && which > 0)
    c = c->next, which--;
  if (!c)
    return 0;
  if (c->prev)
    c->prev->next = c->next;
  if (c->next)
    c->next->prev = c->prev;
  if (c == array->child)
    array->child = c->next;
  c->prev = c->next = 0;
  return c;
}
void azx_cjson_deleteItemFromArray(AZX_CJSON_T *array, int which)
{
  azx_cjson_delete(azx_cjson_detachItemFromArray(array, which));
}
AZX_CJSON_T *azx_cjson_detachItemFromObject(AZX_CJSON_T *object, const char *string)
{
  int i = 0;
  AZX_CJSON_T *c = object->child;
  while (c && cJSON_strcasecmp(c->string, string))
    i++, c = c->next;
  if (c)
    return azx_cjson_detachItemFromArray(object, i);
  return 0;
}
void azx_cjson_deleteItemFromObject(AZX_CJSON_T *object, const char *string)
{
  azx_cjson_delete(azx_cjson_detachItemFromObject(object, string));
}

/* Replace array/object items with new ones. */
void azx_cjson_insertItemInArray(AZX_CJSON_T *array, int which,
    AZX_CJSON_T *newitem)
{
  AZX_CJSON_T *c = array->child;
  while (c && which > 0)
    c = c->next, which--;
  if (!c)
  {
    azx_cjson_addItemToArray(array, newitem);
    return;
  }
  newitem->next = c;
  newitem->prev = c->prev;
  c->prev = newitem;
  if (c == array->child)
    array->child = newitem;
  else
    newitem->prev->next = newitem;
}
void azx_cjson_replaceItemInArray(AZX_CJSON_T *array, int which,
    AZX_CJSON_T *newitem)
{
  AZX_CJSON_T *c = array->child;
  while (c && which > 0)
    c = c->next, which--;
  if (!c)
    return;
  newitem->next = c->next;
  newitem->prev = c->prev;
  if (newitem->next)
    newitem->next->prev = newitem;
  if (c == array->child)
    array->child = newitem;
  else
    newitem->prev->next = newitem;
  c->next = c->prev = 0;
  azx_cjson_delete(c);
}
void azx_cjson_replaceItemInObject(AZX_CJSON_T *object, const char *string,
    AZX_CJSON_T *newitem)
{
  int i = 0;
  AZX_CJSON_T *c = object->child;
  while (c && cJSON_strcasecmp(c->string, string))
    i++, c = c->next;
  if (c)
  {
    newitem->string = cJSON_strdup(string);
    azx_cjson_replaceItemInArray(object, i, newitem);
  }
}

/* Create basic types: */
AZX_CJSON_T *azx_cjson_createNull(void)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = AZX_CJSON_NULL;
  return item;
}
AZX_CJSON_T *azx_cjson_createTrue(void)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = AZX_CJSON_TRUE;
  return item;
}
AZX_CJSON_T *azx_cjson_createFalse(void)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = AZX_CJSON_FALSE;
  return item;
}
AZX_CJSON_T *azx_cjson_createBool(int b)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = b ? AZX_CJSON_TRUE : AZX_CJSON_FALSE;
  return item;
}
AZX_CJSON_T *azx_cjson_createNumber(double num)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
  {
    item->type = AZX_CJSON_NUMBER;
    item->valuedouble = num;
    item->valueint = (int) num;
  }
  return item;
}
AZX_CJSON_T *azx_cjson_createString(const char *string)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
  {
    item->type = AZX_CJSON_STRING;
    item->valuestring = cJSON_strdup(string);
    if (!item->valuestring)
    {
      azx_cjson_delete(item);
      return 0;
    }
  }
  return item;
}
AZX_CJSON_T *azx_cjson_createArray(void)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = AZX_CJSON_ARRAY;
  return item;
}
AZX_CJSON_T *azx_cjson_createObject(void)
{
  AZX_CJSON_T *item = cJSON_New_Item();
  if (item)
    item->type = AZX_CJSON_OBJECT;
  return item;
}

/* Create Arrays: */
AZX_CJSON_T *azx_cjson_createIntArray(const int *numbers, int count)
{
  int i;
  AZX_CJSON_T *n = 0, *p = 0, *a = azx_cjson_createArray();
  for (i = 0; a && i < count; i++)
  {
    n = azx_cjson_createNumber(numbers[i]);
    if (!n)
    {
      azx_cjson_delete(a);
      return 0;
    }
    if (!i)
      a->child = n;
    else
      suffix_object(p, n);
    p = n;
  }
  return a;
}
AZX_CJSON_T *azx_cjson_createFloatArray(const float *numbers, int count)
{
  int i;
  AZX_CJSON_T *n = 0, *p = 0, *a = azx_cjson_createArray();
  for (i = 0; a && i < count; i++)
  {
    n = azx_cjson_createNumber(numbers[i]);
    if (!n)
    {
      azx_cjson_delete(a);
      return 0;
    }
    if (!i)
      a->child = n;
    else
      suffix_object(p, n);
    p = n;
  }
  return a;
}
AZX_CJSON_T *azx_cjson_createDoubleArray(const double *numbers, int count)
{
  int i;
  AZX_CJSON_T *n = 0, *p = 0, *a = azx_cjson_createArray();
  for (i = 0; a && i < count; i++)
  {
    n = azx_cjson_createNumber(numbers[i]);
    if (!n)
    {
      azx_cjson_delete(a);
      return 0;
    }
    if (!i)
      a->child = n;
    else
      suffix_object(p, n);
    p = n;
  }
  return a;
}
AZX_CJSON_T *azx_cjson_createStringArray(const char **strings, int count)
{
  int i;
  AZX_CJSON_T *n = 0, *p = 0, *a = azx_cjson_createArray();
  for (i = 0; a && i < count; i++)
  {
    n = azx_cjson_createString(strings[i]);
    if (!n)
    {
      azx_cjson_delete(a);
      return 0;
    }
    if (!i)
      a->child = n;
    else
      suffix_object(p, n);
    p = n;
  }
  return a;
}

/* Duplication */
AZX_CJSON_T *azx_cjson_duplicate(AZX_CJSON_T *item, int recurse)
{
  AZX_CJSON_T *newitem, *cptr, *nptr = 0, *newchild;
  /* Bail on bad ptr */
  if (!item)
    return 0;
  /* Create new item */
  newitem = cJSON_New_Item();
  if (!newitem)
    return 0;
  /* Copy over all vars */
  newitem->type = item->type & (~AZX_CJSON_IS_REFERENCE), newitem->valueint =
      item->valueint, newitem->valuedouble = item->valuedouble;
  if (item->valuestring)
  {
    newitem->valuestring = cJSON_strdup(item->valuestring);
    if (!newitem->valuestring)
    {
      azx_cjson_delete(newitem);
      return 0;
    }
  }
  if (item->string)
  {
    newitem->string = cJSON_strdup(item->string);
    if (!newitem->string)
    {
      azx_cjson_delete(newitem);
      return 0;
    }
  }
  /* If non-recursive, then we're done! */
  if (!recurse)
    return newitem;
  /* Walk the ->next chain for the child. */
  cptr = item->child;
  while (cptr)
  {
    newchild = azx_cjson_duplicate(cptr, 1); /* Duplicate (with recurse) each item in the ->next chain */
    if (!newchild)
    {
      azx_cjson_delete(newitem);
      return 0;
    }
    if (nptr)
    {
      nptr->next = newchild, newchild->prev = nptr;
      nptr = newchild;
    } /* If newitem->child already set, then crosswire ->prev and ->next and move on */
    else
    {
      newitem->child = newchild;
      nptr = newchild;
    } /* Set newitem->child and move to it */
    cptr = cptr->next;
  }
  return newitem;
}

void azx_cjson_minify(char *json)
{
  char *into = json;
  while (*json)
  {
    if (*json == ' ')
      json++;
    else if (*json == '\t')
      json++; /* Whitespace characters. */
    else if (*json == '\r')
      json++;
    else if (*json == '\n')
      json++;
    else if (*json == '/' && json[1] == '/')
      while (*json && *json != '\n')
        json++; /* double-slash comments, to end of line. */
    else if (*json == '/' && json[1] == '*')
    {
      while (*json && !(*json == '*' && json[1] == '/'))
        json++;
      json += 2;
    } /* multiline comments. */
    else if (*json == '\"')
    {
      *into++ = *json++;
      while (*json && *json != '\"')
      {
        if (*json == '\\')
          *into++ = *json++;
        *into++ = *json++;
      }
      *into++ = *json++;
    } /* string literals, which are \" sensitive. */
    else
      *into++ = *json++; /* All other characters. */
  }
  *into = 0; /* and null-terminate. */
}
