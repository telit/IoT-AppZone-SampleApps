/*Copyright (C) 2020 Telit Communications S.p.A. Italy - All Rights Reserved.*/
/*    See LICENSE file in the project root for full license information.     */

/*===========================================================================*/
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
     azx_cjson.h

 @brief
     JSON objects manipulation

 @details
     Porting on m2mb of the AZX_CJSON_T library. \n
     This library code will give user the possibility to use json string format on their application

@version
     1.0.0

 @note
 Dependencies:
      \n#include "string.h"
      \n#include "m2mb_types.h"
      \n#include "azx_cjson.h"

 @author Fabio Pintus
 @author Argiolas Norman

 @date
     04/02/2020
 */

#ifndef HDR_AZX_CJSON__h
#define HDR_AZX_CJSON__h

#ifdef __cplusplus
extern "C"
{
#endif

/* Local defines ================================================================================*/

/**\name AZX_CJSON_T Types
 * \brief These macros can be used to trim strings removing quotes
 *    @{
 **/
#define AZX_CJSON_FALSE  (1 << 0)     /**<Boolean: False*/
#define AZX_CJSON_TRUE   (1 << 1)     /**<Boolean: True*/
#define AZX_CJSON_NULL   (1 << 2)     /**<Null type*/
#define AZX_CJSON_NUMBER (1 << 3)     /**<Number type*/
#define AZX_CJSON_STRING (1 << 4)     /**<String type*/
#define AZX_CJSON_ARRAY  (1 << 5)     /**<Array type*/
#define AZX_CJSON_OBJECT (1 << 6)     /**<Object type*/
/**   @} */
/**   @} */  //close addtogroup

/*!   \cond PRIVATE */
#define AZX_CJSON_IS_REFERENCE    256     /**<@hideinitializer*/
#define AZX_CJSON_STRING_IS_CONST 512     /**<@hideinitializer*/
/*!   \endcond */
/* =========================================================================== */


/* Global MACROS ==============================================================*/

/* Macros for creating things quickly. */
/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a null item to a AZX_CJSON_T object

 @details
 This macro will add a null item to a AZX_CJSON_T object

 @param[in] object
 destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addNullToObject(myobj, "mynull");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addNullToObject(object,name)      azx_cjson_addItemToObject(object, name, azx_cjson_createNull())

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a boolean True item to a AZX_CJSON_T object

 @details
 This macro will add a boolean True item to a AZX_CJSON_T object

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addTrueToObject(myobj, "mybool");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addTrueToObject(object,name)      azx_cjson_addItemToObject(object, name, azx_cjson_createTrue())

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a boolean False item to a AZX_CJSON_T object

 @details
 This macro will add a boolean False item to a AZX_CJSON_T object

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addFalseToObject(myobj, "mybool");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addFalseToObject(object,name)     azx_cjson_addItemToObject(object, name, azx_cjson_createFalse())

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a boolean item to a AZX_CJSON_T object

 @details
 This macro will add a boolean  item to a AZX_CJSON_T object

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.
 @param[in] b
 The boolean value to be applied

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addBoolToObject(myobj, "mybool", 1);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addBoolToObject(object,name,b)    azx_cjson_addItemToObject(object, name, azx_cjson_createBool(b))

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a number item to a AZX_CJSON_T object

 @details
 This macro will add a number item to a AZX_CJSON_T object

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.
 @param[in] n
 The number value

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete();

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addNumberToObject(myobj, "mynumber", 4);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addNumberToObject(object,name,n)  azx_cjson_addItemToObject(object, name, azx_cjson_createNumber(n))

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro to add a string item to a AZX_CJSON_T object

 @details
 This macro will add a string item to a AZX_CJSON_T object (the string will be duplicated)

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] name
 The label that will be used for the new item.
 @param[in] s
 The string

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete();

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 azx_cjson_addStringToObject(myobj, "mystring", "hello world!");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_addStringToObject(object,name,s)  azx_cjson_addItemToObject(object, name, azx_cjson_createString(s))

