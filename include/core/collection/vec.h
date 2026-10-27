#ifndef __CORE_COLLECTION_VEC_H__
#define __CORE_COLLECTION_VEC_H__

#include <stdlib.h> // Library for the macros

#include "core/collection/slice.h"
#include "core/iter/iter.h"
#include "utils/control.h"

#define vec_new(T) T##_vec_new
#define decl_vec_new(T)                                                        \
    void vec_new(T)(vec(T) * vec, void (*el_destr)(T *), size_t cap)
#define def_vec_new(T)                                                         \
    decl_vec_new(T)                                                            \
    {                                                                          \
        T *temp = malloc(cap * sizeof(T));                                     \
        if (temp == NULL)                                                      \
            panic("failed to allocate vector buffer");                         \
        vec->data = temp;                                                      \
        vec->el_destr = el_destr;                                              \
        vec->cap = cap;                                                        \
        vec->len = 0;                                                          \
    }

#define vec_new_sized(T) T##_vec_new_sized
#define decl_vec_new_sized(T)                                                  \
    void vec_new_sized(T)(vec(T) * vec, void (*el_destr)(T *), size_t size)
#define def_vec_new_sized(T)                                                   \
    decl_vec_new_sized(T)                                                      \
    {                                                                          \
        T *temp = calloc(size, sizeof(T));                                     \
        if (temp == NULL)                                                      \
            panic("failed to allocate vector buffer");                         \
        vec->data = temp;                                                      \
        vec->el_destr = el_destr;                                              \
        vec->len = vec->cap = size;                                            \
    }

#define vec_free(T) T##_vec_free
#define decl_vec_free(T) void vec_free(T)(vec(T) * vec)
#define def_vec_free(T)                                                        \
    decl_vec_free(T)                                                           \
    {                                                                          \
        if (vec->el_destr != NULL)                                             \
            for (size_t i = 0; i < vec->len; ++i)                              \
                vec->el_destr(vec->data + i);                                  \
        free(vec->data);                                                       \
    }

#define vec_update_cap(T) T##_vec_update_cap
#define decl_vec_update_cap(T) void vec_update_cap(T)(vec(T) * vec, size_t cap)
#define def_vec_update_cap(T)                                                  \
    decl_vec_update_cap(T)                                                     \
    {                                                                          \
        if (vec->cap >= cap)                                                   \
            return;                                                            \
        T *temp = realloc(vec->data, cap * sizeof(T));                         \
        if (temp == NULL)                                                      \
            panic("failed to reallocate vector buffer");                       \
        vec->data = temp;                                                      \
        vec->cap = cap;                                                        \
    }

#define vec_iter(T) T##_vec_iter
#define decl_vec_iter(T) iter(T) vec_iter(T)(vec(T) * vec)
#define def_vec_iter(T)                                                        \
    decl_vec_iter(T) { return iter_new(T)(vec->data, vec->len); }

#define vec_push(T) T##_vec_push
#define decl_vec_push(T) void vec_push(T)(vec(T) * vec, T el)
#define def_vec_push(T)                                                        \
    decl_vec_push(T)                                                           \
    {                                                                          \
        vec->data[vec->len++] = el;                                            \
        if (vec->len >= vec->cap)                                              \
            vec_update_cap(T)(vec, vec->cap * 2);                              \
    }

#define vec_get(T) T##_vec_get
#define decl_vec_get(T) T const *vec_get(T)(vec(T) const *vec, size_t idx)
#define def_vec_get(T)                                                         \
    decl_vec_get(T) { return (idx >= vec->len) ? NULL : vec->data + idx; }

#define vec_get_checked(T) T##_vec_get_checked
#define decl_vec_get_checked(T)                                                \
    T const *vec_get_checked(T)(vec(T) const *vec, size_t idx)
#define def_vec_get_checked(T)                                                 \
    decl_vec_get_checked(T)                                                    \
    {                                                                          \
        if (idx >= vec->len)                                                   \
            panic("vector access out of bounds");                              \
        return vec_get(T)(vec, idx);                                           \
    }

#define vec_get_mut(T) T##_vec_get_mut
#define decl_vec_get_mut(T) T *vec_get_mut(T)(vec(T) * vec, size_t idx)
#define def_vec_get_mut(T)                                                     \
    decl_vec_get_mut(T) { return (T *)vec_get(T)(vec, idx); }

