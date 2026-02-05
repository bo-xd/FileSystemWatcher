#include "Watcher.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

FileSystemWatcher *FileSystemWatcher_Create(const char *path) {
  FileSystemWatcher *watcher = malloc(sizeof(FileSystemWatcher));
  if (!watcher)
    return NULL;

  watcher->Path = strdup(path);
  watcher->EnableRaisingEvents = false;
  watcher->IncludeSubdirectories = false;
  watcher->Changed = NULL;
  watcher->Created = NULL;
  watcher->Deleted = NULL;
  watcher->Renamed = NULL;
  watcher->_inotify_fd = -1;
  watcher->_watch_descriptor = -1;
  watcher->_running = false;

  return watcher;
}

static void RaiseEvent(FileSystemEventHandler handler, void *sender,
                       const char *name, const char *path) {
  if (handler) {
    FileSystemEventArgs args;
    args.Name = strdup(name);

    size_t len = strlen(path) + strlen(name) + 2;
    args.FullPath = malloc(len);
    snprintf(args.FullPath, len, "%s/%s", path, name);

    handler(sender, &args);

    free(args.Name);
    free(args.FullPath);
  }
}

static void *WatcherThread(void *arg) {
  FileSystemWatcher *watcher = (FileSystemWatcher *)arg;
  char buffer[4096];

  while (watcher->_running) {
    int length = read(watcher->_inotify_fd, buffer, sizeof(buffer));
    if (length < 0) {
      if (watcher->_running) {
        perror("read");
      }
      break;
    }

    int i = 0;
    while (i < length) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];

      if (event->len) {
        if (event->mask & IN_CREATE) {
          RaiseEvent(watcher->Created, watcher, event->name, watcher->Path);
        }
        if (event->mask & IN_MODIFY) {
          RaiseEvent(watcher->Changed, watcher, event->name, watcher->Path);
        }
        if (event->mask & IN_DELETE) {
          RaiseEvent(watcher->Deleted, watcher, event->name, watcher->Path);
        }
        if (event->mask & (IN_MOVED_FROM | IN_MOVED_TO)) {
          RaiseEvent(watcher->Renamed, watcher, event->name, watcher->Path);
        }
      }

      i += sizeof(struct inotify_event) + event->len;
    }
  }

  return NULL;
}

void FileSystemWatcher_Start(FileSystemWatcher *watcher) {
  if (watcher->_running)
    return;

  watcher->_inotify_fd = inotify_init();
  if (watcher->_inotify_fd < 0) {
    perror("inotify_init");
    return;
  }

  uint32_t mask =
      IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;
  watcher->_watch_descriptor =
      inotify_add_watch(watcher->_inotify_fd, watcher->Path, mask);
  if (watcher->_watch_descriptor < 0) {
    perror("inotify_add_watch");
    close(watcher->_inotify_fd);
    return;
  }

  watcher->_running = true;
  watcher->EnableRaisingEvents = true;

  pthread_create(&watcher->_thread, NULL, WatcherThread, watcher);
}

void FileSystemWatcher_Stop(FileSystemWatcher *watcher) {
  if (!watcher->_running)
    return;

  watcher->_running = false;
  watcher->EnableRaisingEvents = false;

  if (watcher->_watch_descriptor >= 0) {
    inotify_rm_watch(watcher->_inotify_fd, watcher->_watch_descriptor);
  }
  if (watcher->_inotify_fd >= 0) {
    close(watcher->_inotify_fd);
  }

  pthread_join(watcher->_thread, NULL);
}

void FileSystemWatcher_Dispose(FileSystemWatcher *watcher) {
  if (!watcher)
    return;

  FileSystemWatcher_Stop(watcher);
  free((void *)watcher->Path);
  free(watcher);
}
