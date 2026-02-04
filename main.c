#include "Watcher.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    const char *path_to_watch = "./";

    printf("Starting file system watcher on: %s\n", path_to_watch);
    
    StartThread(path_to_watch);

    while (1) {
        sleep(1);
    }

    return 0;
}
