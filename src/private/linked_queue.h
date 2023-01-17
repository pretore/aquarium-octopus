#ifndef _OCTOPUS_PRIVATE_LINKED_QUEUE_H_
#define _OCTOPUS_PRIVATE_LINKED_QUEUE_H_

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <coral.h>

#define OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL                1
#define OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_ZERO                  2
#define OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE             3
#define OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED      4
#define OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL                   5
#define OCTOPUS_LINKED_QUEUE_ERROR_ITEM_IS_NULL                  6
#define OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY                7

struct octopus_linked_queue {
    struct coral_linked_queue queue;
    pthread_mutex_t enqueue;
    pthread_mutex_t dequeue;
};

/**
 * @brief Initialize linked queue.
 * @param [in] object instance to be initialized.
 * @param [in] size of item to be contained within the queue.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_ZERO if size is zero.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE if size is too
 * large.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED if there is
 * insufficient memory to initialize instance.
 */
bool octopus_linked_queue_init(
        struct octopus_linked_queue *object,
        size_t size);

/**
 * @brief Invalidate linked queue.
 * <p>All the items contained within the queue will have the given <i>on
 * destroy</i> callback invoked upon itself. The actual <u>queue instance
 * is not deallocated</u> since it may have been embedded in a larger
 * structure.</p>
 * @param [in] object instance to be invalidated.
 * @param [in] on_destroy called just before the item is to be destroyed.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 */
bool octopus_linked_queue_invalidate(
        struct octopus_linked_queue *object,
        void (*on_destroy)(void *));

/**
 * @brief Retrieve the size of an item.
 * @param [in] object queue instance.
 * @param [out] out receive the size of an item.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool octopus_linked_queue_size(
        const struct octopus_linked_queue *object,
        size_t *out);

/**
 * @brief Add item to the end of the queue.
 * @param [in] object queue instance.
 * @param [in] item to add to the end of the queue.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_ITEM_IS_NULL if item is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED if there is
 * insufficient memory to add item.
 */
bool octopus_linked_queue_add(struct octopus_linked_queue *object,
                              const void *item);

/**
 * @brief Remove item from the front of the queue.
 * @param [in] object queue instance.
 * @param [in] out receive the item in the front of the queue.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY if queue is empty.
 */
bool octopus_linked_queue_remove(struct octopus_linked_queue *object,
                                 void **out);

/**
 * @brief Retrieve the item from the front of the queue without removing it.
 * @param [in] object queue instance.
 * @param [in] out receive the item in the front of the queue without
 * removing it.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY if queue is empty.
 */
bool octopus_linked_queue_peek(struct octopus_linked_queue *object,
                               void **out);

#endif /* _OCTOPUS_PRIVATE_LINKED_QUEUE_H_ */
