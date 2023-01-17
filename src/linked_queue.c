#include <stdlib.h>
#include <assert.h>
#include <seagrass.h>
#include <errno.h>
#include <octopus.h>

#include "private/linked_queue.h"

#ifdef TEST
#include <test/cmocka.h>
#endif

bool octopus_linked_queue_init(
        struct octopus_linked_queue *const object,
        const size_t size) {
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!size) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_ZERO;
        return false;
    }
    *object = (struct octopus_linked_queue) {0};
    if (!coral_linked_queue_init(&object->queue, size)) {
        seagrass_required_true(CORAL_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE
                               == coral_error);
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE;
        return false;
    }
    int error;
    if ((error = pthread_mutex_init(&object->dequeue, NULL))) {
        seagrass_required_true(ENOMEM == error);
        seagrass_required_true(coral_linked_queue_invalidate(
                &object->queue, NULL));
        octopus_error =
                OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    if ((error = pthread_mutex_init(&object->enqueue, NULL))) {
        seagrass_required_true(ENOMEM == error);
        seagrass_required_true(!pthread_mutex_destroy(&object->dequeue));
        seagrass_required_true(coral_linked_queue_invalidate(
                &object->queue, NULL));
        octopus_error =
                OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
        return false;
    }
    return true;
}

bool octopus_linked_queue_invalidate(
        struct octopus_linked_queue *const object,
        void (*const on_destroy)(void *)) {
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    pthread_mutex_t *locks[] = {
            &object->dequeue, &object->enqueue
    };
    const uintmax_t limit = sizeof(locks) / sizeof(pthread_mutex_t *);
    for (uintmax_t i = 0; i < limit; i++) {
        do {
            int error;
            if ((error = pthread_mutex_destroy(locks[i]))) {
                if (EINVAL == error) {
                    break;
                }
                seagrass_required_true(EBUSY == error);
                const struct timespec delay = {
                        .tv_nsec = 1000
                };
                seagrass_required_true(!nanosleep(&delay, NULL)
                                       || errno == EINTR);
                continue;
            }
            break;
        } while (true);
    }
    seagrass_required_true(coral_linked_queue_invalidate(
            &object->queue, on_destroy));
    *object = (struct octopus_linked_queue) {0};
    return true;
}

bool octopus_linked_queue_size(
        const struct octopus_linked_queue *const object,
        size_t *const out) {
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    seagrass_required_true(coral_linked_queue_size(
            &object->queue, out));
    return true;
}

#ifdef TEST
bool octopus_linked_queue_count(
        struct octopus_linked_queue *const object,
        uintmax_t *const out) {
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    seagrass_required_true(!pthread_mutex_lock(&object->dequeue));
    seagrass_required_true(!pthread_mutex_lock(&object->enqueue));
    seagrass_required_true(coral_linked_queue_count(
            &object->queue, out));
    seagrass_required_true(!pthread_mutex_unlock(&object->enqueue));
    seagrass_required_true(!pthread_mutex_unlock(&object->dequeue));
    return true;
}
#endif /* TEST */

bool octopus_linked_queue_add(
        struct octopus_linked_queue *const object,
        const void *const item) {
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!item) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_ITEM_IS_NULL;
        return false;
    }
    seagrass_required_true(!pthread_mutex_lock(&object->enqueue));
    const bool result = coral_linked_queue_add(&object->queue, item);
    if (!result) {
        seagrass_required_true(CORAL_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED
                               == coral_error);
        octopus_error =
                OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED;
    }
    seagrass_required_true(!pthread_mutex_unlock(&object->enqueue));
    return result;
}

static bool retrieve(struct octopus_linked_queue *const object,
                     void **const out,
                     bool (*func)(struct coral_linked_queue *, void **)) {
    assert(func);
    if (!object) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL;
        return false;
    }
    if (!out) {
        octopus_error = OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL;
        return false;
    }
    seagrass_required_true(!pthread_mutex_lock(&object->dequeue));
    uintmax_t count;
    seagrass_required_true(coral_linked_queue_count(
            &object->queue, &count));
    bool result = true;
    if (1 >= count) {
        seagrass_required_true(!pthread_mutex_lock(&object->enqueue));
        if (!(result = func(&object->queue, out))) {
            seagrass_required_true(CORAL_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY
                                   == coral_error);
            octopus_error =
                    OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY;
        }
        seagrass_required_true(!pthread_mutex_unlock(&object->enqueue));
    } else {
        seagrass_required_true(func(&object->queue, out));
    }
    seagrass_required_true(!pthread_mutex_unlock(&object->dequeue));
    return result;
}

bool octopus_linked_queue_remove(
        struct octopus_linked_queue *const object,
        void **const out) {
    return retrieve(object, out, coral_linked_queue_remove);
}

bool octopus_linked_queue_peek(
        struct octopus_linked_queue *const object,
        void **const out) {
    return retrieve(object, out,
                    (bool (*)(struct coral_linked_queue *, void **))
                            coral_linked_queue_peek);
}
