#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <errno.h>
#include <octopus.h>

#include "private/linked_queue.h"

#include <test/cmocka.h>

static void check_invalidate_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_invalidate(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_invalidate(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object = {};
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_invalidate_case_mutexes_are_busy(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object = {};
    pthread_mutex_destroy_is_overridden = true;
    will_return_count(cmocka_test_pthread_mutex_destroy, EBUSY, 2);
    will_return(cmocka_test_pthread_mutex_destroy, 0);
    will_return_count(cmocka_test_pthread_mutex_destroy, EBUSY, 5);
    will_return(cmocka_test_pthread_mutex_destroy, 0);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    pthread_mutex_destroy_is_overridden = false;
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_init(NULL, 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_size_is_zero(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_init((void *) 1, 0));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_ZERO,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_size_is_too_large(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_false(octopus_linked_queue_init(&object, SIZE_MAX));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_SIZE_IS_TOO_LARGE,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_init_error_on_memory_allocation_failed(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    pthread_mutex_init_is_overridden = true;
    will_return(cmocka_test_pthread_mutex_init, 0);
    will_return(cmocka_test_pthread_mutex_init, ENOMEM);
    pthread_mutex_destroy_is_overridden = true;
    will_return(cmocka_test_pthread_mutex_destroy, 0);
    assert_false(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    pthread_mutex_destroy_is_overridden = false;
    pthread_mutex_init_is_overridden = false;
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_size(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_size((void *) 1, NULL));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_size(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object = {
            .queue.size = rand() % UINTMAX_MAX
    };
    uintmax_t out;
    assert_true(octopus_linked_queue_size(&object, &out));
    assert_int_equal(out, object.queue.size);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_count_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_count(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_count_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_count((void *) 1, NULL));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_count(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t out;
    assert_true(octopus_linked_queue_count(&object, &out));
    assert_int_equal(out, object.queue.count);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_add(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_item_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_add((void *) 1, NULL));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_ITEM_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t count;
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 0);
    const uintmax_t item = rand() % UINTMAX_MAX;
    assert_true(octopus_linked_queue_add(&object, &item));
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 1);
    uintmax_t value;
    /* bypass locks to check contents of linked queue */
    assert_true(coral_linked_queue_peek(&object.queue, (void **) &value));
    assert_int_equal(value, item);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_add_error_on_memory_allocation_failed(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    const uintmax_t item = rand() % UINTMAX_MAX;
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = true;
    assert_false(octopus_linked_queue_add(&object, &item));
    malloc_is_overridden = calloc_is_overridden = realloc_is_overridden
            = posix_memalign_is_overridden = false;
    assert_int_equal(
            OCTOPUS_LINKED_QUEUE_ERROR_MEMORY_ALLOCATION_FAILED,
            octopus_error);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_remove(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_remove((void *) 1, NULL));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t count;
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 0);
    const uintmax_t item = rand() % UINTMAX_MAX;
    assert_true(octopus_linked_queue_add(&object, &item));
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 1);
    uintmax_t out;
    assert_true(octopus_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(out, item);
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 0);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_remove_error_on_queue_is_empty(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t out;
    assert_false(octopus_linked_queue_remove(&object, (void **) &out));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_object_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_peek(NULL, (void *) 1));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OBJECT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_out_is_null(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    assert_false(octopus_linked_queue_peek((void *) 1, NULL));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_OUT_IS_NULL,
                     octopus_error);
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek(void **state) {
    srand(time(NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t count;
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 0);
    const uintmax_t item = rand() % UINTMAX_MAX;
    assert_true(octopus_linked_queue_add(&object, &item));
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 1);
    uintmax_t out;
    assert_true(octopus_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(out, item);
    assert_true(octopus_linked_queue_count(&object, &count));
    assert_int_equal(count, 1);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

static void check_peek_error_on_queue_is_empty(void **state) {
    octopus_error = OCTOPUS_ERROR_NONE;
    struct octopus_linked_queue object;
    assert_true(octopus_linked_queue_init(&object, sizeof(uintmax_t)));
    uintmax_t out;
    assert_false(octopus_linked_queue_peek(&object, (void **) &out));
    assert_int_equal(OCTOPUS_LINKED_QUEUE_ERROR_QUEUE_IS_EMPTY,
                     octopus_error);
    assert_true(octopus_linked_queue_invalidate(&object, NULL));
    octopus_error = OCTOPUS_ERROR_NONE;
}

int main(int argc, char *argv[]) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(check_invalidate_error_on_object_is_null),
            cmocka_unit_test(check_invalidate),
            cmocka_unit_test(check_invalidate_case_mutexes_are_busy),
            cmocka_unit_test(check_init_error_on_object_is_null),
            cmocka_unit_test(check_init_error_on_size_is_zero),
            cmocka_unit_test(check_init_error_on_size_is_too_large),
            cmocka_unit_test(check_init),
            cmocka_unit_test(check_init_error_on_memory_allocation_failed),
            cmocka_unit_test(check_size_error_on_object_is_null),
            cmocka_unit_test(check_size_error_on_out_is_null),
            cmocka_unit_test(check_size),
            cmocka_unit_test(check_count_error_on_object_is_null),
            cmocka_unit_test(check_count_error_on_out_is_null),
            cmocka_unit_test(check_count),
            cmocka_unit_test(check_add_error_on_object_is_null),
            cmocka_unit_test(check_add_error_on_item_is_null),
            cmocka_unit_test(check_add),
            cmocka_unit_test(check_add_error_on_memory_allocation_failed),
            cmocka_unit_test(check_remove_error_on_object_is_null),
            cmocka_unit_test(check_remove_error_on_out_is_null),
            cmocka_unit_test(check_remove),
            cmocka_unit_test(check_remove_error_on_queue_is_empty),
            cmocka_unit_test(check_peek_error_on_object_is_null),
            cmocka_unit_test(check_peek_error_on_out_is_null),
            cmocka_unit_test(check_peek),
            cmocka_unit_test(check_peek_error_on_queue_is_empty),
    };
    //cmocka_set_message_output(CM_OUTPUT_XML);
    return cmocka_run_group_tests(tests, NULL, NULL);
}
