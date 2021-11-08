/**
 * @file symtable.c
 * Symbol table operations implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */
#include "symtable.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

symtable_t *symtable_create(void)
{
    symtable_t *t = calloc(1,
            sizeof(size_t) +
            sizeof(size_t) +
            SYMTABLE_BUCKETS * sizeof(uintptr_t));

    if (!t)
        return NULL;

    t->size = 0;
    t->buckets = SYMTABLE_BUCKETS;

    // Initialize all pointers to items to NULL
    for (int i = 0; i < SYMTABLE_BUCKETS; i++)
        t->items[i] = NULL;

    return t;
}

size_t symtable_bucket_count(const symtable_t *t)
{
    return *(((size_t *) t) + 1);
}

void symtable_clear(symtable_t *t)
{
    assert(t);

    size_t buckets = symtable_bucket_count(t);
    symtable_item_t *item = NULL;
    symtable_item_t *next = NULL;

    // Go through all buckets
    for (unsigned int i = 0; i < buckets; i++) {

        item = t->items[i];
        if (!item) // Bucket is empty, continue to next one
            continue;

        do { // Traverse non empty bucket and free all items
            next = item->next;
            free((char *)item->identifier.name);
            if (item->identifier.type == FUNCTION) {
                if (item->identifier.fun.param)
                    free(item->identifier.fun.param);
                if (item->identifier.fun.retval)
                    free(item->identifier.fun.retval);
            }
            free(item);
            item = next;

        } while (item);
        t->items[i] = NULL;
    }
    t->size = 0;
}

void symtable_destroy(symtable_t *t)
{
    assert(t);

    symtable_clear(t);
    free(t);
}

// Source: http://isthe.com/chongo/tech/comp/fnv/ (FNV-1a)
#define OFFSET_BASIS 2166136261
#define FNV_PRIME 16777619
size_t symtable_hash_function(const char *str)
{
    uint32_t h = OFFSET_BASIS;
    const unsigned char *p;
    for (p = (const unsigned char *) str; *p != '\0'; p++) {
        h ^= *p;
        h *= FNV_PRIME;
    }

    return h;
}

identifier_t *symtable_find(symtable_t *t, char *name)
{
    assert(t);
    assert(name);

    size_t buckets = symtable_bucket_count(t);
    size_t hash = symtable_hash_function(name);
    symtable_item_t *item;

    // Get the first item in correct bucket
    item = t->items[hash % buckets];
    if (!item)
        return NULL;

    // If names match, return the identifier
    if (!strcmp(item->identifier.name, name))
        return &item->identifier;

    // Else traverse the bucket and try to find it
    item = item->next;
    while (item) {
        if (!strcmp(item->identifier.name, name))
            return &item->identifier; // return if found

        item = item->next;
    }

    return NULL;
}

symtable_item_t *create_item(char *name)
{
    assert(name);

    size_t name_len;
    symtable_item_t *item;

    // Allocate space on heap for item
    item = calloc(1, sizeof(symtable_item_t));
    if (!item)
        return NULL;

    // Allocate space on heap for name
    name_len = strlen(name) + 1; // Include '\0'
    item->identifier.name = calloc(1, name_len);
    if (!item->identifier.name) {
        free(item);
        return NULL;
    }

    // Copy name to the allocated space and set initial properties of the identifier
    strncpy((char *) item->identifier.name, name, name_len);
    item->identifier.line = 0;
    item->identifier.character = 0;
    item->identifier.var = (struct variable){0, };
    item->next = NULL;
    return item;
}

identifier_t *symtable_add(symtable_t *t, char *name)
{
    assert(t);
    assert(name);

    size_t buckets = symtable_bucket_count(t);
    size_t hash = symtable_hash_function(name);
    symtable_item_t *item;
    symtable_item_t *next;

    // Work in correct bucket as calculated by hash function
    item = t->items[hash % buckets];
    next = item;

    if (!item) { // Bucket is empty, need to create new item and add it

        item = create_item(name);
        if (!item)
            return NULL;

        // Adding new item to table
        t->items[hash % buckets] = item;

        // Increment current size and return pointer to created identifier
        t->size++;
        return &item->identifier;

    }
    // Items exists or hashes to non empty bucket,
    // Traverse bucket and return it or append a new one
    do {
        item = next;

        // Name in identifier matches given name
        if (!strcmp(item->identifier.name, name))
            return &item->identifier;

        next = item->next;
    } while (next);

    // Traversed bucket, not found the item, need to create and append it

    item->next = create_item(name);
    if (!item->next)
        return NULL;

    // Increment current size and return pointer to created identifier
    t->size += 1;
    return &item->next->identifier;
}

