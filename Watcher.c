#include "Watcher.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/inotify.h>
#include <unistd.h>

// Events
static void onChanged(char *sender, SystemEventArgs e) {
  printf("Modified: %s\n", sender);
}

static void onCreated(char *sender, SystemEventArgs e) {
  printf("Created: %s\n", sender);
}

static void onDeleted(char *sender, SystemEventArgs e) {
  printf("Deleted: %s\n", sender);
}

static void onRenamed(char *sender, SystemEventArgs e) {
  printf("Renamed/Moved: %s\n", sender);
}

// Watchers
void *Watcher(void *arg) {
  const char *path = (const char *)arg;

  int fd = inotify_init();
  if (fd < 0) {
    perror("inotify_init");
    return NULL;
  }

  int wd = inotify_add_watch(fd, path,
                             IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM |
                                 IN_MOVED_TO);

  if (wd < 0) {
    perror("inotify_add_watch");
    close(fd);
    return NULL;
  }

  char buffer[4096];

  while (1) {
    int length = read(fd, buffer, sizeof(buffer));
    if (length < 0) {
      perror("read");
      break;
    }

    int i = 0;
    while (i < length) {
      struct inotify_event *event = (struct inotify_event *)&buffer[i];

      if (event->len) {
        SystemEventArgs args = {0};
        if (event->mask & IN_CREATE) {
          args.Created = true;
          onCreated(event->name, args);
        }
        if (event->mask & IN_MODIFY) {
          args.Changed = true;
          onChanged(event->name, args);
        }
        if (event->mask & IN_DELETE) {
          args.Deleted = true;
          onDeleted(event->name, args);
        }
        if (event->mask & (IN_MOVED_FROM | IN_MOVED_TO)) {
          onRenamed(event->name, args);
        }
      }

      i += sizeof(struct inotify_event) + event->len;
    }
  }

  inotify_rm_watch(fd, wd);
  close(fd);
  return NULL;
}

void StartThread(const char *path) {
  pthread_t id;
  pthread_create(&id, NULL, Watcher, (void *)path);
  pthread_detach(id);
}
