/* Deliberately simple oracle: line-by-line input and linear city lookup.
 * Independent of the challenge solutions; standard library only.
 */
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CITIES 10000
#define MAX_NAME_BYTES 100

typedef struct {
    char name[MAX_NAME_BYTES + 1];
    int min, max;
    int64_t sum, count;
} City;

static City cities[MAX_CITIES];

static int compare_cities(const void *a, const void *b) {
    return strcmp(((const City *)a)->name, ((const City *)b)->name);
}

static int parse_temperature(const char *text, int *result) {
    int negative = *text == '-';
    const char *p = text + negative;
    int value = 0, digits = 0;
    while (*p >= '0' && *p <= '9') {
        if (++digits > 2) return 0;
        value = value * 10 + (*p++ - '0');
    }
    if (digits == 0 || *p++ != '.') return 0;
    if (*p < '0' || *p > '9') return 0;
    value = value * 10 + (*p++ - '0');
    if (*p != '\0') return 0;
    *result = negative ? -value : value;
    return 1;
}

static void print_tenths(int64_t value) {
    if (value < 0) {
        putchar('-');
        value = -value;
    }
    printf("%" PRId64 ".%" PRId64, value / 10, value % 10);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s INPUT > EXPECTED\n", argv[0]);
        return 2;
    }
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        perror(argv[1]);
        return 2;
    }

    /* 100 name bytes + ';' + '-99.9' + newline + terminator. */
    char line[108];
    size_t city_count = 0;
    uint64_t line_number = 0;
    while (fgets(line, sizeof line, input)) {
        ++line_number;
        size_t length = strlen(line);
        if (length && line[length - 1] == '\n') line[--length] = '\0';
        char *separator = strchr(line, ';');
        int temperature;
        if (!separator || separator == line ||
            (size_t)(separator - line) > MAX_NAME_BYTES ||
            !parse_temperature(separator + 1, &temperature)) {
            fprintf(stderr, "Invalid input at line %" PRIu64 "\n", line_number);
            fclose(input);
            return 2;
        }
        *separator = '\0';

        size_t i = 0;
        while (i < city_count && strcmp(cities[i].name, line) != 0) ++i;
        if (i == city_count) {
            if (city_count == MAX_CITIES) {
                fprintf(stderr, "More than %d unique cities\n", MAX_CITIES);
                fclose(input);
                return 2;
            }
            strcpy(cities[i].name, line);
            cities[i].min = temperature;
            cities[i].max = temperature;
            ++city_count;
        }
        City *city = &cities[i];
        if (temperature < city->min) city->min = temperature;
        if (temperature > city->max) city->max = temperature;
        city->sum += temperature;
        ++city->count;
    }
    int read_error = ferror(input);
    int close_error = fclose(input);
    if (read_error || close_error) {
        fprintf(stderr, "Failed to read input\n");
        return 2;
    }

    qsort(cities, city_count, sizeof cities[0], compare_cities);
    for (size_t i = 0; i < city_count; ++i) {
        City *city = &cities[i];
        /* README: roundTowardPositive to one decimal place.
         * C division truncates toward zero: only positive remainders
         * require an adjustment to obtain the mathematical ceiling.
         */
        int64_t mean = city->sum / city->count;
        if (city->sum % city->count > 0) ++mean;
        printf("%s:", city->name);
        print_tenths(city->min);
        putchar('/');
        print_tenths(mean);
        putchar('/');
        print_tenths(city->max);
        putchar('\n');
    }
    if (fflush(stdout) == EOF || ferror(stdout)) {
        fprintf(stderr, "Failed to write output\n");
        return 2;
    }
    return 0;
}
