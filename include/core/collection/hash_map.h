#ifndef __CORE_COLLECTION_HASH_MAP_H__
#define __CORE_COLLECTION_HASH_MAP_H__

#include "utils/general.h"

#define REHASH_THRESHOLD 0.75

#define hm_kv(K, V) concat(concat(K, _), V)
#define hm_entry(K, V) concat(hm_kv(K, V), _hm_entry_t)
#define hash_map(K, V) concat(hm_kv(K, V), _hm_t)

#define hm_new(K, V) concat(hm_kv(K, V), _hm_new)
#define decl_hm_new(K, V)                                                      \
    void hm_new(K, V)(hash_map(K, V) * hm, size_t (*key_hash)(K const *),      \
                      bool (*key_eq)(K const *, K const *),                    \
                      void (*key_destr)(K *), void (*val_destr)(V *))
#define def_hm_new(K, V)                                                       \
    decl_hm_new(K, V)                                                          \
    {                                                                          \
        assert(key_hash != NULL, "missing required key hasher");               \
        assert(key_eq != NULL, "missing required key comparator");             \
                                                                               \
        hm->cap = 8, hm->len = 0;                                              \
                                                                               \
        hm_entry(K, V) **temp = calloc(8, sizeof(hm_entry(K, V) *));           \
        if (temp == NULL)                                                      \
            panic("failed to allocate hash map buffer");                       \
                                                                               \
        hm->entries = temp;                                                    \
        hm->key_hash = key_hash;                                               \
        hm->key_eq = key_eq;                                                   \
        hm->key_destr = key_destr;                                             \
        hm->val_destr = val_destr;                                             \
    }

#define hm_free(K, V) concat(hm_kv(K, V), _hm_free)
#define decl_hm_free(K, V) void hm_free(K, V)(hash_map(K, V) * hm)
#define def_hm_free(K, V)                                                      \
    decl_hm_free(K, V)                                                         \
    {                                                                          \
        for (size_t i = 0; i < hm->cap; ++i)                                   \
        {                                                                      \
            hm_entry(K, V) *entry = hm->entries[i];                            \
            if (entry == NULL)                                                 \
                continue;                                                      \
                                                                               \
            if (hm->key_destr)                                                 \
                hm->key_destr(&entry->key);                                    \
                                                                               \
            if (hm->val_destr)                                                 \
                hm->val_destr(&entry->val);                                    \
                                                                               \
            free(entry);                                                       \
        }                                                                      \
                                                                               \
        free(hm->entries);                                                     \
    }

#define hm_load_factor(K, V) concat(hm_kv(K, V), _hm_load_factor)
#define decl_hm_load_factor(K, V)                                              \
    double hm_load_factor(K, V)(hash_map(K, V) * hm)
#define def_hm_load_factor(K, V)                                               \
    decl_hm_load_factor(K, V) { return (double)hm->len / hm->cap; }

#define hm_hash_key(K, V) concat(hm_kv(K, V), _hm_hash_key)
#define decl_hm_hash_key(K, V)                                                 \
    size_t hm_hash_key(K, V)(hash_map(K, V) const *hm, K const *key)
#define def_hm_hash_key(K, V)                                                  \
    decl_hm_hash_key(K, V) { return hm->key_hash(key) % hm->cap; }

#define hm_rehash(K, V) concat(hm_kv(K, V), _hm_rehash)
#define decl_hm_rehash(K, V) void hm_rehash(K, V)(hash_map(K, V) * hm)
#define def_hm_rehash(K, V)                                                    \
    decl_hm_rehash(K, V)                                                       \
    {                                                                          \
        if (hm_load_factor(K, V)(hm) < REHASH_THRESHOLD)                       \
            return;                                                            \
                                                                               \
        size_t init_cap = hm->cap;                                             \
        hm->cap *= 2;                                                          \
                                                                               \
        hm_entry(K, V) **temp = calloc(hm->cap, sizeof(hm_entry(K, V) *));     \
        if (temp == NULL)                                                      \
            panic("failed to allocate hash map entry buffer");                 \
                                                                               \
        for (size_t i = 0; i < init_cap; ++i)                                  \
        {                                                                      \
            hm_entry(K, V) *entry = hm->entries[i];                            \
            if (entry == NULL)                                                 \
                continue;                                                      \
                                                                               \
            hm_entry(K, V) *curr = entry;                                      \
            while (curr != NULL)                                               \
            {                                                                  \
                size_t hash = hm_hash_key(K, V)(hm, &curr->key);               \
                hm_entry(K, V) *exst = temp[hash];                             \
                                                                               \
                if (exst == NULL)                                              \
                    temp[hash] = curr;                                         \
                else                                                           \
                {                                                              \
                    while (exst->next != NULL)                                 \
                        exst = exst->next;                                     \
                                                                               \
                    exst->next = curr;                                         \
                }                                                              \
                                                                               \
                hm_entry(K, V) *temp = curr->next;                             \
                curr->next = NULL;                                             \
                curr = temp;                                                   \
            }                                                                  \
        }                                                                      \
                                                                               \
        free(hm->entries);                                                     \
        hm->entries = temp;                                                    \
    }

