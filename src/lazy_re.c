/**
 * @file
 * @brief Lazy RE
 *
 * It is based on POSIX regular expression:
 *
 * http://www.regular-expressions.info/posix.html
 *
 * Special characters:
 * `.[{}()\*+?|^$`
 */

#include <sys/types.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lazy_re.h"

/**
 * @brief Assert to check regex functions input parameters.
 *
 * @param condition Assert condition.
 * @param msg Pointer to string with message.
 */
#define REGEX_ASSERT(condition, msg) assert((condition) || !printf("REGEX_ERROR: %s\n", msg))

/**
 * @brief Static array used in return values of search.
 */
static RegexMatch buffMatch = {
    .innerGroups = {
        {
            .rm_so = -1,
            .rm_eo = -1
        }
    },
    .groups = {0}
};

/**
 * @brief Represents Regex cache.
 */
typedef struct {
    regex_t regex;
    const char* string;
    int eflags;
} RegexCache;

/**
 * @brief Stores last created pattern.
 */
static RegexCache regexCache = {
    .string = 0
};

/**
 * @brief Free match.
 *
 * @param match Pointer to a match.
 */
static void Regex_cleanMatch(RegexMatch *match) {
    REGEX_ASSERT(match, "Null match passed");

    for (size_t idx = 0; idx < REGEX_MAX_GROUP; idx++) {
        if (match->groups[idx]) {
            free(match->groups[idx]);
            match->groups[idx] = 0;
        }

        match->innerGroups[idx].rm_so = -1;
        match->innerGroups[idx].rm_eo = -1;
    }
}

/*
 * External methods.
 */
void Regex_compile(regex_t *regex, const char* pattern, const int cflags) {
    int tmp_result = regcomp(regex, pattern, cflags);

    REGEX_ASSERT(!tmp_result, "Cannot compile regex");
}

void Regex_free(regex_t* regex) {
    REGEX_ASSERT(regex, "Null regex passed");
    regfree(regex);
}

bool Regex_test(const char* pattern, const char* string) {
    regex_t regex;

    Regex_compile(&regex, pattern, REG_EXTENDED | REG_NOSUB);

    int tmp_result = regexec(&regex, string, 0, 0, 0);

    return tmp_result != REG_NOMATCH;
}


RegexMatch* Regex_compiledSearch(regex_t* regex, const char* string, const int eflags) {
    Regex_cleanMatch(&buffMatch);

    int tmp_result = regexec(regex, string, REGEX_MAX_GROUP, &buffMatch.innerGroups[0], eflags);

    if (tmp_result == REG_NOMATCH) return 0;

    /* Fill groups slices */
    for (size_t idx = 0; idx < REGEX_MAX_GROUP; idx++) {
        if (buffMatch.innerGroups[idx].rm_so == (regoff_t)-1) {
            /* No more groups */
            break;
        }

        const size_t group_len = buffMatch.innerGroups[idx].rm_eo - buffMatch.innerGroups[idx].rm_so;
        const char *start = string + buffMatch.innerGroups[idx].rm_so;

        buffMatch.groups[idx] = malloc(sizeof(char) * (group_len + 1));
        strncpy(buffMatch.groups[idx], start, group_len);
        buffMatch.groups[idx][group_len] = 0;
    }

    return &buffMatch;

}

RegexMatch* Regex_search(const char* pattern, const char* string, const int eflags) {
    regex_t *regex = &regexCache.regex;
    regexCache.eflags = eflags;

    Regex_free(regex);

    Regex_compile(regex, pattern, REG_EXTENDED);

    RegexMatch *result = Regex_compiledSearch(regex, string, eflags);

    if (result &&
        buffMatch.innerGroups[0].rm_eo < (regoff_t)strlen(string))
    {
        regexCache.string = string + buffMatch.innerGroups[0].rm_eo;
    }
    else regexCache.string = 0;

    return result;
}

RegexMatch* Regex_searchNext() {
    if (!regexCache.string) return 0;

    RegexMatch *result = Regex_compiledSearch(&regexCache.regex, regexCache.string, regexCache.eflags);

    if (result &&
        buffMatch.innerGroups[0].rm_eo < (regoff_t)strlen(regexCache.string))
    {
        regexCache.string += buffMatch.innerGroups[0].rm_eo;
    }
    else regexCache.string = 0;

    return result;
}
