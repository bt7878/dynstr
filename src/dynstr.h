#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DYNSTR_SSO_CAP 15
#define DYNSTR_MAX_CAP (SIZE_MAX / 2)

typedef struct dynstr_t {
    union {
        char *ptr;
        char buf[DYNSTR_SSO_CAP + 1];
    };

    size_t len;
    size_t cap;
} dynstr_t;

bool dynstr_create(dynstr_t *out, const char *str);

void dynstr_destroy(const dynstr_t *dynstr);

bool dynstr_clone(dynstr_t *out, const dynstr_t *dynstr);

void dynstr_clear(dynstr_t *dynstr);

const char *dynstr_cstr(const dynstr_t *dynstr);

size_t dynstr_cap(const dynstr_t *dynstr);

size_t dynstr_len(const dynstr_t *dynstr);

bool dynstr_reserve(dynstr_t *dynstr, size_t cap);

bool dynstr_shrink_to_fit(dynstr_t *dynstr);

char *dynstr_at(dynstr_t *dynstr, size_t i);

bool dynstr_append(dynstr_t *dynstr, const char *str);

bool dynstr_push(dynstr_t *dynstr, char c);

void dynstr_pop(dynstr_t *dynstr);

bool dynstr_insert(dynstr_t *dynstr, size_t pos, const char *str);

bool dynstr_remove(dynstr_t *dynstr, size_t pos, size_t len);

bool dynstr_substr(dynstr_t *out, const dynstr_t *dynstr, size_t pos, size_t len);
