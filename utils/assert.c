#include <assert.h>

void kernel_assert(int statement, char* message) {
    if (statement != 1) {
        kernel_printf("[ASSERT ERROR]: %s\n", message);
        while (1)
            ;
    }
}