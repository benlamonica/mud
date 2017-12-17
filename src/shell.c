#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>

int main(int argc, char** argv) {
    char *const params[] = {"/bin/nc", "localhost", "4000", NULL};
    
    execvp("nc", params);
    printf("Error: %d", errno);
    return 0;
}
