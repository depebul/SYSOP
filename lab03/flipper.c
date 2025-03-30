#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void reverse_line(char *line)
{
    int length = strlen(line);
    if (length > 0 && line[length - 1] == '\n')
    {
        length--;
    }

    int start = 0;
    int end = length - 1;

    while (start < end)
    {
        char temp = line[start];
        line[start] = line[end];
        line[end] = temp;

        start++;
        end--;
    }
}

void process_file(const char *input_path, const char *output_path)
{
    FILE *input_file = fopen(input_path, "r");
    if (input_file == NULL)
    {
        perror("Error opening input file");
        return;
    }

    FILE *output_file = fopen(output_path, "w");
    if (output_file == NULL)
    {
        perror("Error opening output file");
        fclose(input_file);
        return;
    }

    char line[1024];
    while (fgets(line, sizeof(line), input_file) != NULL)
    {
        reverse_line(line);
        fprintf(output_file, "%s", line);
    }

    fclose(input_file);
    fclose(output_file);
}

void process_directory(const char *input_directory, const char *output_directory)
{
    DIR *dir = opendir(input_directory);
    if (dir == NULL)
    {
        perror("Error opening input directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        {
            char input_path[1024];
            char output_path[1024];

            snprintf(input_path, sizeof(input_path), "%s/%s", input_directory, entry->d_name);
            snprintf(output_path, sizeof(output_path), "%s/%s", output_directory, entry->d_name);

            int name_len = strlen(entry->d_name);
            if (name_len >= 4 && strcmp(entry->d_name + name_len - 4, ".txt") == 0)
            {
                process_file(input_path, output_path);
            }
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <input_directory> <output_directory>\n", argv[0]);
        return 1;
    }

    process_directory(argv[1], argv[2]);

    return 0;
}