#define hm_insert(K, V) concat(hm_kv(K, V), _hm_insert)
#define decl_hm_insert(K, V)                                                   \
    void hm_insert(K, V)(hash_map(K, V) * hm, K key, V val)
#define def_hm_insert(K, V)                                                    \
    decl_hm_insert(K, V)                                                       \
    {                                                                          \
        if (hm_load_factor(K, V)(hm) >= REHASH_THRESHOLD)                      \
            hm_rehash(K, V)(hm);                                               \
                                                                               \
        size_t hash = hm_hash_key(K, V)(hm, &key);                             \
        hm_entry(K, V) **entry = hm->entries + hash;                           \
                                                                               \
        if (*entry == NULL)                                                    \
        {                                                                      \
            hm_entry(K, V) *temp = malloc(sizeof(hm_entry(K, V)));             \
            if (temp == NULL)                                                  \
                panic("failed to allocate hash map entry");                    \
                                                                               \
            temp->key = key;                                                   \
            temp->val = val;                                                   \
                                                                               \
            *entry = temp;                                                     \
        }                                                                      \
                                                                               \
        else                                                                   \
        {                                                                      \
            hm_entry(K, V) *curr = *entry;                                     \
            while (curr->next != NULL)                                         \
                curr = curr->next;                                             \
                                                                               \
            hm_entry(K, V) *temp = malloc(sizeof(hm_entry(K, V)));             \
            if (temp == NULL)                                                  \
                panic("failed to allocate hash map entry");                    \
                                                                               \
            curr = curr->next = temp;                                          \
            curr->key = key;                                                   \
            curr->val = val;                                                   \
        }                                                                      \
                                                                               \
        ++hm->len;                                                             \
    }

#define hm_get(K, V) concat(hm_kv(K, V), _hm_get)
#define decl_hm_get(K, V)                                                      \
    V const *hm_get(K, V)(hash_map(K, V) const *hm, K const *key)
#define def_hm_get(K, V)                                                       \
    decl_hm_get(K, V)                                                          \
    {                                                                          \
        size_t hash = hm_hash_key(K, V)(hm, key);                              \
        hm_entry(K, V) const *entry = hm->entries[hash], *curr = entry;        \
                                                                               \
        while (curr != NULL && !hm->key_eq(&curr->key, key))                   \
            curr = curr->next;                                                 \
                                                                               \
        return curr == NULL ? NULL : &curr->val;                               \
    }

#define hm_get_mut(K, V) concat(hm_kv(K, V), _hm_get_mut)
#define decl_hm_get_mut(K, V) V *hm_get_mut(K, V)(hash_map(K, V) * hm, K * key)
#define def_hm_get_mut(K, V)                                                   \
    decl_hm_get_mut(K, V) { return (V *)hm_get(K, V)(hm, key); }

#define declare_hm(K, V)                                                       \
    typedef struct hm_entry(K, V)                                              \
    {                                                                          \
        V val;                                                                 \
        K key;                                                                 \
        struct hm_entry(K, V) * next;                                          \
    } hm_entry(K, V);                                                          \
    typedef struct                                                             \
    {                                                                          \
        hm_entry(K, V) * *entries;                                             \
        size_t cap, len;                                                       \
        size_t (*key_hash)(K const *);                                         \
        bool (*key_eq)(K const *, K const *);                                  \
        void (*key_destr)(K *);                                                \
        void (*val_destr)(V *);                                                \
    } hash_map(K, V);                                                          \
    decl_hm_new(K, V);                                                         \
    decl_hm_free(K, V);                                                        \
    decl_hm_load_factor(K, V);                                                 \
    decl_hm_hash_key(K, V);                                                    \
    decl_hm_rehash(K, V);                                                      \
    decl_hm_insert(K, V);                                                      \
    decl_hm_get(K, V);                                                         \
    decl_hm_get_mut(K, V);

#define define_hm(K, V)                                                        \
    def_hm_new(K, V);                                                          \
    def_hm_free(K, V);                                                         \
    def_hm_load_factor(K, V);                                                  \
    def_hm_hash_key(K, V);                                                     \
    def_hm_rehash(K, V);                                                       \
    def_hm_insert(K, V);                                                       \
    def_hm_get(K, V);                                                          \
    def_hm_get_mut(K, V);

#define hasher(T) T##_hasher
#define hasher_decl(T) size_t hasher(T)(T const *val)

#define eq(T) T##_eq
#define eq_decl(T) bool eq(T)(T const *a, T const *b)

#endif
