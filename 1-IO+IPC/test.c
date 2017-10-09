#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
    char a;
    char b;
} my_struct;

my_struct a;
my_struct b;

int main() {

    my_struct *my_array[2] = {&a, &b};
    a.a = 'h';
    b.b = 'j';

    printf("%c\n", my_array[0]->a);

    exit(0);
}