#define azx_cjson_setIntValue(object,val)     ((object)?(object)->valueint=(object)->valuedouble=(val):(val))
#define azx_cjson_setNumberValue(object,val)  ((object)?(object)->valueint=(object)->valuedouble=(val):(val))

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Macro for iterating over an array

 @details
 This macro will iterate the elements of an array

 @param[out] pos
 element. it is a AZX_CJSON_T * element that is returned by the macro and can be used for further processing
 @param[in] head
 AZX_CJSON_T Array pointer

 @return
 None

 <b>Refer to</b>
 azx_cjson_delete();

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"list\": [\"elem1\", \"elem2\"]}");
 AZX_CJSON_T *list = azx_cjson_gGetObjectItem(root,"list");
 AZX_CJSON_T *elem = NULL;
 azx_cjson_arrayForEach(elem,list)
 {
 PRINT("value: %s\r\n", elem->valuestring);
 }
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
#define azx_cjson_arrayForEach(pos, head)     for(pos = (head)->child; pos != NULL; pos = pos->next)

/* =========================================================================== */


/* Global typedefs ===========================================================*/
/**
 *  @brief A structure to allow you walk through array/object json chains.
 *  @details This structure holds the parameters required to manage a AZX_CJSON_T object
 **/
typedef struct AZX_CJSON_TAG {
  struct AZX_CJSON_TAG *next;  /**<Next/prev allow you to walk array/object chains. Alternatively, use azx_cjson_getArraySize/azx_cjson_getArrayItem/azx_cjson_getObjectItem */
  struct AZX_CJSON_TAG *prev;  /**<Prev allow you to walk array/object chains. Alternatively, use azx_cjson_getArraySize/azx_cjson_getArrayItem/azx_cjson_getObjectItem */
  struct AZX_CJSON_TAG *child; /**<An array or object item will have a child pointer pointing to a chain of the items in the array/object. */

  int type;                   /**<The type of the item, as above. */

  char *valuestring;          /**<The item's string, if type==AZX_CJSON_STRING */
  int valueint;               /**<The item's number, if type==AZX_CJSON_NUMBER */
  double valuedouble;         /**<The item's number, if type==AZX_CJSON_NUMBER */

  char *string;               /**<The item's name string, if this item is the child of, or is in the list of subitems of an object. */
} AZX_CJSON_T;                /**<Typedef of struct AZX_CJSON_T*/

/* =========================================================================== */


/* Global functions ==========================================================*/

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Parses the provided JSON string into a JSON object, using default options.

 @details
 This function returns a AZX_CJSON_T object that can be interrogated. Call azx_cjson_delete to release the AZX_CJSON_T
 object resource.

 @param[in] value
 string buffer containing the AZX_CJSON_T block

 @return
 a AZX_CJSON_T object pointer in case of success
 @return
 NULL in case of failure

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"name\": \"myname\"}");
 @endcode
 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T* azx_cjson_parse(const char *value);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Convert a AZX_CJSON_T object to formatted text

 @details
 This function will render a AZX_CJSON_T entity to text for transfer/storage, formatting it with tabs/ new lines.
 Free the char* when finished.

 @param[in] item
 AZX_CJSON_T object

 @return
 A string containing the formatted content of the AZX_CJSON_T object

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"name\": \"myname\"}");
 char *text = azx_cjson_print(root);
 @endcode
 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
char *azx_cjson_print(AZX_CJSON_T *item);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Convert a AZX_CJSON_T object to unformatted text

 @details
 This function will render a AZX_CJSON_T entity to text for transfer/storage without any formatting.
 Free the char* when finished.

 @param[in] item
 AZX_CJSON_T object

 @return
 A string containing the unformatted content of the AZX_CJSON_T object

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_pelete()

 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"name\": \"myname\"}");
 char *text = azx_cjson_printUnformatted(root);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
char *azx_cjson_printUnformatted(AZX_CJSON_T *item);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Convert a AZX_CJSON_T object to text using a buffered strategy.

 @details
 This function will render a AZX_CJSON_T entity to text using a buffered strategy. Prebuffer is a
 guess at the final size. Guessing well reduces reallocation. fmt=0 gives unformatted, =1 gives formatted.
 Free the char* when finished.

 @param[in] item
 AZX_CJSON_T object
 @param[in] prebuffer
 Guess of the final buffer size.
 @param[in] fmt
 If 1, consider formatted  text. if 0, consider unformatted.


 @return
 A string containing the unformatted content of the AZX_CJSON_T object

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"name\": \"myname\"}");
 char *text = azx_cjson_printBuffered(root, 30, 1);
 @endcode
 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
