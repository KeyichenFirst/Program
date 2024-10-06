#include <ctype.h>
#include "postgres.h"
#include "math.h"
#include "string.h"
#include "fmgr.h"
#include "common/shortest_dec.h"
#include "libpq/pqformat.h"		
PG_MODULE_MAGIC;
#define MAXFLOATWIDTH 32

// 定义 Vector 结构体，表示向量类型
typedef struct Vector {
    int32 const_len;   // PostgreSQL 必须的长度字段
    int dimension;   // 向量的维度
    float data[];    // 可变长度数组，用于存储向量数据
} Vector;

PG_FUNCTION_INFO_V1(vector_in);
Datum
vector_in(PG_FUNCTION_ARGS)
{
    char *input_str = PG_GETARG_CSTRING(0); // 获取输入字符串
    Vector *result;
    char *token;
    float value;
    int open_braces = 0, close_braces = 0;  // 记录左右大括号的数量
    char *endptr;
    int dimension = 0;
    float values[1028];  // 向量的最大维度为 1024
    
    // 验证输入字符串是否合法

    int str_len = strlen(input_str);

    // 检查是否有连续的逗号或逗号是否出现在最后
    for (int i = 0; i < str_len - 1; i++) {
        if (input_str[i] == ',' && input_str[i + 1] == ',') {
            ereport(ERROR, (errmsg("错误：无效的向量格式")));
        }
        if (i == str_len - 2 && input_str[i] == ',') {
            ereport(ERROR, (errmsg("错误：无效的向量格式")));
        }
    }

    // 检查是否有未匹配的花括号
    for (int i = 0; i < str_len; i++) {
        if (input_str[i] == '{') open_braces++;
        if (input_str[i] == '}') close_braces++;
    }
    if (open_braces != 1 || close_braces != 1) {
        ereport(ERROR, (errmsg("错误：无效的向量格式。")));
    }

    // 检查向量格式是否符合 "{...}" 的规范
    if (input_str[0] == '{' && input_str[str_len - 1] == '}') {
        token = strtok(input_str, "{,}");  // 分割字符串，提取数值
        while (token != NULL) {
            if (dimension >= 1024)
                ereport(ERROR, (errmsg("错误：向量的维度")));
            
            // 去除空格
            while (isspace(*token)) token++;

            // 检查是否为有效的浮点数
            if (token[0] == '.') ereport(ERROR, (errmsg("错误：无效的向量格式")));  
            if (strlen(token) > 1 && (token[0] == '+' || token[0] == '-') && token[1] == '.') 
                ereport(ERROR, (errmsg("错误：无效的向量格式"))); 
            if (token[strlen(token) - 1] == '.') 
                ereport(ERROR, (errmsg("错误：无效的向量格式")));

            // 使用 strtof 将字符串转换为浮点数
            value = strtof(token, &endptr);
            
             // 如果转换后的值为 0 且原字符串不以 '0' 开头，则说明转换失败
            if (value == 0 && token == endptr) {
                ereport(ERROR, (errmsg("invalid vector")));
            }
            // 检查转换是否成功
            if (*endptr != '\0') {
                ereport(ERROR, (errmsg("错误：无效的向量格")));
            }

            values[dimension] = value;  // 存储转换后的浮点数
            dimension++;  // 增加维度
            token = strtok(NULL, "{,}");  // 继续分割下一个数值
        }
    } else {
        ereport(ERROR, (errmsg("错误：无效的向量格式")));
    }

    // 检查是否为 0 维向量
    if (dimension == 0) ereport(ERROR, (errmsg("错误：无效的向量格式")));

    // 分配内存并存储向量数据
    result = (Vector *) palloc(VARHDRSZ + sizeof(int) + sizeof(float) * dimension);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int) + sizeof(float) * dimension);
    result->dimension = dimension;
    for (int i = 0; i < dimension; i++) {
        result->data[i] = values[i];
    }

    PG_RETURN_POINTER(result);
}

// 向量输出函数
PG_FUNCTION_INFO_V1(vector_out);
Datum
vector_out(PG_FUNCTION_ARGS)
{
    Vector *vector = (Vector *) PG_GETARG_POINTER(0);
    char *result;

    StringInfoData buf;
    int count;
    char float_str[100];
    initStringInfo(&buf);

    appendStringInfoChar(&buf, '{');  // 输出以 { 开始
    count = 0;
    
    // 输出向量中的每个元素
    for (int i = 0; i < vector->dimension; i++) {
        count++;
        memset(float_str, '\0', sizeof(float_str));
        
        // 判断是否为整数输出
        if (vector->data[i] - (int)vector->data[i] == 0) {
            snprintf(float_str, sizeof(float_str), "%g", vector->data[i]);
        } else {
            float_to_shortest_decimal_bufn(vector->data[i], float_str);
        }

        appendStringInfo(&buf, "%s", float_str);

        if (i < vector->dimension - 1) {
            appendStringInfoChar(&buf, ',');  // 在元素之间添加逗号
        }
    }

    appendStringInfoChar(&buf, '}');  // 输出以 } 结束

    result = pstrdup(buf.data);  // 将输出结果转换为字符串

    PG_RETURN_CSTRING(result);
}

// 向量相加函数
PG_FUNCTION_INFO_V1(vector_add);
Datum
vector_add(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
    Vector *result;

    if (a->dimension != b->dimension)
        ereport(ERROR, (errmsg("维度不同，无法进行向量加法计算。")));

    result = (Vector *) palloc(VARHDRSZ + sizeof(int) + sizeof(float) * a->dimension);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int) + sizeof(float) * a->dimension);
    result->dimension = a->dimension;

    for (int i = 0; i < a->dimension; i++)
        result->data[i] = a->data[i] + b->data[i];

    PG_RETURN_POINTER(result);
}

// 向量相减函数
PG_FUNCTION_INFO_V1(vector_subtract);
Datum
vector_subtract(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
    Vector *result;

    if (a->dimension != b->dimension)
        ereport(ERROR, (errmsg("维度不同，无法进行向量减法计算。")));

    result = (Vector *) palloc(VARHDRSZ + sizeof(int) + sizeof(float) * a->dimension);
    SET_VARSIZE(result, VARHDRSZ + sizeof(int) + sizeof(float) * a->dimension);
    result->dimension = a->dimension;

    for (int i = 0; i < a->dimension; i++)
        result->data[i] = a->data[i] - b->data[i];

    PG_RETURN_POINTER(result);
}

// 计算向量的 L2 距离
PG_FUNCTION_INFO_V1(vector_l2_distance);
Datum
vector_l2_distance(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    Vector *b = (Vector *) PG_GETARG_POINTER(1);
    float sum = 0.0;

    if (a->dimension != b->dimension)
        ereport(ERROR, (errmsg("维度不同，无法计算 L2 距离。")));

    for (int i = 0; i < a->dimension; i++)
        sum += (a->data[i] - b->data[i]) * (a->data[i] - b->data[i]);

    PG_RETURN_FLOAT4(sqrt(sum));
}

// 获取向量的维度
PG_FUNCTION_INFO_V1(vector_dimensions);
Datum
vector_dimensions(PG_FUNCTION_ARGS)
{
    Vector *a = (Vector *) PG_GETARG_POINTER(0);
    PG_RETURN_INT32(a->dimension);
}
