#include <stdbool.h>
#include <stdio.h>

#define RUN_SCENARIO(f)                                                        \
    bool f(char**);                                                            \
    ++total;                                                                   \
    msg = NULL;                                                                \
    printf("Running %s... ", #f);                                              \
    if (f(&msg)) {                                                             \
        puts("passed.");                                                       \
        ++passed;                                                              \
    } else {                                                                   \
        puts("failed.");                                                       \
        ++failed;                                                              \
    }                                                                          \
    if (msg) {                                                                 \
        printf("\t%s\n", msg);                                                 \
    }

char* msg = NULL;
int passed = 0;
int failed = 0;
int total = 0;

int main() {
    fclose(stderr);

    RUN_SCENARIO(xform_buffer_scenario);
    RUN_SCENARIO(complex_scenario);
    RUN_SCENARIO(basic_compose_scenario);
    RUN_SCENARIO(basic_xform_scenario);
    RUN_SCENARIO(xform_anticipated_scenario);

    printf("\n%d tests passed.\n"
           "%d tests failed.\n"
           "%d tests total.\n",
           passed, failed, total);

    if (failed > 0) {
        return 1;
    }
}
