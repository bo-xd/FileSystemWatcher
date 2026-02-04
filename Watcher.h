#ifndef WATCHER_H
#define WATCHER_H

#include <stdbool.h>

typedef struct {
  const char *path;
} Watch;

typedef struct {
  bool Changed;
  bool Created;
  bool Deleted;
} SystemEventArgs;

// Events
static void onChanged(char *sender, SystemEventArgs e);
static void OnCreated(char *sender, SystemEventArgs e);
static void onDeleted(char *sender, SystemEventArgs e);
static void onRenamed(char *sender, SystemEventArgs e);

// Watchers
void *Watcher(void *arg);
void StartThread(const char *path);

#endif
