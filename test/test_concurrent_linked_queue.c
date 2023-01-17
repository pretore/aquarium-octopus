#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <octopus.h>

#include "private/linked_queue.h"

#include <test/cmocka.h>
#include "test/linked_queue.h"

static void check_invalidate_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_invalidate(
            NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_invalidate(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object = {};
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_init(
            NULL, sizeof(uintmax_t), 8));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_size_is_zero(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_init(
            (void *) 1, 0, 8));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_SIZE_IS_ZERO,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_size_is_too_large(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_false(octopus_concurrent_linked_queue_init(
            &object, SIZE_MAX, 8));
    assert_int_equal(
            OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE,
            octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_concurrent_is_zero(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_init(
            (void *) 1, sizeof(uintmax_t), 0));
    assert_int_equal(
            OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_CONCURRENCY_IS_ZERO,
            octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    assert_int_equal(atomic_load(&object.enqueue), 0);
    assert_int_equal(atomic_load(&object.dequeue), 0);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_memory_allocation_failed(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = true;
    assert_false(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = false;
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_size(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_size((void *) 1, NULL));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    const uintmax_t check = 1 + (rand() % UINT8_MAX);
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, check, 8));
    size_t out;
    assert_true(octopus_concurrent_linked_queue_size(&object, &out));
    assert_int_equal(out, check);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_concurrency_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_concurrency(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_concurrency_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_concurrency((void *) 1, NULL));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_concurrency(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    const uintmax_t check = 1 + (rand() % UINT8_MAX);
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), check));
    size_t out;
    assert_true(octopus_concurrent_linked_queue_concurrency(&object, &out));
    assert_int_equal(out, check);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_add(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_add((void *) 1, NULL));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_ITEM_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = 64; /* concurrency ^ 2 */
    for (uintmax_t i = 0; i < check; i++) {
        const uintmax_t value = rand();
        assert_true(octopus_concurrent_linked_queue_add(&object, &value));
    }
    assert_int_equal(atomic_load(&object.enqueue), check);
    for (uintmax_t i = 0; i < 8; i++) {
        struct octopus_linked_queue *queue;
        assert_true(coral_array_list_get(
                &object.queues, i, (void **) &queue));
        uintmax_t count;
        assert_true(octopus_linked_queue_count(queue, &count));
        assert_int_equal(count, 8);
    }
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_memory_allocation_failed(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = true;
    const uintmax_t value = rand();
    assert_false(octopus_concurrent_linked_queue_add(&object, &value));
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = false;
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_remove(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_remove((void *) 1, NULL));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_queue_is_empty(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(void *), 8));
    assert_false(octopus_concurrent_linked_queue_remove(&object, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_case_enqueue_dequeue_aligned(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 1);
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 1);
    assert_int_equal(out, check);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_case_enqueue_dequeue_misaligned(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 1);
    atomic_store(&object.dequeue, 1); /* misaligned */
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 9);
    assert_int_equal(out, check);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_case_enqueue_dequeue_integer_overflow(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    atomic_store(&object.enqueue, 2);
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 3);
    atomic_store(&object.dequeue, UINTMAX_MAX);
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 1);
    assert_int_equal(out, check);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_peek(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_concurrent_linked_queue_peek((void *) 1, NULL));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_queue_is_empty(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(void *), 8));
    assert_false(octopus_concurrent_linked_queue_peek(&object, (void *) 1));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_case_enqueue_dequeue_aligned(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 1);
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 0);
    assert_int_equal(out, check);
    out = ~out;
    assert_true(octopus_concurrent_linked_queue_add(&object, &out));
    assert_int_equal(atomic_load(&object.enqueue), 2);
    assert_true(octopus_concurrent_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 0);
    assert_int_equal(out, check);
    out = ~out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 1);
    assert_int_equal(out, check);
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 9);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_case_enqueue_dequeue_misaligned(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 1);
    atomic_store(&object.dequeue, 1); /* misaligned */
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 1);
    assert_int_equal(out, check);
    out = ~out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 9);
    assert_int_equal(out, check);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_case_enqueue_dequeue_integer_overflow(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_concurrent_linked_queue object;
    assert_true(octopus_concurrent_linked_queue_init(
            &object, sizeof(uintmax_t), 8));
    uintmax_t check = rand() % UINTMAX_MAX;
    atomic_store(&object.enqueue, 2);
    assert_true(octopus_concurrent_linked_queue_add(&object, &check));
    assert_int_equal(atomic_load(&object.enqueue), 3);
    atomic_store(&object.dequeue, UINTMAX_MAX);
    uintmax_t out;
    assert_true(octopus_concurrent_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), UINTMAX_MAX);
    assert_int_equal(out, check);
    out = ~out;
    assert_true(octopus_concurrent_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(atomic_load(&object.dequeue), 1);
    assert_int_equal(out, check);
    assert_false(octopus_concurrent_linked_queue_remove(
            &object, (void **) &out));
    assert_int_equal(OCTOPUS_CONCURRENT_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_concurrent_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(check_invalidate_error_on_object_is_null),
            cmocka_unit_test(check_invalidate),
            cmocka_unit_test(check_init_error_on_object_is_null),
            cmocka_unit_test(check_init_error_on_size_is_zero),
            cmocka_unit_test(check_init_error_on_size_is_too_large),
            cmocka_unit_test(check_init_error_on_concurrent_is_zero),
            cmocka_unit_test(check_init),
            cmocka_unit_test(check_init_error_on_memory_allocation_failed),
            cmocka_unit_test(check_size_error_on_object_is_null),
            cmocka_unit_test(check_size_error_on_out_is_null),
            cmocka_unit_test(check_size),
            cmocka_unit_test(check_concurrency_error_on_object_is_null),
            cmocka_unit_test(check_concurrency_error_on_out_is_null),
            cmocka_unit_test(check_concurrency),
            cmocka_unit_test(check_add_error_on_object_is_null),
            cmocka_unit_test(check_add_error_on_out_is_null),
            cmocka_unit_test(check_add),
            cmocka_unit_test(check_add_error_on_memory_allocation_failed),
            cmocka_unit_test(check_remove_error_on_object_is_null),
            cmocka_unit_test(check_remove_error_on_out_is_null),
            cmocka_unit_test(check_remove_error_on_queue_is_empty),
            cmocka_unit_test(check_remove_case_enqueue_dequeue_aligned),
            cmocka_unit_test(check_remove_case_enqueue_dequeue_misaligned),
            cmocka_unit_test(check_remove_case_enqueue_dequeue_integer_overflow),
            cmocka_unit_test(check_peek_error_on_object_is_null),
            cmocka_unit_test(check_peek_error_on_out_is_null),
            cmocka_unit_test(check_peek_error_on_queue_is_empty),
            cmocka_unit_test(check_peek_case_enqueue_dequeue_aligned),
            cmocka_unit_test(check_peek_case_enqueue_dequeue_misaligned),
            cmocka_unit_test(check_peek_case_enqueue_dequeue_integer_overflow),
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