char *azx_cjson_printBuffered(AZX_CJSON_T *item, int prebuffer, int fmt);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Delete a AZX_CJSON_T entity and all subentities.

 @details
 This function will destroy a AZX_CJSON_T entity, and all the subentities that compose it

 @param[in] c
 AZX_CJSON_T object

 @return
 None

 <b>Refer to</b>
 azx_cjson_parse()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"name\": \"myname\"}");
 azx_cjson_delete(root);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_delete(AZX_CJSON_T *c);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Returns the number of items in an array (or object)

 @details
 This function will return the number of items in an array (or object)

 @param[in] array
 AZX_CJSON_T object of array type

 @return
 The number of elements in the array

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_getObjectItem()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"list\": [\"elem1\", \"elem2\"]}");
 AZX_CJSON_T *list = azx_cjson_getObjectItem(root,"list");
 int list_elems;
 if (list && list->type == azx_cjson_array)
 {
 list_elems = azx_cjson_getArraySize (list);
 }
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
int azx_cjson_getArraySize(AZX_CJSON_T *array);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Retrieves item number "item" from array "array".

 @details
 This function will return item number "item" from array "array". Returns NULL if unsuccessful.

 @param[in] array
 AZX_CJSON_T object of array type
 @param[in] item
 The index number for the element to be retrieved from array (starts from 0)

 @return
 The array item as AZX_CJSON_T object

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_getObjectItem()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"list\": [\"elem1\", \"elem2\"]}");
 AZX_CJSON_T *list = azx_cjson_getObjectItem(root,"list");
 AZX_CJSON_T *elem1 = NULL;
 if (list && list->type == azx_cjson_array)
 {
 elem1 = azx_cjson_getArrayItem(list, 0);
 if (elem1)
 {
 PRINT( "first element is \"%s\"", elem1->valuestring);
 }
 }
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_getArrayItem(AZX_CJSON_T *array, int item);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Retrieves item "string" from object. Case insensitive.

 @details
 This function will retrieve the item "string" from object. Case insensitive.

 @param[in] object
 AZX_CJSON_T object
 @param[in] string
 The string with the name of the item to be retrieved.

 @return
 The item as AZX_CJSON_T object

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_getObjectItem()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"myobj\": \"myvalue\"}");
 AZX_CJSON_T *obj = azx_cjson_getObjectItem(root,"myobj");
 if (obj)
 {
 PRINT( "myobj is \"%s\"", obj->valuestring);
 }
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_getObjectItem(AZX_CJSON_T *object, const char *string);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Checks if the object contains the item "string"

 @details
 This function will verify if the item "string" is present in object. Case insensitive.

 @param[in] object
 AZX_CJSON_T object
 @param[in] string
 The string with the name of the item to be verified

 @return
 1 if the item is present, 0 otherwise.

 <b>Refer to</b>
 azx_cjson_parse()
 azx_cjson_getObjectItem()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"myobj\": \"myvalue\"}");
 int is_present = azx_cjson_hasObjectItem(root, "myobj");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
int azx_cjson_hasObjectItem(AZX_CJSON_T *object, const char *string);


/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 For analysing failed parses. This returns a pointer to the parse error point.

 @details
 This function will return the point in the string where the parse failed.
 It will be probably needed to look a few chars back to make sense of it.

 @return
 The address of the point in the input string where the fails occur when azx_cjson_parse() returns 0
 @return
 0 when azx_cjson_parse() succeeds.

 <b>Refer to</b>
 azx_cjson_parse()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parse("{ \"myobj\" \"myvalue\"}"); //malformed string
 char *error = azx_cjson_getErrorPtr();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
const char *azx_cjson_getErrorPtr(void);

/* These calls create a AZX_CJSON_T item of the appropriate type. */

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a Null object

 @details
 This function will create a Null AZX_CJSON_T object

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createNull();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createNull(void);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a boolean object with True value

 @details
 This function will create a AZX_CJSON_T object with a boolean True value

 @return
 the AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createTrue();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createTrue(void);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a boolean object with False value

 @details
 This function will create a AZX_CJSON_T object with a boolean False value

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createFalse();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createFalse(void);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a boolean object with provided value

 @details
 This function will create a AZX_CJSON_T object with a boolean value

 @param[in] b
 The boolean value to be set

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createBool(1);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createBool(int b);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a numeric object with provided value

 @details
 This function will create a AZX_CJSON_T object with a numeric value

 @param[in] num
 The numeric value to be set, as a double

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_reateNum(34.5);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createNumber(double num);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create a string object with provided string

 @details
 This function will create a AZX_CJSON_T object with a string value. The string will be duplicated,
 not referenced.

 @param[in] string
 The string to be set

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise


 <b>Refer to</b>
 azx_cjson_dDelete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createString("hello");
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createString(const char *string);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an Array object

 @details
 This function will create a AZX_CJSON_T Array object.

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myArray = azx_cjson_createArray();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createArray(void);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an empty object

 @details
 This function will create a AZX_CJSON_T object, that can be used to later add elements to it.

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createObject(void);

