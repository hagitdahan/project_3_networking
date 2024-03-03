#include <stdlib.h>
#include <time.h>
/*
* @brief A random data generator function based on srand() and rand().
* @param size The size of the data to generate (up to 2^32 bytes).
* @return A pointer to the buffer.
*/
char *util_generate_random_data(unsigned int size) {
    char *buffer = NULL;
// Argument check.
    if (size == 0)
        return NULL;
    buffer = (char *) calloc(size, sizeof(char));
// Error checking.
    if (buffer == NULL)
        return NULL;
// Randomize the seed of the random number generator.
    srand(time(NULL));
    for (unsigned int i = 0; i < size; i++) {
        *(buffer + i) = 'A';//((unsigned int) rand() % 256);
        //printf("%d, %c",i,*(buffer+i));
    }
    return buffer;
}
