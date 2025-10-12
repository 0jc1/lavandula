#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/json.h"

JsonBuilder *jsonBuilder() {
    JsonBuilder *builder = malloc(sizeof(JsonBuilder));
    builder->json = NULL;
    builder->jsonCount = 0;
    builder->jsonCapacity = 0;

    return builder;
}

JsonArray jsonArray() {
    JsonArray array = {};

    array.items = NULL;
    array.count = 0;
    array.capacity = 0;

    return array;
}

void freeJsonArray(JsonArray *jsonArray) {
    free(jsonArray->items);
}

void freeJsonBuilder(JsonBuilder *builder)
{
    for (int i = 0; i < builder->jsonCount; i++) {
        Json json = builder->json[i];

        if (json.type == JSON_STRING && json.value) {
            free(json.value);
        } else if (json.type == JSON_OBJECT && json.object) {
            freeJsonBuilder(json.object);
            // free(json.object);
        } else if (json.type == JSON_ARRAY && json.array) {
            // freeJsonBuilder(json.array);
            free(json.array->items);
        }

        if (json.key != NULL) {
            free(json.key);
        }
    }

    free(builder->json);

    builder->json = NULL;
    builder->jsonCount = 0;
    builder->jsonCapacity = 0;
}

void addJson(JsonBuilder *builder, Json json) {
    if (builder->jsonCount >= builder->jsonCapacity) {
        builder->jsonCapacity = builder->jsonCapacity == 0 ? 1 : builder->jsonCapacity * 2;
        builder->json = realloc(builder->json, sizeof(Json) * builder->jsonCapacity);
    }
    builder->json[builder->jsonCount++] = json;
}

Json makeJson(char *key, JsonType type) {
    return (Json){
        .type = type,
        .key = strdup(key),
    };
}

void jsonPutString(JsonBuilder *builder, char *key, char *value) {
    Json json = makeJson(key, JSON_STRING);
    json.value = strdup(value);

    addJson(builder, json);
}

void jsonPutBool(JsonBuilder *builder, char *key, bool value) {
    Json json = makeJson(key, value ? JSON_TRUE : JSON_FALSE);
    json.boolean = value;

    addJson(builder, json);
}

void jsonPutInteger(JsonBuilder *builder, char *key, int value) {
    Json json = makeJson(key, JSON_NUMBER);
    json.integer = value;

    addJson(builder, json);
}

void jsonPutNull(JsonBuilder *builder, char *key) {
    Json json = makeJson(key, JSON_NULL);

    addJson(builder, json);
}

void jsonPutObject(JsonBuilder *builder, char *key, JsonBuilder *object) {
    Json json = makeJson(key, JSON_OBJECT);
    json.object = object;

    addJson(builder, json);
}

void jsonPutArray(JsonBuilder *builder, char *key, JsonArray *array) {
    Json json = makeJson(key, JSON_ARRAY);
    json.array = array;

    addJson(builder, json);
}

Json jsonInteger(int value) {
    Json json = {
        .type = JSON_NUMBER,
        .key = NULL,
        .integer = value,
    };

    return json;
}

Json jsonBool(bool value) {
    Json json = {
        .type = value ? JSON_TRUE : JSON_FALSE,
        .key = NULL,
        .boolean = value,
    };
    return json;
}

Json jsonString(char *value) {
    Json json = {
        .type = JSON_STRING,
        .key = NULL,
        .value = strdup(value),
    };
    return json;
}

Json jsonObject(JsonBuilder *builder) {
    Json json = {
        .type = JSON_OBJECT,
        .key = NULL,
        .object = builder,
    };

    return json;
}

Json jsonArrayJson(JsonArray *array) {
    Json json = {
        .type = JSON_ARRAY,
        .key = NULL,
        .array = array,
    };

    return json;
}

void jsonArrayAppend(JsonArray *array, Json value) {
    if (array->count >= array->capacity) {
        array->capacity = array->capacity == 0 ? 1 : array->capacity * 2;
        array->items = realloc(array->items, sizeof(Json) * array->capacity);
    }
    array->items[array->count++] = value;
}

