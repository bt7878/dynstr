#include "../src/dynstr.h"

#include <cmocka.h>
#include <string.h>

void test_create_empty(void **state) {
    (void) state;
    dynstr_t s;
    assert_true(dynstr_create(&s, ""));
    assert_int_equal(dynstr_len(&s), 0);
    assert_int_equal(dynstr_cap(&s), DYNSTR_SSO_CAP);
    assert_string_equal(dynstr_cstr(&s), "");
    dynstr_destroy(&s);
}

void test_create_sso(void **state) {
    (void) state;
    dynstr_t s;
    const char *sso_str = "0123456789ABCDE"; // 15 chars
    assert_true(dynstr_create(&s, sso_str));
    assert_int_equal(dynstr_len(&s), 15);
    assert_int_equal(dynstr_cap(&s), DYNSTR_SSO_CAP);
    assert_string_equal(dynstr_cstr(&s), sso_str);
    dynstr_destroy(&s);
}

void test_create_heap(void **state) {
    (void) state;
    dynstr_t s;
    const char *heap_str = "0123456789ABCDEF"; // 16 chars
    assert_true(dynstr_create(&s, heap_str));
    assert_int_equal(dynstr_len(&s), 16);
    assert_int_equal(dynstr_cap(&s), 31);
    assert_string_equal(dynstr_cstr(&s), heap_str);
    dynstr_destroy(&s);
}

void test_create_large_heap(void **state) {
    (void) state;
    dynstr_t s;
    const char *large_str =
            "This is a much longer string that should definitely be on the heap and require a larger capacity.";
    assert_true(dynstr_create(&s, large_str));
    assert_int_equal(dynstr_len(&s), strlen(large_str));
    assert_true(dynstr_cap(&s) >= strlen(large_str));
    assert_string_equal(dynstr_cstr(&s), large_str);
    dynstr_destroy(&s);
}

void test_destroy_null(void **state) {
    (void) state;
    // NULL destroy should be safe
    dynstr_destroy(NULL);
}

void test_clone_sso(void **state) {
    (void) state;
    dynstr_t s1, s2;

    // SSO clone
    dynstr_create(&s1, "SSO String");
    assert_true(dynstr_clone(&s2, &s1));
    assert_int_equal(dynstr_len(&s1), dynstr_len(&s2));
    assert_int_equal(dynstr_cap(&s1), dynstr_cap(&s2));
    assert_string_equal(dynstr_cstr(&s1), dynstr_cstr(&s2));
    assert_ptr_not_equal(dynstr_cstr(&s1), dynstr_cstr(&s2));
    dynstr_destroy(&s1);
    dynstr_destroy(&s2);
}

void test_clone_heap(void **state) {
    (void) state;
    dynstr_t s1, s2;

    // Heap clone
    dynstr_create(&s1, "This is a heap string because it exceeds the SSO limit of 15 characters.");
    assert_true(dynstr_clone(&s2, &s1));
    assert_int_equal(dynstr_len(&s1), dynstr_len(&s2));
    assert_int_equal(dynstr_cap(&s1), dynstr_cap(&s2));
    assert_string_equal(dynstr_cstr(&s1), dynstr_cstr(&s2));
    assert_ptr_not_equal(dynstr_cstr(&s1), dynstr_cstr(&s2));
    dynstr_destroy(&s1);
    dynstr_destroy(&s2);
}

void test_clear_sso(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Some content");
    dynstr_clear(&s);
    assert_int_equal(dynstr_len(&s), 0);
    assert_string_equal(dynstr_cstr(&s), "");
    // Capacity should remain same
    assert_int_equal(dynstr_cap(&s), DYNSTR_SSO_CAP);
    dynstr_destroy(&s);
}

void test_clear_heap(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "This is a long heap string for clearing.");
    const size_t prev_cap = dynstr_cap(&s);
    dynstr_clear(&s);
    assert_int_equal(dynstr_len(&s), 0);
    assert_string_equal(dynstr_cstr(&s), "");
    assert_int_equal(dynstr_cap(&s), prev_cap);
    dynstr_destroy(&s);
}

void test_reserve_smaller(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small"); // len 5, cap 15
    // Reserve less than current
    assert_true(dynstr_reserve(&s, 10));
    assert_int_equal(dynstr_cap(&s), 15);
    dynstr_destroy(&s);
}

