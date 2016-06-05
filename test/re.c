#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>

#include "lazy_re.h"

#define PATTERN "pattern"
#define STRING "__ kawaii wa segi! __"
static regmatch_t dummyGroups[REGEX_MAX_GROUP] = {{-1}};

int __wrap_regexec(const regex_t *restrict preg, const char *restrict string, size_t nmatch, regmatch_t pmatch[restrict], int eflags) {
    check_expected_ptr(string);
    bool is_to_find = mock_type(bool);

    if (pmatch && is_to_find) {
        memcpy(pmatch, dummyGroups, sizeof(dummyGroups));
    }

    return mock_type(int);
}

int __wrap_regcomp(regex_t* preg, const char* pattern, int cflags) {
    check_expected_ptr(pattern);
    return mock_type(int);
}

void __wrap_regfree(regex_t *preg) {
    check_expected_ptr(preg);
}

static void test_true(void **state) {
    expect_string(__wrap_regcomp, pattern, PATTERN);
    will_return(__wrap_regcomp, 0);

    expect_string(__wrap_regexec, string, STRING);
    will_return(__wrap_regexec, false);
    will_return(__wrap_regexec, 0);
    assert_true(Regex_test(PATTERN, STRING));
}

static void test_false(void **state) {
    expect_string(__wrap_regcomp, pattern, PATTERN);
    will_return(__wrap_regcomp, 0);

    expect_string(__wrap_regexec, string, STRING);
    will_return(__wrap_regexec, false);
    will_return(__wrap_regexec, 1);
    assert_false(Regex_test(PATTERN, STRING));
}

static void free_check_arg(void **state) {
    regex_t dummy;
    expect_memory(__wrap_regfree, preg, &dummy, sizeof(dummy));
    Regex_free(&dummy);
}

static void search_no_match(void **state) {
    expect_string(__wrap_regcomp, pattern, PATTERN);
    will_return(__wrap_regcomp, 0);

    expect_string(__wrap_regexec, string, STRING);
    will_return(__wrap_regexec, false);
    will_return(__wrap_regexec, REG_NOMATCH);
    assert_null(Regex_search(PATTERN, STRING, 0));
}

static void search_match(void **state) {
    expect_string(__wrap_regcomp, pattern, PATTERN);
    will_return(__wrap_regcomp, 0);

    /* Setup some findings */
    memset(dummyGroups, -1, sizeof(dummyGroups));
    dummyGroups[0].rm_so = 3; /* whole sentence */
    dummyGroups[0].rm_eo = 18;
    dummyGroups[1].rm_so = 3; /* kawaii */
    dummyGroups[1].rm_eo = 9;
    dummyGroups[2].rm_so = 10; /* wa */
    dummyGroups[2].rm_eo = 12;
    dummyGroups[3].rm_so = 13; /* segi */
    dummyGroups[3].rm_eo = 17;
    expect_string(__wrap_regexec, string, STRING);
    will_return(__wrap_regexec, true);
    will_return(__wrap_regexec, 0);

    RegexMatch *result = Regex_search(PATTERN, STRING, 0);

    assert_non_null(result);

    for (size_t idx = 0; idx < 4; idx++) {
        assert_int_equal(dummyGroups[idx].rm_so, result->innerGroups[idx].rm_so);
        assert_int_equal(dummyGroups[idx].rm_eo, result->innerGroups[idx].rm_eo);
        assert_int_equal(strncmp(STRING + dummyGroups[idx].rm_so,
                                 result->groups[idx],
                                 dummyGroups[idx].rm_eo - dummyGroups[idx].rm_so
                    ), 0);
    }

    for (size_t idx = 4; idx < REGEX_MAX_GROUP; idx++) {
        assert_null(result->groups[4]);
        assert_int_equal(result->innerGroups[4].rm_so, -1);
        assert_int_equal(result->innerGroups[4].rm_eo, -1);
    }
}

static void search_next_some(void **state) {
    expect_any(__wrap_regfree, preg);
    /* Setup some findings */
    memset(dummyGroups, -1, sizeof(dummyGroups));
    dummyGroups[0].rm_so = 1; /* whole sentence */
    dummyGroups[0].rm_eo = 3; /* "__" */

    expect_string(__wrap_regexec, string, STRING + 18);
    will_return(__wrap_regexec, true);
    will_return(__wrap_regexec, 0);

    RegexMatch *result = Regex_searchNext();

    assert_non_null(result);
    assert_int_equal(dummyGroups[0].rm_so, result->innerGroups[0].rm_so);
    assert_int_equal(dummyGroups[0].rm_eo, result->innerGroups[0].rm_eo);

    assert_int_equal(strncmp(STRING + 18 + dummyGroups[0].rm_so,
                             result->groups[0],
                             dummyGroups[0].rm_eo - dummyGroups[0].rm_so
                    ), 0);

    for (size_t idx = 1; idx < REGEX_MAX_GROUP; idx++) {
        assert_null(result->groups[4]);
        assert_int_equal(result->innerGroups[4].rm_so, -1);
        assert_int_equal(result->innerGroups[4].rm_eo, -1);
    }
}

static void search_next_none(void **state) {
    memset(dummyGroups, -1, sizeof(dummyGroups));

    RegexMatch *result = Regex_searchNext();
    assert_null(result);
}

int main(void) {
    /* Suites ain't working on windows :( */
    run_test(test_true);
    run_test(test_false);
    run_test(free_check_arg);
    run_test(search_no_match);
    /* match, next_some and  next_none should go each after each in that order. */
    run_test(search_match);
    run_test(search_next_some);
    run_test(search_next_none);

    return 0;
}
