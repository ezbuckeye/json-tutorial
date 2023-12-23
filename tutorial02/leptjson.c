#include "leptjson.h"
#include <math.h>
#include <errno.h>
#include <stdlib.h> 
#include <assert.h>  /* assert() */ 
#include <stdlib.h>  /* NULL, strtod() */ 
#include <string.h> 
#include <ctype.h> 
#define EXPECT(c, ch)       do { assert(*c->json == (ch)); c->json++; } while(0) 

typedef struct { 
    const char* json; 
}lept_context; 

static int is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void lept_parse_whitespace(lept_context* c) { 
    const char *p = c->json; 
    while(is_whitespace(*p))
        p++;
    c->json = p;
}

static int lept_parse_literal(lept_context* c, lept_value* v, char* expected) {
    int size, i;
    size = strlen(expected);
    for(i=0; i<size; i++) {
        if(c->json[i] != expected[i])   return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += size;
    switch(expected[0]) {
        case 'n':   v->type = LEPT_NULL;    break;
        case 't':   v->type = LEPT_TRUE;    break;
        case 'f':   v->type = LEPT_FALSE;   break;
        default:    v->type = LEPT_NULL;
    }
    return LEPT_PARSE_OK;
}

/*
static int lept_parse_true(lept_context* c, lept_value* v) { EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context* c, lept_value* v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context* c, lept_value* v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}
*/

static void validate_neg(char** json_cpy) {
    if(*json_cpy[0] == '-') (*json_cpy)++;  
}

static int validate_int(char** json_cpy) {
    int ret;
    ret = isdigit(*json_cpy[0]);
    if(ret) {
        if(*json_cpy[0]!='0') 
            for(; isdigit(*json_cpy[0]); (*json_cpy)++);
        else
            (*json_cpy)++;
    }
    return ret;
}

static int validate_frac(char** json_cpy) {
    if(*json_cpy[0]=='.') {
        (*json_cpy)++;
        if(!isdigit(*json_cpy[0])) return 0;
        for(; isdigit(*json_cpy[0]); (*json_cpy)++);
    }
    return 1;
}

static int validate_sign(char** json_cpy) {
    return (*json_cpy[0] == '+' || *json_cpy[0] == '-');
}

static int validate_exp(char** json_cpy) {
    if(*json_cpy[0]=='e' || *json_cpy[0]=='E') {
        (*json_cpy)++;
        if(validate_sign(json_cpy)) {
            (*json_cpy)++;    
        }
        if(!isdigit(*json_cpy[0]))   return 0;
        for(; isdigit(*json_cpy[0]); (*json_cpy)++);
    }
    return 1;
}

static int lept_parse_number(lept_context* c, lept_value* v) {
    char* json_cpy;
    void* json_cpy_head;
    int validation_failure;
    

    json_cpy_head = malloc(strlen(c->json)+1);
    json_cpy = (char*)json_cpy_head;
    strcpy(json_cpy, c->json);
    validate_neg(&json_cpy);
    validation_failure = !validate_int(&json_cpy)||!validate_frac(&json_cpy)||!validate_exp(&json_cpy);
    if(validation_failure)  return LEPT_PARSE_INVALID_VALUE;
    if(!is_whitespace(*json_cpy) && *json_cpy!='\0')    return LEPT_PARSE_OK; 
    errno = 0;
    v->n = strtod(c->json, NULL);
    if(errno == ERANGE && (v->n==HUGE_VAL || v->n==-HUGE_VAL))    return LEPT_PARSE_NUMBER_TOO_BIG; 
    c->json += (json_cpy-(char*)json_cpy_head);
    free(json_cpy_head);
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context* c, lept_value* v) {
    switch (*c->json) {
        case 't':  return lept_parse_literal(c, v, "true"); 
        case 'f':  return lept_parse_literal(c, v, "false");
        case 'n':  return lept_parse_literal(c, v, "null");
        default:   return lept_parse_number(c, v);
        case '\0': return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value* v, const char* json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value* v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value* v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
