#ifndef UTILS_H
#define UTILS_H

void reverse_string(char *str, int length);
int int_to_str(unsigned int num, char *str);
void build_filename(char *filename, int pid, const char *prefix);
void custom_srand(unsigned int new_seed);
unsigned int custom_rand();
int custom_atoi(char *s);
void print_with_leading_zeros(unsigned int number, int width);
void read_mem_stats(const char *filename, unsigned int *alloc_time, unsigned int *free_time);
void read_stats(const char *filename, unsigned int *write_time, unsigned int *read_time, unsigned int *delete_time, unsigned int *alloc_time, unsigned int *free_time);

#endif // UTILS_H
