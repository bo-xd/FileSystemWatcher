#ifndef WATCHER_H
#define WATCHER_H
#include <pthread.h>
#include <stdbool.h>

typedef struct {
  char *FullPath;
  char *Name;
} FileSystemEventArgs;

typedef void (*FileSystemEventHandler)(void *sender, FileSystemEventArgs *e);

typedef struct {
  const char *Path;
  bool EnableRaisingEvents;
  bool IncludeSubdirectories;

  FileSystemEventHandler Changed;
  FileSystemEventHandler Created;
  FileSystemEventHandler Deleted;
  FileSystemEventHandler Renamed;

  int _inotify_fd;
  int _watch_descriptor;
  pthread_t _thread;
  bool _running;
} FileSystemWatcher;

FileSystemWatcher *FileSystemWatcher_Create(const char *path);
void FileSystemWatcher_Dispose(FileSystemWatcher *watcher);
void FileSystemWatcher_Start(FileSystemWatcher *watcher);

#endif
