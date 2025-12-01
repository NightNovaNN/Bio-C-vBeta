#ifndef BIOC_STD_H
#define BIOC_STD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* ===========================================
        BIOC STANDARD LIBRARY — v2
   =========================================== */


/* -----------------------
      PRINT HELPERS
   ----------------------- */

#define println_int(x) printf("%d\n", x)
#define println_str(x) printf("%s\n", x)
#define println_float(x) printf("%f\n", x)


/* -----------------------
      INPUT HELPERS
   ----------------------- */

static inline int read_int() {
    int x;
    scanf("%d", &x);
    return x;
}

static inline char* read_str() {
    char* s = malloc(1024);
    scanf("%1023s", s);
    return s;
}


/* -----------------------
     STRING UTILITIES
   ----------------------- */

static inline char* strip(char* s) {
    while (isspace(*s)) s++;
    char* end = s + strlen(s) - 1;
    while (end > s && isspace(*end)) *end-- = '\0';
    return s;
}

static inline char* lower(char* s) {
    for (char* p = s; *p; p++) *p = tolower(*p);
    return s;
}

static inline char* upper(char* s) {
    for (char* p = s; *p; p++) *p = toupper(*p);
    return s;
}

static inline bool startswith(const char* s, const char* prefix) {
    return strncmp(s, prefix, strlen(prefix)) == 0;
}

static inline bool endswith(const char* s, const char* suffix) {
    int slen = strlen(s), suflen = strlen(suffix);
    if (suflen > slen) return false;
    return strcmp(s + slen - suflen, suffix) == 0;
}

#define len(s) (strlen(s))

static inline char** split(const char* s, char delim, int* count) {
    char** result = malloc(128 * sizeof(char*));
    *count = 0;

    char* temp = strdup(s);
    char* token = strtok(temp, (char[]){delim, 0});

    while (token) {
        result[*count] = strdup(token);
        (*count)++;
        token = strtok(NULL, (char[]){delim, 0});
    }
    free(temp);
    return result;
}

static inline char* replace(const char* s, const char* from, const char* to) {
    char* out = malloc(2048);
    out[0] = 0;

    const char* pos = s;
    const char* f;

    while ((f = strstr(pos, from))) {
        strncat(out, pos, f - pos);
        strcat(out, to);
        pos = f + strlen(from);
    }
    strcat(out, pos);
    return out;
}


/* -----------------------
      MATH UTILITIES
   ----------------------- */

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))


/* -----------------------
       DYNAMIC ARRAY
   ----------------------- */

typedef struct {
    void** items;
    int size;
    int capacity;
} array;

static inline array* array_new() {
    array* a = malloc(sizeof(array));
    a->size = 0;
    a->capacity = 8;
    a->items = malloc(sizeof(void*) * a->capacity);
    return a;
}

static inline void array_push(array* a, void* item) {
    if (a->size >= a->capacity) {
        a->capacity *= 2;
        a->items = realloc(a->items, sizeof(void*) * a->capacity);
    }
    a->items[a->size++] = item;
}

static inline void* array_get(array* a, int i) {
    return (i < a->size) ? a->items[i] : NULL;
}

static inline int array_len(array* a) {
    return a->size;
}


/* -----------------------
          DICTIONARY
   ----------------------- */

typedef struct dict_item {
    char* key;
    void* value;
    struct dict_item* next;
} dict_item;

typedef struct {
    dict_item** table;
    int capacity;
} dict;

static inline unsigned hash(const char* s) {
    unsigned h = 5381;
    while (*s) h = ((h << 5) + h) + *s++;
    return h;
}

static inline dict* dict_new() {
    dict* d = malloc(sizeof(dict));
    d->capacity = 128;
    d->table = calloc(d->capacity, sizeof(dict_item*));
    return d;
}

static inline void dict_set(dict* d, const char* key, void* value) {
    unsigned idx = hash(key) % d->capacity;

    dict_item* it = d->table[idx];
    while (it) {
        if (strcmp(it->key, key) == 0) {
            it->value = value;
            return;
        }
        it = it->next;
    }

    dict_item* n = malloc(sizeof(dict_item));
    n->key = strdup(key);
    n->value = value;
    n->next = d->table[idx];
    d->table[idx] = n;
}

static inline void* dict_get(dict* d, const char* key) {
    unsigned idx = hash(key) % d->capacity;
    dict_item* it = d->table[idx];

    while (it) {
        if (strcmp(it->key, key) == 0) return it->value;
        it = it->next;
    }
    return NULL;
}

#endif
