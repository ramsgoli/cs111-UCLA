#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main() {
    char buff[256];

    int num = read(0, &buff, sizeof(buff));
    printf("%d\n", num);
    write(1, &buff, num);

    exit(0);
}