#define vec_get_mut_checked(T) T##_vec_get_mut_checked
#define decl_vec_get_mut_checked(T)                                            \
    T *vec_get_mut_checked(T)(vec(T) * vec, size_t idx)
#define def_vec_get_mut_checked(T)                                             \
    decl_vec_get_mut_checked(T)                                                \
    {                                                                          \
        if (idx >= vec->len)                                                   \
            panic("vector access out of bounds");                              \
        return vec_get_mut(T)(vec, idx);                                       \
    }

#define vec_first(T) T##_vec_first
#define decl_vec_first(T) T const *vec_first(T)(vec(T) const *vec)
#define def_vec_first(T)                                                       \
    decl_vec_first(T) { return vec->len == 0 ? NULL : vec->data; }

#define vec_first_mut(T) T##_vec_first_mut
#define decl_vec_first_mut(T) T *vec_first_mut(T)(vec(T) * vec)
#define def_vec_first_mut(T)                                                   \
    decl_vec_first_mut(T) { return (T *)vec_first(T)(vec); }

#define vec_last(T) T##_vec_last
#define decl_vec_last(T) T const *vec_last(T)(vec(T) const *vec)
#define def_vec_last(T)                                                        \
    decl_vec_last(T)                                                           \
    {                                                                          \
        return vec->len == 0 ? NULL : vec->data + (vec->len - 1);              \
    }

#define vec_last_mut(T) T##_vec_last_mut
#define decl_vec_last_mut(T) T *vec_last_mut(T)(vec(T) * vec)
#define def_vec_last_mut(T)                                                    \
    decl_vec_last_mut(T) { return (T *)vec_last(T)(vec); }

#define vec_expand(T) T##_vec_expand
#define decl_vec_expand(T) T *vec_expand(T)(vec(T) * vec)
#define def_vec_expand(T)                                                      \
    decl_vec_expand(T)                                                         \
    {                                                                          \
        if (vec->len >= vec->cap)                                              \
            vec_update_cap(T)(vec, vec->cap * 2);                              \
        return vec_get_mut(T)(vec, vec->len++);                                \
    }

#define declare_vec(T)                                                         \
    typedef struct                                                             \
    {                                                                          \
        T *data;                                                               \
        void (*el_destr)(T *);                                                 \
        size_t len, cap;                                                       \
    } vec(T);                                                                  \
    declare_slice(T);                                                          \
    declare_iter(T);                                                           \
    decl_vec_new(T);                                                           \
    decl_vec_new_sized(T);                                                     \
    decl_vec_free(T);                                                          \
    decl_vec_update_cap(T);                                                    \
    decl_vec_iter(T);                                                          \
    decl_vec_push(T);                                                          \
    decl_vec_get_mut(T);                                                       \
    decl_vec_get_mut_checked(T);                                               \
    decl_vec_get(T);                                                           \
    decl_vec_get_checked(T);                                                   \
    decl_vec_first(T);                                                         \
    decl_vec_first_mut(T);                                                     \
    decl_vec_last(T);                                                          \
    decl_vec_last_mut(T);                                                      \
    decl_vec_expand(T);

#define define_vec(T)                                                          \
    define_slice(T);                                                           \
    define_iter(T);                                                            \
    def_vec_new(T);                                                            \
    def_vec_new_sized(T);                                                      \
    def_vec_free(T);                                                           \
    def_vec_update_cap(T);                                                     \
    def_vec_iter(T);                                                           \
    def_vec_push(T);                                                           \
    def_vec_get_mut(T);                                                        \
    def_vec_get_mut_checked(T);                                                \
    def_vec_get(T);                                                            \
    def_vec_get_checked(T);                                                    \
    def_vec_first(T);                                                          \
    def_vec_first_mut(T);                                                      \
    def_vec_last(T);                                                           \
    def_vec_last_mut(T);                                                       \
    def_vec_expand(T);

#define vec(T) T##_vec_t

#define vec_for_each(T, el, vec, block)                                        \
    for (size_t i = 0; i < vec.len; ++i)                                       \
    {                                                                          \
        T const *el = vec_get(T)(&vec, i);                                     \
        block                                                                  \
    }

#define vec_for_each_mut(T, el, vec, block)                                    \
    for (size_t i = 0; i < vec.len; ++i)                                       \
    {                                                                          \
        T *el = vec_get_mut(T)(&vec, i);                                       \
        block                                                                  \
    }

#endif
