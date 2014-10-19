#include "../../array.h"
#include "unit.h"

static bool ensure_size_on_empty_array_increases_capacity_to_one(char** msg) {
    const size_t EXPECTED = 1;

    array a;
    array_init(&a, 0);
    array_ensure_size(&a);

    size_t actual = a.cap;
    ASSERT_INT_EQUAL(EXPECTED, actual,
                     "The array's capacity didn't increase to 1.", msg);

    array_free(&a);
    return true;
}

static bool ensure_size_on_non_empty_array_doubles_capacity(char** msg) {
    const size_t EXPECTED = 4;

    array a;
    array_init(&a, 0);
    a.len = 2;
    a.cap = 2;
    array_ensure_size(&a);

    size_t actual = a.cap;
    ASSERT_INT_EQUAL(EXPECTED, actual, "The array's capacity wasn't doubled.",
                     msg);

    array_free(&a);
    return true;
}

static bool append_increases_array_length_by_one(char** msg) {
    const size_t EXPECTED = 1;

    array a;
    array_init(&a, 0);
    array_append(&a);

    size_t actual = a.len;
    ASSERT_INT_EQUAL(EXPECTED, actual, "The array's length wasn't increased.",
                     msg);

    array_free(&a);
    return true;
}

static bool copy_copies_array_with_no_elements(char** msg) {
    array src;
    array_init(&src, sizeof(char));

    array dst;
    array_copy(&dst, &src);

    ASSERT_INT_EQUAL(src.len, dst.len,
                     "The copied array's length was incorrect.", msg);
    ASSERT_INT_EQUAL(src.cap, dst.cap,
                     "The copied array's capacity was incorrect.", msg);
    ASSERT_INT_EQUAL(src.size, dst.size,
                     "The copied array's size was incorrect.", msg);

    char* expected_data = (char*)src.data;
    char* actual_data = (char*)dst.data;
    int data_equal = (memcmp(expected_data, actual_data, src.len) == 0);
    ASSERT_CONDITION(data_equal, expected_data, actual_data,
                     "The copied data wasn't equal to the source data.", msg);

    array_free(&src);
    array_free(&dst);
    return true;
}

static bool copy_copies_array_with_one_element(char** msg) {
    array src;
    array_init(&src, sizeof(char));
    char* elem = (char*)array_append(&src);
    *elem = 1;

    array dst;
    array_copy(&dst, &src);

    ASSERT_INT_EQUAL(src.len, dst.len,
                     "The copied array's length was incorrect.", msg);
    ASSERT_INT_EQUAL(src.cap, dst.cap,
                     "The copied array's capacity was incorrect.", msg);
    ASSERT_INT_EQUAL(src.size, dst.size,
                     "The copied array's size was incorrect.", msg);

    char* expected_data = (char*)src.data;
    char* actual_data = (char*)dst.data;
    int data_equal = (memcmp(expected_data, actual_data, src.len) == 0);
    ASSERT_CONDITION(data_equal, expected_data, actual_data,
                     "The copied data wasn't equal to the source data.", msg);

    array_free(&src);
    array_free(&dst);
    return true;
}

static bool copy_copies_array_with_two_elements(char** msg) {
    array src;
    array_init(&src, sizeof(char));
    char* elem1 = (char*)array_append(&src);
    *elem1 = 1;
    char* elem2 = (char*)array_append(&src);
    *elem2 = 2;

    array dst;
    array_copy(&dst, &src);

    ASSERT_INT_EQUAL(src.len, dst.len,
                     "The copied array's length was incorrect.", msg);
    ASSERT_INT_EQUAL(src.cap, dst.cap,
                     "The copied array's capacity was incorrect.", msg);
    ASSERT_INT_EQUAL(src.size, dst.size,
                     "The copied array's size was incorrect.", msg);

    char* expected_data = (char*)src.data;
    char* actual_data = (char*)dst.data;
    int data_equal = (memcmp(expected_data, actual_data, src.len) == 0);
    ASSERT_CONDITION(data_equal, expected_data, actual_data,
                     "The copied data wasn't equal to the source data.", msg);

    array_free(&src);
    array_free(&dst);
    return true;
}

results array_tests() {
    RUN_TEST(ensure_size_on_empty_array_increases_capacity_to_one);
    RUN_TEST(ensure_size_on_non_empty_array_doubles_capacity);
    RUN_TEST(append_increases_array_length_by_one);
    RUN_TEST(copy_copies_array_with_no_elements);
    RUN_TEST(copy_copies_array_with_one_element);
    RUN_TEST(copy_copies_array_with_two_elements);

    return (results) { passed, failed };
}
