/**
 * @file
 * @brief Lazy RE header file.
 */

#pragma once

#include <regex.h>
#include <stdbool.h>

/** @brief Max number of groups in match. */
#define REGEX_MAX_GROUP 10

/**
 * @brief Represents regex match.
 *
 * First group contains the whole match.
 *
 * @note Number of groups are limited to REGEX_MAX_GROUP.
 */
typedef struct {
    regmatch_t innerGroups[REGEX_MAX_GROUP]; /**< Posix regex group array. */
    char *groups[REGEX_MAX_GROUP]; /**< Group array of matched strings. */
} RegexMatch;

/**
 * @brief Compiles regex.
 *
 * Regex cflags reference http://www.gnu.org/software/libc/manual/html_node/Flags-for-POSIX-Regexps.html#Flags-for-POSIX-Regexps
 *
 * @param regex Pointer to a struct with regex.
 * @param pattern String containing regex.
 * @param cflags Regex flags with which regex is created.
 */
void Regex_compile(regex_t *regex, const char* pattern, const int cflags);
/**
 * @brief Frees memory allocated for regex.
 *
 * @param regex Regex struct.
 */
void Regex_free(regex_t* regex);

/**
 * @brief Test whether pattern presents in a string.
 *
 * @param regex Pattern which will be used to match.
 * @param string String against which to perform match.
 *
 * @retval true If pattern is found.
 * @retval false otherwise.
 */
bool Regex_test(const char* pattern, const char* string);
/**
 * @brief Search first occurrence of a pattern in a string.
 *
 * Regex eflags reference http://www.gnu.org/software/libc/manual/html_node/Matching-POSIX-Regexps.html#Matching-POSIX-Regexps
 *
 * @param regex Pattern which will be used to match.
 * @param string String against which to perform match.
 * @param eflags Regex flags with which to match.
 *
 * @return Match with groups if pattern is found. 0 Otherwise.
 */
RegexMatch* Regex_search(const char* pattern, const char* string, const int eflags);

/**
 * @brief Search a first occurrence of a regex in a string.
 *
 * Regex eflags reference http://www.gnu.org/software/libc/manual/html_node/Matching-POSIX-Regexps.html#Matching-POSIX-Regexps
 *
 * @param regex Regex which will be used to match.
 * @param string String against which to perform match.
 * @param eflags Regex flags with which to match.
 *
 * @return Match with groups if pattern is found. 0 Otherwise.
 */

RegexMatch* Regex_compiledSearch(regex_t* regex, const char* string, const int eflags);

/**
 * @brief Performs next search from cache of the last successful one.
 *
 * @note Make sure to call @ref Regex_search before calling this one.
 *
 * @return Match with groups if pattern is found. 0 Otherwise.
 */
RegexMatch* Regex_searchNext();
