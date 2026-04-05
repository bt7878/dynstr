#include "dynstr.h"

#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool is_large_cap(const size_t cap) {
    return cap > DYNSTR_SSO_CAP;
}

static char *get_ptr(dynstr_t *s) {
    return is_large_cap(s->cap) ? s->ptr : s->buf;
}

static const char *get_const_ptr(const dynstr_t *s) {
    return is_large_cap(s->cap) ? s->ptr : s->buf;
}

static size_t min_cap(size_t n) {
    if (n == 0) {
        return DYNSTR_SSO_CAP;
    }

    for (size_t i = 1; i < sizeof(size_t) * CHAR_BIT; i <<= 1) {
        n |= n >> i;
    }

    return n > DYNSTR_SSO_CAP ? n : DYNSTR_SSO_CAP;
}

bool dynstr_create(dynstr_t *out, const char *str) {
    const size_t len = strlen(str);
    const size_t cap = min_cap(len);
    if (cap > DYNSTR_MAX_CAP) {
        return false;
    }

    if (is_large_cap(cap)) {
        char *ptr = malloc(cap + 1);
        if (ptr == NULL) {
            return false;
        }
        out->ptr = ptr;
    }
    out->len = len;
    out->cap = cap;

    memcpy(get_ptr(out), str, len + 1);

    return true;
}

void dynstr_destroy(const dynstr_t *dynstr) {
    if (dynstr == NULL) {
        return;
    }
    if (is_large_cap(dynstr->cap)) {
        free(dynstr->ptr);
    }
}

bool dynstr_clone(dynstr_t *out, const dynstr_t *dynstr) {
    const size_t len = dynstr->len;
    const size_t cap = dynstr->cap;
    if (cap > DYNSTR_MAX_CAP) {
        return false;
    }

    if (is_large_cap(cap)) {
        char *ptr = malloc(cap + 1);
        if (ptr == NULL) {
            return false;
        }
        out->ptr = ptr;
    }
    out->len = len;
    out->cap = cap;

    memcpy(get_ptr(out), get_const_ptr(dynstr), len + 1);

    return true;
}

void dynstr_clear(dynstr_t *dynstr) {
    get_ptr(dynstr)[0] = '\0';
    dynstr->len = 0;
}

const char *dynstr_cstr(const dynstr_t *dynstr) {
    return get_const_ptr(dynstr);
}

size_t dynstr_cap(const dynstr_t *dynstr) {
    return dynstr->cap;
}

size_t dynstr_len(const dynstr_t *dynstr) {
    return dynstr->len;
}

bool dynstr_reserve(dynstr_t *dynstr, const size_t cap) {
    const size_t requested_cap = min_cap(cap);
    if (requested_cap > DYNSTR_MAX_CAP) {
        return false;
    }
    if (requested_cap <= dynstr->cap) {
        return true;
    }

    const bool small_to_large = !is_large_cap(dynstr->cap) && is_large_cap(requested_cap);
    char *new_ptr = realloc(small_to_large ? NULL : dynstr->ptr, requested_cap + 1);
    if (new_ptr == NULL) {
        return false;
    }
    if (small_to_large) {
        memcpy(new_ptr, dynstr->buf, dynstr->len + 1);
    }

    dynstr->ptr = new_ptr;
    dynstr->cap = requested_cap;

    return true;
}

bool dynstr_shrink_to_fit(dynstr_t *dynstr) {
    const size_t new_cap = min_cap(dynstr->len);
    if (new_cap >= dynstr->cap) {
        return true;
    }

    const bool large_to_small = is_large_cap(dynstr->cap) && !is_large_cap(new_cap);
    if (large_to_small) {
        char *old_ptr = dynstr->ptr;
        memcpy(dynstr->buf, old_ptr, dynstr->len + 1);
        free(old_ptr);
    } else {
        char *new_ptr = realloc(dynstr->ptr, new_cap + 1);
        if (new_ptr == NULL) {
            return false;
        }
        dynstr->ptr = new_ptr;
    }
    dynstr->cap = new_cap;

    return true;
}

char *dynstr_at(dynstr_t *dynstr, const size_t i) {
    return get_ptr(dynstr) + i;
}

bool dynstr_append(dynstr_t *dynstr, const char *str) {
    const size_t to_append_len = strlen(str);
    if (to_append_len > SIZE_MAX - dynstr->len) {
        return false;
    }
    const size_t new_len = dynstr->len + to_append_len;

    if (!dynstr_reserve(dynstr, new_len)) {
        return false;
    }

    memcpy(get_ptr(dynstr) + dynstr->len, str, to_append_len + 1);
    dynstr->len = new_len;

    return true;
}

bool dynstr_push(dynstr_t *dynstr, const char c) {
    if (!dynstr_reserve(dynstr, dynstr->len + 1)) {
        return false;
    }

    get_ptr(dynstr)[dynstr->len] = c;
    get_ptr(dynstr)[dynstr->len + 1] = '\0';
    dynstr->len++;

    return true;
}

void dynstr_pop(dynstr_t *dynstr) {
    if (dynstr->len == 0) {
        return;
    }

    get_ptr(dynstr)[dynstr->len - 1] = '\0';
    dynstr->len--;
}

bool dynstr_insert(dynstr_t *dynstr, const size_t pos, const char *str) {
    if (pos > dynstr->len) {
        return false;
    }

    const size_t str_len = strlen(str);
    if (str_len > SIZE_MAX - dynstr->len) {
        return false;
    }
    if (!dynstr_reserve(dynstr, dynstr->len + str_len)) {
        return false;
    }

    memmove(get_ptr(dynstr) + pos + str_len, get_ptr(dynstr) + pos, dynstr->len - pos + 1);
    memcpy(get_ptr(dynstr) + pos, str, str_len);
    dynstr->len += str_len;

    return true;
}

bool dynstr_remove(dynstr_t *dynstr, const size_t pos, const size_t len) {
    if (pos > dynstr->len || len > dynstr->len - pos) {
        return false;
    }

    memmove(get_ptr(dynstr) + pos, get_ptr(dynstr) + pos + len, dynstr->len - pos - len + 1);
    dynstr->len -= len;

    return true;
}

bool dynstr_substr(dynstr_t *out, const dynstr_t *dynstr, const size_t pos, const size_t len) {
    if (pos > dynstr->len || len > dynstr->len - pos) {
        return false;
    }

    const size_t cap = min_cap(len);

    if (is_large_cap(cap)) {
        char *ptr = malloc(cap + 1);
        if (ptr == NULL) {
            return false;
        }
        out->ptr = ptr;
    }
    out->len = len;
    out->cap = cap;

    memcpy(get_ptr(out), get_const_ptr(dynstr) + pos, len);
    get_ptr(out)[len] = '\0';

    return true;
}
