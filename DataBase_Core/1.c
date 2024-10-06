/*
 * src/tutorial/complex.c
 *
 ******************************************************************************
  This file contains routines that can be bound to a Postgres backend and
  called by the backend in the process of processing queries.  The calling
  format for these routines is dictated by Postgres architecture.
******************************************************************************/

#include "postgres.h"
#include "math.h"
#include "string.h"
#include "fmgr.h"
#include "common/shortest_dec.h"
#include "libpq/pqformat.h"		/* needed for send/recv functions */

PG_MODULE_MAGIC;
#define MAXFLOATWIDTH 32

typedef struct Vector {
	int32 vl_len_;   // PostgreSQL 必须的长度字段
    int dim;         // 向量的维度
    float data[];    // 可变长度数组
} Vector;


/*****************************************************************************
 * Input/Output functions
 *****************************************************************************/
PG_FUNCTION_INFO_V1(vector_in);
Datum
vector_in(PG_FUNCTION_ARGS)
{
    char *str = PG_GETARG_CSTRING(0);
    Vector *result;
    char *token;
    int dim = 0;
    float values[1024];  // 向量最大1024维
	
	// elog(NOTICE,"ffff");
    token = strtok(str, "{, }");
    while (token != NULL)
    {
        if (dim >= 1024)
            ereport(ERROR, (errmsg("vector cannot big than 1024 dimensions")));
        
        values[dim] = strtof(token, NULL);
        dim++;
        token = strtok(NULL, "{, }");
    }
	// elog(NOTICE, "ffffff dimension: %d", dim);
	

    result = (Vector *) palloc(VARHDRSZ+  sizeof(int) + sizeof(float) * dim);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int)  + sizeof(float) * dim);
    result->dim = dim;
	for (int i = 0; i < dim; i++) {
		result->data[i] = values[i];
	}
	// elog(NOTICE, "最后一个元素: %f", result->data[dim-1]);
    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(vector_out);
Datum
vector_out(PG_FUNCTION_ARGS)
{
    Vector *vector = (Vector *) PG_GETARG_POINTER(0);
   char *result;

    StringInfoData buf;
	int cnt;
	char float_str[100];
    initStringInfo(&buf);
	// elog(NOTICE,"输出函数开始");
    appendStringInfoChar(&buf, '{');
   	// elog(NOTICE, "中间");
	cnt = 0;
	// elog(NOTICE,"循环开始之前dim=%d", vector->dim); 
	
    for (int i = 0; i < vector->dim; i++)
    {
	cnt++;
	// elog(NOTICE,"第%d个数为%f", cnt+1, vector->data[i]);
		memset(float_str, '\0', sizeof(float_str));  
        if (vector->data[i]-(int)vector->data[i] == 0) {
			snprintf(float_str, sizeof(float_str), "%g", vector->data[i]);
		} else {
			float_to_shortest_decimal_bufn(vector->data[i], float_str);
		}
        // snprintf(float_str, sizeof(float_str), "%g", vector->data[i]);
		// elog(NOTICE, "转换之后的值: %s", float_str);
		
		appendStringInfo(&buf, "%s", float_str);
		
        if (i < vector->dim - 1)
            appendStringInfoChar(&buf, ',');
    }
    // elog(NOTICE, "循环执行次数：%d", cnt);
	
    appendStringInfoChar(&buf, '}');
    
    result = pstrdup(buf.data);
	// elog(NOTICE, "结尾");
    PG_RETURN_CSTRING(result);
}

PG_FUNCTION_INFO_V1(vector_add);
Datum
vector_add(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
	Vector *result;
    if (a->dim != b->dim)
        ereport(ERROR, (errmsg("维度不同，无法计算")));

    result = (Vector *) palloc(VARHDRSZ + sizeof(int) + sizeof(float) * a->dim);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int) + sizeof(float) * a->dim);
    result->dim = a->dim;

    for (int i = 0; i < a->dim; i++)
        result->data[i] = a->data[i] + b->data[i];

    PG_RETURN_POINTER(result);
}

PG_FUNCTION_INFO_V1(vector_subtract);
Datum
vector_subtract(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
	Vector *result;
    if (a->dim != b->dim)
        ereport(ERROR, (errmsg("维度不同，无法计算。")));

    result = (Vector *) palloc(VARHDRSZ+ sizeof(int)  + sizeof(float) * a->dim);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int) + sizeof(float) * a->dim);
    result->dim = a->dim;

    for (int i = 0; i < a->dim; i++)
        result->data[i] = a->data[i] - b->data[i];

    PG_RETURN_POINTER(result);
}
PG_FUNCTION_INFO_V1(vector_l2_distance);
Datum
vector_l2_distance(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
	float sum = 0.0;
    if (a->dim != b->dim)
        ereport(ERROR, (errmsg("维度不同，无法计算")));

    

    for (int i = 0; i < a->dim; i++)
        sum += (a->data[i] - b->data[i]) * (a->data[i] - b->data[i]);

    PG_RETURN_FLOAT4(sqrt(sum));
}

PG_FUNCTION_INFO_V1(vector_dimensions);
Datum
vector_dimensions(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
	PG_RETURN_INT32(a->dim);
}


