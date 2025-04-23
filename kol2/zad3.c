#include "zad3.h"

void readwrite(int pd, size_t block_size);

void createpipe(size_t block_size)
{
    /* Utwórz potok nienazwany */
    int fd[2];
    pipe(fd);

    /* Odkomentuj poniższe funkcje zamieniając ... na deskryptory potoku */
    check_pipe(fd);
    check_write(fd, block_size, readwrite);
}

void readwrite(int write_pd, size_t block_size)
{
    /* Otworz plik `unix.txt`, czytaj go po `block_size` bajtów
    i w takich `block_size` bajtowych kawałkach pisz do potoku `write_pd`.*/

    FILE *file = fopen("./unix.txt", "r");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    char *buffer = malloc(block_size);
    if (buffer == NULL)
    {
        perror("malloc");
        fclose(file);
        return;
    }

    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, block_size, file)) > 0)
    {
        if (write(write_pd, buffer, bytes_read) != bytes_read)
        {
            perror("write");
            break;
        }
    }

    free(buffer);
    /* Zamknij plik */
    fclose(file);
}

int main()
{
    srand(42);
    size_t block_size = rand() % 128;
    createpipe(block_size);

    return 0;
}