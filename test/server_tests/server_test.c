//
// Created by vladkanash on 6.11.16.
//

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

/* A test case that does nothing and succeeds. */
static void null_test_success(void **state) {
    (void) state; /* unused */
    assert_false(0);
}

static void null_test_error(void **state) {
    assert_false(1);
}

int main(void) {
    const struct CMUnitTest tests[] = {
            cmocka_unit_test(null_test_success),
            cmocka_unit_test(null_test_error),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}