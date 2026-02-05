#include "Watcher.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

FileSystemWatcher *watcher = NULL;

void OnFileCreated(void *sender, FileSystemEventArgs *e) {
  printf("Created: %s\n", e->FullPath);
}

void OnFileChanged(void *sender, FileSystemEventArgs *e) {
  printf("Modified: %s\n", e->FullPath);
}

void OnFileDeleted(void *sender, FileSystemEventArgs *e) {
  printf("Deleted: %s\n", e->FullPath);
}

void signal_handler(int sig) {
  printf("\nCleaning up...\n");
  FileSystemWatcher_Dispose(watcher);
  exit(0);
}

int main() {
  signal(SIGINT, signal_handler);

  watcher = FileSystemWatcher_Create("/home/");
  watcher->Created = OnFileCreated;
  watcher->Changed = OnFileChanged;
  watcher->Deleted = OnFileDeleted;

  FileSystemWatcher_Start(watcher);

  while (1) {
    sleep(1);
  }

  return 0;
}