char *jsonStringify(JsonBuilder *builder) {
    int capacity = 16;
    int length = 0;
    char *json = malloc(capacity);

    json[length++] = '{';

    for (int i = 0; i < builder->jsonCount; i++) {
        Json node = builder->json[i];
        char buffer[256];

        switch (node.type) {
            case JSON_STRING:
                snprintf(buffer, sizeof(buffer), "\"%s\": \"%s\"", node.key, node.value);
                break;
            case JSON_TRUE:
                snprintf(buffer, sizeof(buffer), "\"%s\": true", node.key);
                break;
            case JSON_FALSE:
                snprintf(buffer, sizeof(buffer), "\"%s\": false", node.key);
                break;
            case JSON_NUMBER:
                snprintf(buffer, sizeof(buffer), "\"%s\": %d", node.key, node.integer);
                break;
            case JSON_NULL:
                snprintf(buffer, sizeof(buffer), "\"%s\": null", node.key);
                break;
            case JSON_ARRAY: {
                int arrCap = 64;
                int arrLen = 0;
                char *arrStr = malloc(arrCap);
                arrStr[arrLen++] = '[';

                for (int j = 0; j < node.array->count; j++) {
                    Json arrItem = node.array->items[j];
                    char arrBuf[256];
                    switch (arrItem.type) {
                        case JSON_STRING:
                            snprintf(arrBuf, sizeof(arrBuf), "\"%s\"", arrItem.value);
                            break;
                        case JSON_NUMBER:
                            snprintf(arrBuf, sizeof(arrBuf), "%d", arrItem.integer);
                            break;
                        case JSON_TRUE:
                            snprintf(arrBuf, sizeof(arrBuf), "true");
                            break;
                        case JSON_FALSE:
                            snprintf(arrBuf, sizeof(arrBuf), "false");
                            break;
                        case JSON_NULL:
                            snprintf(arrBuf, sizeof(arrBuf), "null");
                            break;
                        case JSON_OBJECT: {
                            char *nested = jsonStringify(arrItem.object);
                            snprintf(arrBuf, sizeof(arrBuf), "%s", nested);
                            free(nested);
                            break;
                        }
                        case JSON_ARRAY: {
                            // char *nested = jsonStringify(arrItem.array);
                            // snprintf(arrBuf, sizeof(arrBuf), "%s", nested);
                            // free(nested);
                            break;
                        }
                        default:
                            arrBuf[0] = '\0';
                            break;
                    }
                    int arrBufLen = strlen(arrBuf);
                    if (arrLen + arrBufLen + 3 > arrCap) {
                        arrCap = (arrLen + arrBufLen + 3) * 2;
                        arrStr = realloc(arrStr, arrCap);
                    }
                    memcpy(arrStr + arrLen, arrBuf, arrBufLen);
                    arrLen += arrBufLen;
                    if (j < node.array->count - 1) {
                        arrStr[arrLen++] = ',';
                        arrStr[arrLen++] = ' ';
                    }
                }
                arrStr[arrLen++] = ']';
                arrStr[arrLen] = '\0';
                snprintf(buffer, sizeof(buffer), "\"%s\": %s", node.key, arrStr);
                free(arrStr);
                break;
            }
            case JSON_OBJECT: {
                char *nestedJson = jsonStringify(node.object);
                snprintf(buffer, sizeof(buffer), "\"%s\": %s", node.key, nestedJson);
                free(nestedJson);
                break;
            }
            default:
                buffer[0] = '\0';
                break;
        }

        int bufferLength = strlen(buffer);
        if (length + bufferLength + 3 > capacity) {
            capacity = (length + bufferLength + 3) * 2;
            json = realloc(json, capacity);
        }

        memcpy(json + length, buffer, bufferLength);
        length += bufferLength;

        if (i < builder->jsonCount - 1) {
            json[length++] = ',';
            json[length++] = ' ';
        }
    }

    json[length++] = '}';
    json[length] = '\0';

    return json;
}

static char *skipWhitespace(char *str) {
    while (*str && (*str == ' ' || *str == '\t' || *str == '\n' || *str == '\r')) {
        str++;
    }
    return str;
}

static char *parseJsonString(char **str) {
    char *start = *str;
    if (*start != '"') return NULL;
    
    start++;
    char *end = start;
    
    while (*end && *end != '"') {
        end++;
    }
    
    if (*end != '"') return NULL;
    
    int length = end - start;
    char *result = malloc(length + 1);
    strncpy(result, start, length);
    result[length] = '\0';
    
    *str = end + 1;
    return result;
}

static int parseJsonNumber(char **str) {
    char *start = *str;
    int result = 0;
    int negative = 0;
    
    if (*start == '-') {
        negative = 1;
        start++;
    }
    
    while (*start >= '0' && *start <= '9') {
        result = result * 10 + (*start - '0');
        start++;
    }
    
    *str = start;
    return negative ? -result : result;
}

static Json parseJsonValue(char **str);

static JsonArray *parseJsonArray(char **str) {
    *str = skipWhitespace(*str);
    if (**str != '[') return NULL;
    
    (*str)++;
    JsonArray *array = malloc(sizeof(JsonArray));
    *array = jsonArray();
    
    *str = skipWhitespace(*str);
    
    if (**str == ']') {
        (*str)++;
        return array;
    }
    
    while (1) {
        *str = skipWhitespace(*str);
        Json value = parseJsonValue(str);
        jsonArrayAppend(array, value);
        
        *str = skipWhitespace(*str);
        if (**str == ']') {
            (*str)++;
            break;
        } else if (**str == ',') {
            (*str)++;
        } else {
            freeJsonArray(array);
            free(array);
            return NULL;
        }
    }
    
    return array;
}

