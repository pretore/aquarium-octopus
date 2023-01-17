#include <stdlib.h>
#include <assert.h>
#include <seagrass.h>
#include <octopus.h>

#include "private/linked_queue.h"

#ifdef TEST
#include <test/cmocka.h>
#endif

static bool retrieve(struct octopus_concurrent_linked_queue *const object,
                     const uintmax_t concurrency,
                     const uintmax_t at,
                     void **const out,
                     bool (*func)(struct octopus_linked_queue *, void **)) {
    assert(object);
    assert(concurrency);
    assert(out);
    assert(func);
    uintmax_t qr[2];
    seagrass_required_true(seagrass_uintmax_t_divide(
            at, concurrency, &qr[0], &qr[1]));
    struct octopus_linked_queue *queue;
    seagrass_required_true(coral_array_list_get(
            &object->queues, qr[1], (void **) &queue));
    const bool result = func(queue, out);
    if (!result) {
        seagrass_required_true(OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY
                               == octopus_error);
    }
    return result;
}

static void
concurrency(const struct octopus_concurrent_linked_queue *const object,
            uintmax_t *const out) {
    assert(object);
    assert(out);
    seagrass_required_true(coral_array_list_get_length(
            &object->queues, out));
}

static bool remove(struct octopus_concurrent_linked_queue *const object,
                   void **const out) {
    assert(object);
    assert(out);
    uintmax_t c;
    concurrency(object, &c);
    const uintmax_t begin = atomic_fetch_add(&object->dequeue, 1);
    const uintmax_t end = begin + c; /* allow integer overflow */
    uintmax_t at = begin;
    while ((begin < end && at < end)
           || ((begin > end && at >= begin) || at < end)) {
        if (retrieve(object, c, at, out, octopus_linked_queue_remove)) {
            return true;
        }
        at = atomic_fetch_add(&object->dequeue, 1);
    }
    if (retrieve(object, c, at, out, octopus_linked_queue_remove)) {
        return true;
    }
    octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY;
    return false;
}

bool octopus_concurrent_linked_queue_init(
        struct octopus_concurrent_linked_queue *const object,
        const size_t size,
        const uintmax_t concurrency) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!size) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_SIZE_IS_ZERO;
        return false;
    }
    if (!concurrency) {
        octopus_error =
                OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_CONCURRENCY_IS_ZERO;
        return false;
    }
    *object = (struct octopus_concurrent_linked_queue) {0};
    if (!coral_array_list_init(&object->queues,
                               sizeof(struct octopus_linked_queue),
                               concurrency)) {
        seagrass_required_true(CORAL_ARRAY_LIST_ERROR_MEMORY_ALLOCATION_FAILED
                               == coral_error);
        octopus_error =
                OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    seagrass_required_true(coral_array_list_set_length(
            &object->queues, concurrency));
    for (uintmax_t i = 0; i < concurrency; i++) {
        struct octopus_linked_queue *item;
        seagrass_required_true(coral_array_list_get(
                &object->queues, i, (void **) &item));
        if (!octopus_linked_queue_init(item, size)) {
            uintmax_t error;
            switch (octopus_error) {
                default: {
                    seagrass_required_true(false);
                }
                case OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE: {
                    error =
                            OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE;
                    break;
                }
                case OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED: {
                    error =
                            OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
                    break;
                }
            }
            for (uintmax_t o = 0; o <= i; o++) {
                struct octopus_linked_queue *queue;
                seagrass_required_true(coral_array_list_get(
                        &object->queues, o, (void **) &queue));
                seagrass_required_true(octopus_linked_queue_invalidate(
                        queue, NULL));
            }
            seagrass_required_true(coral_array_list_invalidate(
                    &object->queues, NULL));
            *object = (struct octopus_concurrent_linked_queue) {0};
            octopus_error = error;
            return false;
        }
    }
    return true;
}

bool octopus_concurrent_linked_queue_invalidate(
        struct octopus_concurrent_linked_queue *const object,
        void (*const on_destroy)(void *)) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    uintmax_t count;
    seagrass_required_true(coral_array_list_get_length(
            &object->queues, &count));
    if (count) {
        void *out;
        while (remove(object, &out)) {
            if (on_destroy) {
                on_destroy(out);
            }
        }
        seagrass_required_true(
                OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY
                == octopus_error);
    }
    seagrass_required_true(coral_array_list_invalidate(
            &object->queues, NULL));
    *object = (struct octopus_concurrent_linked_queue) {0};
    return true;
}

bool octopus_concurrent_linked_queue_size(
        const struct octopus_concurrent_linked_queue *const object,
        size_t *const out) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    const struct octopus_linked_queue *queue;
    seagrass_required_true(coral_array_list_get(
            &object->queues, 0, (void **) &queue));
    seagrass_required_true(octopus_linked_queue_size(queue, out));
    return true;
}

bool octopus_concurrent_linked_queue_concurrency(
        const struct octopus_concurrent_linked_queue *const object,
        uintmax_t *const out) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    concurrency(object, out);
    return true;
}

bool octopus_concurrent_linked_queue_add(
        struct octopus_concurrent_linked_queue *const object,
        const void *const item) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!item) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_ITEM_IS_NULL;
        return false;
    }
    uintmax_t c;
    concurrency(object, &c);
    const uintmax_t begin = atomic_fetch_add(&object->enqueue, 1);
    uintmax_t at;
    seagrass_required_true(seagrass_uintmax_t_divide(
            begin, c, &at, NULL));
    struct octopus_linked_queue *queue;
    seagrass_required_true(coral_array_list_get(
            &object->queues, at, (void **) &queue));
    const bool result = octopus_linked_queue_add(queue, item);
    if (!result) {
        seagrass_required_true(
                OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED
                == octopus_error);
        octopus_error =
                OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    return result;
}

bool octopus_concurrent_linked_queue_remove(
        struct octopus_concurrent_linked_queue *const object,
        void **const out) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    return remove(object, out);
}

bool octopus_concurrent_linked_queue_peek(
        struct octopus_concurrent_linked_queue *const object,
        void **const out) {
    if (!object) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    uintmax_t c;
    concurrency(object, &c);
    const uintmax_t begin = atomic_load(&object->dequeue);
    const uintmax_t end = begin + c; /* allow integer overflow */
    uintmax_t at = begin;
    for (; (begin < end && at < end)
           || ((begin > end && at >= begin) || at < end); at++) {
        if (retrieve(object, c, at, out, octopus_linked_queue_peek)) {
            return true;
        }
    }
    if (retrieve(object, c, at, out, octopus_linked_queue_peek)) {
        return true;
    }
    octopus_error = OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY;
    return false;
}