void test_reserve_sso_to_heap(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small"); // len 5, cap 15
    // Reserve to Heap
    assert_true(dynstr_reserve(&s, 20));
    assert_int_equal(dynstr_cap(&s), 31);
    assert_string_equal(dynstr_cstr(&s), "Small");
    dynstr_destroy(&s);
}

void test_reserve_heap_to_larger_heap(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small");
    dynstr_reserve(&s, 20);
    // Reserve larger Heap
    assert_true(dynstr_reserve(&s, 100));
    assert_int_equal(dynstr_cap(&s), 127);
    dynstr_destroy(&s);
}

void test_reserve_above_max_cap(void **state) {
    (void) state;
    dynstr_t s;
    const char *initial_str = "Initial string content";
    assert_true(dynstr_create(&s, initial_str));
    const size_t initial_len = dynstr_len(&s);
    const size_t initial_cap = dynstr_cap(&s);
    const char *initial_ptr = dynstr_cstr(&s);
    // Should fail to reserve more than DYNSTR_MAX_CAP
    assert_false(dynstr_reserve(&s, DYNSTR_MAX_CAP + 1));
    // Ensure state is unchanged
    assert_int_equal(dynstr_len(&s), initial_len);
    assert_int_equal(dynstr_cap(&s), initial_cap);
    assert_string_equal(dynstr_cstr(&s), initial_str);
    assert_ptr_equal(dynstr_cstr(&s), initial_ptr);
    dynstr_destroy(&s);
}

void test_shrink_to_fit_heap_to_sso(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small");
    dynstr_reserve(&s, 100);
    // Shrink to fit (should go back to SSO since len is 5)
    assert_true(dynstr_shrink_to_fit(&s));
    assert_int_equal(dynstr_cap(&s), 15);
    assert_string_equal(dynstr_cstr(&s), "Small");
    dynstr_destroy(&s);
}

void test_shrink_to_fit_heap_to_smaller_heap(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small");
    dynstr_reserve(&s, 100);
    dynstr_append(&s, " and now it is longer but not that long."); // len 5 + 40 = 45
    assert_int_equal(dynstr_cap(&s), 127);
    assert_true(dynstr_shrink_to_fit(&s));
    assert_int_equal(dynstr_cap(&s), 63); // min_cap(45) -> 63
    assert_int_equal(dynstr_len(&s), 45);
    dynstr_destroy(&s);
}

void test_push(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "H");
    assert_true(dynstr_push(&s, 'e'));
    assert_true(dynstr_push(&s, 'l'));
    assert_true(dynstr_push(&s, 'l'));
    assert_true(dynstr_push(&s, 'o'));
    assert_string_equal(dynstr_cstr(&s), "Hello");
    dynstr_destroy(&s);
}

void test_append_sso(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Hello");
    assert_true(dynstr_append(&s, " world"));
    assert_string_equal(dynstr_cstr(&s), "Hello world");
    assert_int_equal(dynstr_len(&s), 11);
    dynstr_destroy(&s);
}

void test_append_transition_to_heap(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Hello world");
    assert_true(dynstr_append(&s, "! This string is now on the heap."));
    assert_int_equal(dynstr_len(&s), 11 + 33);
    assert_true(dynstr_cap(&s) > 15);
    dynstr_destroy(&s);
}

void test_pop(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Hello world! This string is now on the heap.");
    dynstr_pop(&s); // Remove '.'
    assert_int_equal(dynstr_len(&s), 43);
    assert_int_equal(dynstr_cstr(&s)[42], 'p');
    dynstr_destroy(&s);
}

void test_pop_until_sso(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Hello world! This string is now on the heap.");
    // Pop until SSO
    while (dynstr_len(&s) > 15) {
        dynstr_pop(&s);
    }
    // Note: pop does not shrink capacity automatically
    assert_true(dynstr_cap(&s) > 15);
    assert_int_equal(dynstr_len(&s), 15);
    dynstr_destroy(&s);
}

void test_pop_empty(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "");
    dynstr_pop(&s);
    assert_int_equal(dynstr_len(&s), 0);
    dynstr_destroy(&s);
}

void test_at(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "ABC");
    *dynstr_at(&s, 0) = 'X';
    *dynstr_at(&s, 1) = 'Y';
    *dynstr_at(&s, 2) = 'Z';
    assert_string_equal(dynstr_cstr(&s), "XYZ");
    dynstr_destroy(&s);
}

