#ifndef _OCTOPUS_TEST_LINKED_QUEUE_H_
#define _OCTOPUS_TEST_LINKED_QUEUE_H_
#ifdef TEST

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

struct octopus_linked_queue;

/**
 * @brief Retrieve the count of items.
 * @param [in] object instance whose count we are to retrieve.
 * @param [out] out receive the count.
 * @return On success true, otherwise false if an error has occurred.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL if object is <i>NULL</i>.
 * @throws OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL if out is <i>NULL</i>.
 */
bool octopus_linked_queue_count(
        struct octopus_linked_queue *object,
        uintmax_t *out);

#endif /* TEST */
#endif /* _OCTOPUS_TEST_LINKED_QUEUE_H_ */