/* These utilities create an Array of count items. */

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an Array object of count integers

 @details
 This function will create a AZX_CJSON_T Array object of count integers, that will be filled with
 the values provided with the "numbers" array.

 @param[in] numbers
 array of integer numbers to be used
 @param[in] count
 the length of the array

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 int numbers[] = {4,5,6};
 AZX_CJSON_T * myArray = azx_cjson_createIntArray(numbers, 3);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createIntArray(const int *numbers, int count);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an Array object of count floats

 @details
 This function will create a AZX_CJSON_T Array object of count floats, that will be filled with
 the values provided with the "numbers" array.

 @param[in] numbers
 Array of float numbers to be used

 @param[in] count
 The length of the array

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 float numbers[] = {4.1,5.2,6.3};
 AZX_CJSON_T * myArray = azx_cjson_createFloatArray(numbers, 3);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createFloatArray(const float *numbers, int count);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an Array object of count doubles

 @details
 This function will create a AZX_CJSON_T Array object of count doubles, that will be filled with
 the values provided with the "numbers" array.

 @param[in] numbers
 Array of double numbers to be used
 @param[in] count
 The length of the array

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 double numbers[] = {4.111111,5.222222,6.333333};
 AZX_CJSON_T * myArray = azx_cjson_createDoubleArray(numbers, 3);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createDoubleArray(const double *numbers, int count);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Create an Array object of count strings

 @details
 This function will create a AZX_CJSON_T Array object of count strings, that will be filled with
 the values provided with the "strings" array.

 @param[in] strings
 Array of strings to be used. Strings are copied, not referenced.
 @param[in] count
 The length of the array

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 double strings[] = {"hello", "world"};
 AZX_CJSON_T * myArray = azx_cjson_createStringArray(strings, 2);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_createStringArray(const char **strings, int count);

/* Append item to the specified array/object. */

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Add a AZX_CJSON_T item to an Array object

 @details
 This function will add a generic AZX_CJSON_T item to an Array object

 @param[in] array
 Destination AZX_CJSON_T array
 @param[in] item
 AZX_CJSON_T item to be added

 @return
 None

 <b>Refer to</b>
 azx_cjson_createArray()
 azx_cjson_deleteItemFromArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myArray = azx_cjson_createArray();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToArray(myArray, myNum);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_addItemToArray(AZX_CJSON_T *array, AZX_CJSON_T *item);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Add a AZX_CJSON_T item to a AZX_CJSON_T object

 @details
 This function will add a generic AZX_CJSON_T item to another AZX_CJSON_T object, using the provided label

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] string
 The label that will be used for "item". If "item" already has one, it will be replaced.
 @param[in] item
 AZX_CJSON_T item to be added

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_deleteItemFromObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_addItemToObject(AZX_CJSON_T *object, const char *string, AZX_CJSON_T *item);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Add a AZX_CJSON_T item to a AZX_CJSON_T object, passing a const char string as label

 @details
 This function will add a generic AZX_CJSON_T item to another AZX_CJSON_T object, using the provided label, which is
 definitely const (i.e. a literal, or as good as). This will assure that the string will survive the AZX_CJSON_T object.

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] string
 The label that will be used for "item"
 @param[in] item
 AZX_CJSON_T item to be added

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_addItemToObjectCS(AZX_CJSON_T *object, const char *string, AZX_CJSON_T *item);

