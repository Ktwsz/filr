#include <check.h>
#include <stdlib.h>
#include <stdio.h>

#include "../src/linked_list.h"
#include "../src/result.h"

START_TEST(test_ll_init)
{
    list_t list = list_init();

    ck_assert_int_eq(list.head, -1);
    ck_assert(list.tail == NULL);
}
END_TEST

START_TEST(test_ll_new)
{
    list_t list = list_init();
    result err = list_new(&list, 2137);

    ck_assert(!err.err);

    ck_assert_int_eq(list.tail->head, 2137);
    ck_assert(list.tail->tail == NULL);

    list_clear(list.tail);
}
END_TEST

START_TEST(test_ll_query_add)
{
    result err;

    list_t list = list_init();

    err = list_query(&list, 1);
    ck_assert(!err.err);

    err = list_query(&list, 2);
    ck_assert(!err.err);

    err = list_query(&list, 3);
    ck_assert(!err.err);

    err = list_query(&list, 4);
    ck_assert(!err.err);

    list_t *it = list.tail;
    ck_assert_int_eq(it->head, 1);
    it = it->tail;
    ck_assert_int_eq(it->head, 2);
    it = it->tail;
    ck_assert_int_eq(it->head, 3);
    it = it->tail;
    ck_assert_int_eq(it->head, 4);
    it = it->tail;

    ck_assert(it == NULL);

    list_clear(list.tail);
}
END_TEST

START_TEST(test_ll_query_clear)
{
    result err;

    list_t list = list_init();

    err = list_query(&list, 1);
    ck_assert(!err.err);
    
    err = list_query(&list, 2);
    ck_assert(!err.err);
    
    err = list_query(&list, 3);
    ck_assert(!err.err);
    
    err = list_query(&list, 1);
    ck_assert(!err.err);
    
    err = list_query(&list, 2);
    ck_assert(!err.err);
    
    err = list_query(&list, 3);
    ck_assert(!err.err);
    

    ck_assert(list.tail == NULL);
}
END_TEST

Suite *ll_suite(void) {
    Suite *s = suite_create("Linked list");

    TCase *tc_init = tcase_create("Init");
    tcase_add_test(tc_init, test_ll_init);

    TCase *tc_new = tcase_create("New");
    tcase_add_test(tc_new, test_ll_new);

    TCase *tc_query = tcase_create("query");
    tcase_add_test(tc_query, test_ll_query_add);
    tcase_add_test(tc_query, test_ll_query_clear);
        
    suite_add_tcase(s, tc_init);
    suite_add_tcase(s, tc_new);
    suite_add_tcase(s, tc_query);

    return s;
}

int main(void) {

    Suite *s = ll_suite();

    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

    int failed = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (failed == 0)? EXIT_SUCCESS : EXIT_FAILURE;
}