static JsonBuilder *parseJsonObject(char **str) {
    *str = skipWhitespace(*str);
    if (**str != '{') return NULL;
    
    (*str)++;
    JsonBuilder *builder = jsonBuilder();
    
    *str = skipWhitespace(*str);
    
    if (**str == '}') {
        (*str)++;
        return builder;
    }
    
    while (1) {
        *str = skipWhitespace(*str);
        
        char *key = parseJsonString(str);
        if (!key) {
            freeJsonBuilder(builder);
            free(builder);
            return NULL;
        }
        
        *str = skipWhitespace(*str);
        if (**str != ':') {
            free(key);
            freeJsonBuilder(builder);
            free(builder);
            return NULL;
        }
        (*str)++;
        
        *str = skipWhitespace(*str);
        
        Json value = parseJsonValue(str);
        value.key = key;
        addJson(builder, value);
        
        *str = skipWhitespace(*str);
        if (**str == '}') {
            (*str)++;
            break;
        } else if (**str == ',') {
            (*str)++;
        } else {
            freeJsonBuilder(builder);
            free(builder);
            return NULL;
        }
    }
    
    return builder;
}

static Json parseJsonValue(char **str) {
    Json json = {0};
    *str = skipWhitespace(*str);
    
    if (**str == '"') {
        char *value = parseJsonString(str);
        json.type = JSON_STRING;
        json.value = value;
    } else if (**str == '{') {
        JsonBuilder *object = parseJsonObject(str);
        json.type = JSON_OBJECT;
        json.object = object;
    } else if (**str == '[') {
        JsonArray *array = parseJsonArray(str);
        json.type = JSON_ARRAY;
        json.array = array;
    } else if (strncmp(*str, "true", 4) == 0) {
        json.type = JSON_TRUE;
        json.boolean = true;
        *str += 4;
    } else if (strncmp(*str, "false", 5) == 0) {
        json.type = JSON_FALSE;
        json.boolean = false;
        *str += 5;
    } else if (strncmp(*str, "null", 4) == 0) {
        json.type = JSON_NULL;
        *str += 4;
    } else if (**str == '-' || (**str >= '0' && **str <= '9')) {
        int value = parseJsonNumber(str);
        json.type = JSON_NUMBER;
        json.integer = value;
    }
    
    return json;
}

JsonBuilder *jsonParse(char *jsonString) {
    if (!jsonString) return NULL;
    
    char *str = jsonString;
    str = skipWhitespace(str);
    
    if (*str != '{') return NULL;
    
    JsonBuilder *builder = parseJsonObject(&str);
    return builder;
}

char *jsonGetString(JsonBuilder *jsonBuilder, char *key) {
    for (int i = 0; i < jsonBuilder->jsonCount; i++) {
        Json json = jsonBuilder->json[i];
        
        if (json.type == JSON_STRING && strcmp(json.key, key) == 0) {
            return json.value;
        }
    }

    return NULL;
}

bool jsonGetBool(JsonBuilder *jsonBuilder, char *key) {
    for (int i = 0; i < jsonBuilder->jsonCount; i++) {
        Json json = jsonBuilder->json[i];

        if ((json.type == JSON_TRUE || json.type == JSON_FALSE) && strcmp(json.key, key) == 0) {
            return json.boolean;
        }
    }

    return false;
}

int jsonGetInteger(JsonBuilder *jsonBuilder, char *key) {
    for (int i = 0; i < jsonBuilder->jsonCount; i++) {
        Json json = jsonBuilder->json[i];

        if (json.type == JSON_NUMBER && strcmp(json.key, key) == 0) {
            return json.integer;
        }
    }

    return 0;
}

bool jsonHasKey(JsonBuilder *jsonBuilder, char *key) {
    for (int i = 0; i < jsonBuilder->jsonCount; i++) {
        Json json = jsonBuilder->json[i];

        if (strcmp(json.key, key) == 0) {
            return true;
        }
    }

    return false;
}

void jsonPrintNode(Json json) {
    switch (json.type) {
        case JSON_STRING:
            printf("\"%s\": \"%s\"", json.key, json.value);
            break;
        case JSON_TRUE:
            printf("\"%s\": true", json.key);
            break;
        case JSON_FALSE:
            printf("\"%s\": false", json.key);
            break;
        case JSON_NUMBER:
            printf("\"%s\": %d", json.key, json.integer);
            break;
        case JSON_NULL:
            printf("\"%s\": null", json.key);
            break;
        case JSON_ARRAY:
            break;
        case JSON_OBJECT:
            printf("\"%s\": ", json.key);
            jsonPrint(json.object);
            break;
        default:
            break;
    }
}

void jsonPrint(JsonBuilder *builder) {
    printf("{");
    for (int i = 0; i < builder->jsonCount; i++) {
        jsonPrintNode(builder->json[i]);

        if (i < builder->jsonCount - 1) {
            printf(", ");
        }
    }
    printf("}\n");
}