/* Append reference to item to the specified array/object. Use this when you want to add an existing AZX_CJSON_T to a new cJSON, but don't want to corrupt your existing cJSON. */
/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Append a AZX_CJSON_T item to a AZX_CJSON_T array as a reference

 @details
 This function will append a generic AZX_CJSON_T item to a AZX_CJSON_T array. This function should be used when it is
 required to add an existing AZX_CJSON_T to a new cJSON, but without the risk of corrupting the existing one.

 @param[in] array
 Destination AZX_CJSON_T array
 @param[in] item
 AZX_CJSON_T item to be added as a reference

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_deleteItemFromArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myArray = azx_cjson_createArray();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemReferenceToArray(myArray, myNum);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_addItemReferenceToArray(AZX_CJSON_T *array, AZX_CJSON_T *item);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Append a AZX_CJSON_T item to a AZX_CJSON_T object as a reference

 @details
 This function will append a generic AZX_CJSON_T item to a AZX_CJSON_T object. This function should be used when it is
 required to add an existing AZX_CJSON_T to a new cJSON, but without the risk of corrupting the existing one.

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] string
 The label that will be used for "item". If "item" already has one, it will be replaced.
 @param[in] item
 AZX_CJSON_T item to be added as a reference

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_deleteItemFromArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemReferenceToObject(myArray, "mynumber", myNum);
 @endcode

 @ingroup cjsonUsage
 */
/*---------------AZX_CJSON_T --------------------------------------------------------------------------*/
void azx_cjson_addItemReferenceToObject(AZX_CJSON_T *object, const char *string, AZX_CJSON_T *item);

/* Remove/Detatch items from Arrays/Objects. */

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Detach a AZX_CJSON_T item from a AZX_CJSON_T array

 @details
 This function will detach a AZX_CJSON_T item from a AZX_CJSON_T array, and return the pointer to the child item
 Deleting the array will not affect the child at this point

 @param[in] array
 Source AZX_CJSON_T array
 @param[in] which
 The index of the element to be detached

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_createArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 int numbers[] = {4,5,6};
 AZX_CJSON_T * myArray = azx_cjson_createIntArray(numbers, 3);
 AZX_CJSON_T *mynum = azx_cjson_detachItemFromArray(myArray, 2); //mynum value will be 6 (the third element of the array)
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_detachItemFromArray(AZX_CJSON_T *array, int which);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Delete a AZX_CJSON_T item from a AZX_CJSON_T array

 @details
 This function will delete a AZX_CJSON_T item from a AZX_CJSON_T array

 @param[in] array
 Source AZX_CJSON_T array
 @param[in] which
 The index of the element to be deleted

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_createArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 int numbers[] = {4,5,6};
 AZX_CJSON_T * myArray = azx_cjson_createIntArray(numbers, 3);
 azx_cjson_deleteItemFromArray(myArray, 2); //myArray now does not contain 6 anymore
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_deleteItemFromArray(AZX_CJSON_T *array, int which);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Detach a AZX_CJSON_T item from a AZX_CJSON_T object

 @details
 This function will detach a AZX_CJSON_T item from a AZX_CJSON_T object, and return the pointer to the child item
 Deleting the object will not affect the child at this point

 @param[in] object
 Source AZX_CJSON_T object
 @param[in] string
 The string label of the element to be detached

 @return
 The AZX_CJSON_T object in case of success, NULL otherwise

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_addItemToObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 AZX_CJSON_T *num = azx_cjson_detachItemFromObject(myobj, "mynumber"); //num value will be 5
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_detachItemFromObject(AZX_CJSON_T *object, const char *string);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Delete a AZX_CJSON_T item from a AZX_CJSON_T object

 @details
 This function will delete a AZX_CJSON_T item from a AZX_CJSON_T object

 @param[in] object
 Source AZX_CJSON_T object
 @param[in] string
 The string label of the element to be deleted

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_addItemToObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 azx_cjson_deleteItemFromObject(myobj, "mynumber"); //now myobj does not contain "mynumber" anymore
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_deleteItemFromObject(AZX_CJSON_T *object, const char *string);

