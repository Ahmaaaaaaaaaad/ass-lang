#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "core/collection/vec.h"
#include "core/string/slice.h"
#include "core/string/string.h"
#include "utils/control.h"

define_vec(string_slice_t);

string_slice_t as_slicen(char const *s, size_t n)
{
    return (string_slice_t){s, n};
}

string_slice_t as_slice(char const *s) { return as_slicen(s, strlen(s)); }

size_t string_slice_cmp(string_slice_t a, string_slice_t b)
{
    return strncmp(a.data, b.data, a.n);
}

vec(string_slice_t) string_slice_split(string_slice_t slice, char const *delim)
{
    vec(string_slice_t) segments;
    vec_new(string_slice_t)(&segments, NULL, 4);

    size_t n = strlen(delim), curr_start = 0, curr_n = n == 0;

    for (size_t i = 0; i < slice.n; ++i)
    {
        bool is_last = i == slice.n - 1;
        if (memcmp(slice.data + i, delim, n) == 0 || is_last)
        {
            vec_push(string_slice_t)(
                &segments,
                as_slicen(slice.data + curr_start, curr_n + is_last));
            curr_start = i + (n ? n : 1);
            curr_n = n == 0;
            i += n ? n - 1 : n;
        }
        else
            ++curr_n;
    }

    return segments;
}

void string_slice_to_string(string_slice_t *slice, string_t *out)
{
    string_init(out);
    out->data = strndup(slice->data, slice->n);
    out->data[slice->n] = '\0';
    out->len = out->cap = slice->n;
}

void string_slice_trim_left(string_slice_t *slice)
{
    size_t i = 0;
    while (isspace(slice->data[i]))
        ++i;

    slice->data += i;
    slice->n -= i;
}

void string_slice_trim_right(string_slice_t *slice)
{
    size_t i = 0;
    while (isspace(slice->data[slice->n - i - 1]))
        ++i;

    slice->n -= i;
}

void string_slice_trim(string_slice_t *slice)
{
    string_slice_trim_left(slice);
    string_slice_trim_right(slice);
}

bool string_slice_starts_with(string_slice_t *str, char const *pref)
{
    size_t pref_len = strlen(pref);
    return str->n >= pref_len && strncmp(str->data, pref, pref_len) == 0;
}

bool string_slice_ends_with(string_slice_t *str, char const *suf)
{
    size_t suf_len = strlen(suf);
    return str->n >= suf_len &&
           strncmp(str->data + (str->n - suf_len), suf, suf_len) == 0;
}

hasher_decl(string_slice_t)
{
    string_t temp = (string_t){.data = (char *)val->data,
                               .el_destr = NULL,
                               .len = val->n,
                               .cap = val->n};
    return hasher(string_t)(&temp);
}

eq_decl(string_slice_t) { return string_slice_cmp(*a, *b) == 0; }