void test_insert_bounds(void **state) {
    (void) state;
    dynstr_t s;
    assert_true(dynstr_create(&s, "Hello")); // len 5
    assert_true(dynstr_insert(&s, 0, "[")); // OK: "[Hello" (len 6)
    assert_true(dynstr_insert(&s, 6, "]")); // OK: "[Hello]" (len 7)
    assert_true(dynstr_insert(&s, 3, "---")); // OK: "[He---llo]" (len 10)
    assert_false(dynstr_insert(&s, 11, "X")); // Out of bounds
    dynstr_destroy(&s);
}

void test_remove_bounds(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "[He---llo]");
    assert_false(dynstr_remove(&s, 10, 1)); // Out-of-bounds index
    assert_false(dynstr_remove(&s, 5, 6)); // Range out of bounds
    assert_true(dynstr_remove(&s, 2, 0)); // OK: No-op remove
    assert_int_equal(dynstr_len(&s), 10);
    assert_true(dynstr_remove(&s, 3, 3)); // OK: "[Hello]" (len 7)
    assert_string_equal(dynstr_cstr(&s), "[Hello]");
    assert_true(dynstr_remove(&s, 0, 7)); // OK: "" (len 0)
    assert_int_equal(dynstr_len(&s), 0);
    dynstr_destroy(&s);
}

void test_substr_bounds(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Original"); // len 8
    dynstr_t sub;
    assert_true(dynstr_substr(&sub, &s, 0, 8)); // OK: "Original"
    assert_string_equal(dynstr_cstr(&sub), "Original");
    dynstr_destroy(&sub);

    assert_true(dynstr_substr(&sub, &s, 2, 0)); // OK: Empty substring
    assert_int_equal(dynstr_len(&sub), 0);
    assert_string_equal(dynstr_cstr(&sub), "");
    dynstr_destroy(&sub);

    assert_true(dynstr_substr(&sub, &s, 2, 4)); // OK: "igin"
    assert_string_equal(dynstr_cstr(&sub), "igin");
    dynstr_destroy(&sub);

    assert_false(dynstr_substr(&sub, &s, 0, 9)); // Too long
    assert_false(dynstr_substr(&sub, &s, 9, 0)); // Out-of-bounds index
    assert_false(dynstr_substr(&sub, &s, 5, 4)); // pos+len > total_len (5+4=9 > 8)

    dynstr_destroy(&s);
}

void test_sso_to_heap_and_back(void **state) {
    (void) state;
    dynstr_t s;
    dynstr_create(&s, "Small"); // SSO
    assert_int_equal(dynstr_cap(&s), 15);

    // Force heap
    for (int i = 0; i < 20; ++i) {
        dynstr_push(&s, 'a');
    }
    assert_int_equal(dynstr_len(&s), 25);
    assert_int_equal(dynstr_cap(&s), 31);

    // Force SSO via shrink
    while (dynstr_len(&s) > 5) {
        dynstr_pop(&s);
    }
    assert_true(dynstr_shrink_to_fit(&s));
    assert_int_equal(dynstr_cap(&s), 15);
    assert_string_equal(dynstr_cstr(&s), "Small");

    dynstr_destroy(&s);
}

int main() {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_create_empty),
        cmocka_unit_test(test_create_sso),
        cmocka_unit_test(test_create_heap),
        cmocka_unit_test(test_create_large_heap),
        cmocka_unit_test(test_destroy_null),
        cmocka_unit_test(test_clone_sso),
        cmocka_unit_test(test_clone_heap),
        cmocka_unit_test(test_clear_sso),
        cmocka_unit_test(test_clear_heap),
        cmocka_unit_test(test_reserve_smaller),
        cmocka_unit_test(test_reserve_sso_to_heap),
        cmocka_unit_test(test_reserve_heap_to_larger_heap),
        cmocka_unit_test(test_shrink_to_fit_heap_to_sso),
        cmocka_unit_test(test_reserve_above_max_cap),
        cmocka_unit_test(test_shrink_to_fit_heap_to_smaller_heap),
        cmocka_unit_test(test_push),
        cmocka_unit_test(test_append_sso),
        cmocka_unit_test(test_append_transition_to_heap),
        cmocka_unit_test(test_pop),
        cmocka_unit_test(test_pop_until_sso),
        cmocka_unit_test(test_pop_empty),
        cmocka_unit_test(test_at),
        cmocka_unit_test(test_insert_bounds),
        cmocka_unit_test(test_remove_bounds),
        cmocka_unit_test(test_substr_bounds),
        cmocka_unit_test(test_sso_to_heap_and_back),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