/* Update array items. */

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Add a AZX_CJSON_T item into an existing AZX_CJSON_T array

 @details
 This function will insert a AZX_CJSON_T item into a AZX_CJSON_T array at the provided index.
 Pre-existing items will be shifted to the right.

 @param[in] array
 Destination AZX_CJSON_T array
 @param[in] which
 the index where the element will be added
 @param[in] newitem
 AZX_CJSON_T item to be added

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_createArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 int numbers[] = {4,5,6};
 AZX_CJSON_T * myArray = azx_cjson_createIntArray(numbers, 3);
 azx_cjson_insertItemFromArray(myArray, 2, azx_cjson_createTrue()); //myArray now will be [4, 5, True, 6]
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_insertItemInArray(AZX_CJSON_T *array, int which, AZX_CJSON_T *newitem);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Replace a AZX_CJSON_T item into an existing AZX_CJSON_T array

 @details
 This function will replace the AZX_CJSON_T item at the provided index of a AZX_CJSON_T array.
 The original item at the same index will be replaced and deleted.

 @param[in] array
 Destination AZX_CJSON_T array
 @param[in] which
 the index where the element will be replaced
 @param[in] newitem
 AZX_CJSON_T item to be used as replacement

 @return
 None

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_createArray()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 int numbers[] = {4,5,6};
 AZX_CJSON_T * myArray = azx_cjson_createIntArray(numbers, 3);
 azx_cjson_replaceItemFromArray(myArray, 2, azx_cjson_createTrue()); //myArray now will be [4, 5, True]
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_replaceItemInArray(AZX_CJSON_T *array, int which, AZX_CJSON_T *newitem);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Replace a AZX_CJSON_T item into an existing AZX_CJSON_T object

 @details
 This function will replace the AZX_CJSON_T item with the provided label of a AZX_CJSON_T object.
 The original item will be replaced and deleted.

 @param[in] object
 Destination AZX_CJSON_T object
 @param[in] string
 The label that will be used for "item". If "item" already has one, it will be replaced.
 @param[in] newitem
 AZX_CJSON_T item to be used as replacement

 @return
 None
 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_addItemToObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 azx_cjson_replaceItemInObject(myobj, "mynumber", azx_cjson_createNumber(6)); //will result in {"mynumber" : 6}
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_replaceItemInObject(AZX_CJSON_T *object, const char *string, AZX_CJSON_T *newitem);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Duplicate a AZX_CJSON_T item.

 @details
 This function will duplicate a AZX_CJSON_T item. This will create in the memory pool a new AZX_CJSON_T item (identical to the passed one)
 that will need to be released.
 The item->next and ->prev pointers are always zero on return from Duplicate.

 @param[in] item
 Source AZX_CJSON_T object to be duplicated
 @param[in] recurse
 indicates if it is required to duplicate also any childred connected to the item.
 1: duplicate children
 0: do not duplicate children

 @return
 A AZX_CJSON_T object, copy of "item", in case of success. NULL otherwise.

 <b>Refer to</b>
 azx_cjson_createObject()
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T * myobj = azx_cjson_createObject();
 AZX_CJSON_T * myNum = azx_cjson_createNumber(5);
 azx_cjson_addItemToObject(myobj, "mynumber", myNum);  //will result in {"mynumber" : 5}
 AZX_CJSON_T * mycopy = azx_cjson_duplicate(myobj, 1); //mycopy will contain a full copy of myobj, child included
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_duplicate(AZX_CJSON_T *item, int recurse);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Parses the JSON string into an object, allows to require and check if the JSON is null terminated.

 @details
 This function parses the JSON string, and allows to check if the JSON is null terminated and returns the pointer to the final byte parsed.

 @param[in] value
 Source AZX_CJSON_T string to be parsed
 @param[in] return_parse_end
 If supplied, it will contain a pointer to the error (if any). Otherwise, azx_cjson_getErrorPtr() shall be used.
 @param[in] require_null_terminated
 If 1, it requires that the string is null terminated (no garbage bytes at the end of the JSON string)

 @return
 A AZX_CJSON_T object pointer in case of success,
 @return
 NULL in case of failure

 <b>Refer to</b>
 azx_cjson_delete()

 <b>Sample usage</b>
 @code
 AZX_CJSON_T *root = azx_cjson_parseWithOpts("{ \"name\": \"myname\"}", NULL, 1);
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
AZX_CJSON_T *azx_cjson_parseWithOpts(const char *value, const char **return_parse_end, int require_null_terminated);

/*-----------------------------------------------------------------------------------------------*/
/*!
 @brief
 Minifies the JSON string, removing any whitespaces, newlines, or unwanted characters

 @details
 This function will minify the JSON string, removing any whitespaces, newlines and unwanted characters
 (e.g. comments). The original string will be modified.

 @param[in] json
 Source AZX_CJSON_T string to be minified

 @return
 None

 <b>Sample usage</b>
 @code
 azx_cjson_minify("{  \"name\"   :\r\n\"myname\"\r\n\r\n }");  //will result in "{\"name\":\"myname\"}"
 @endcode

 @ingroup cjsonUsage
 */
/*-----------------------------------------------------------------------------------------------*/
void azx_cjson_minify(char *json);

#ifdef __cplusplus
}
#endif

#endif //HDR_AZX_CJSON__h
