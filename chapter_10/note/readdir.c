#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main(int argc, char **argv){
    DIR *streamp;
    struct dirent *dep;

    streamp = opendir(argv[1]);

    errno = 0;
    while ((dep = readdir(streamp)) != NULL)
        printf("%s\n", dep->d_name);
    if (errno != 0)

    closedir(streamp);
    exit(0);
}