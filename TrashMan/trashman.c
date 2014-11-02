//  TrashMan
//
//  Created 2014-11-01.
//
//  Copyright © 2014 mathew <meta@pobox.com>. All rights reserved.

#define _XOPEN_SOURCE
#define _BSD_SOURCE
#include <stdio.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/param.h>
#include <limits.h>
#include "debug.h"
#include "trashman.h"

#define PROGNAME "trashman"

// The xattr name to use to store the timestamp when we first saw the file.
// Linux requires a "user." prefix.
#ifdef __linux__
  #define ATTR_NAME "user.com.ath0.tm.date"
#else
  #define ATTR_NAME "com.ath0.tm.date"
#endif
// The format string to use for the timestamp
#define ATTR_FORMAT "%Y-%m-%dT%H:%M:%SZ"
// The size of a timestamp string, in bytes.
#define ATTR_SIZE 21 // e.g. 2014-04-28T13:28:00Z + '\0'

// Gets and/or sets the timestamp on a given file.
// If the file has no timestamp, it's set to now and -1 is returned.
// If it has a timestamp, the timestamp is placed in the ts buffer and 0 is returned.
// fspc is the file path.
// ts is the timestamp, passed as a pointer to a buffer of at least ATTR_SIZE bytes.
// now is the current time as an ISO-8601 UTC string.
ssize_t getsettimestamp(const char *fspc, void *ts, char *now) {
  ssize_t bytes = p_lgetxattr(fspc, ATTR_NAME, ts, (size_t) ATTR_SIZE);
  if (bytes == -1) {
    debug("No xattr on %s", fspc);
//    printf("No xattr on %s\n", fspc);
    // If it's a simple case of missing attribute, add it now.
    if (errno == ENOATTR) {
      int rc = p_lsetxattr(fspc, ATTR_NAME, now, (size_t) ATTR_SIZE);
      if (rc == -1) {
        fprintf(stderr, "%s: %s setting xattr\n", fspc, strerror(errno));
      }
    } else {
      fprintf(stderr, "%s: %s reading xattr\n", fspc, strerror(errno));
    }
  }
  debug("got %ld bytes from getxattr", bytes);
  return bytes;
}

// Process one file. Read the xattr date/time, compute the difference between then and now,
// and if it exceeds the specified number of days, delete the file.
// If days <= 0, no deletion is performed.
// Both nowstr and ttnow are the current date/time; once as a string, once as a time_t.
// Returns 1 if some sort of error occurred, else 0.
void processfile(const char *fspc, char *nowstr, time_t ttnow, int days) {
  double ddays = (double) days;
  char tsstr[ATTR_SIZE+1];
  if (getsettimestamp(fspc, tsstr, nowstr) > 0) {
    // The file had a timestamp
    debug("tsstr = %s, days = %d", tsstr, days);
    struct tm tm;
    if (days > 0 && strptime(tsstr, ATTR_FORMAT, &tm)) {
      time_t ttstamp = timegm(&tm);
      debug("ttstamp = %ld", ttstamp);
      double dt = difftime(ttnow, ttstamp) / 86400;
      debug("dt = %lf\n", dt);
      if (dt >= ddays) {
        unlink(fspc);
        debug("delete %s, %lf days old", fspc, dt);
      } else {
        debug("do not delete %s, %lf days old", fspc, dt);
      }
    }
  }
}

void processdir(const char *fspc, char *nowstr, time_t ttnow, int days) {
  DIR *dir;
  dir = opendir(fspc);
  if (dir == NULL) {
    perror(fspc);
    // but continue to process other arguments
    return;
  }
  struct dirent *dirent;
  char fpath[PATH_MAX];
  while ((dirent = readdir(dir)) != NULL) {
    char *file = dirent->d_name;
    if (file[0] != '.') {
    snprintf(fpath, PATH_MAX, "%s/%s", fspc, file);
    debug("fpath = %s", fpath);
    processfile(fpath, nowstr, ttnow, days);
    }
  }
  closedir(dir);
}

// Process a path which might either be a file or a directory
void processpath(const char *fspc, char *nowstr, time_t ttnow, int days) {
  struct stat s;
  // Use lstat because realpath has been used to process arguments from the command line and deal with
  // any symlinks, so any others we find should not be dereferenced
  if (lstat(fspc, &s) != 0) {
    perror(fspc);
    // but continue to process other arguments
    return;
  }
  if (S_ISDIR(s.st_mode)) {
    debug("%s is a directory", fspc);
    // Strip any trailing / with dirname
    processdir(fspc, nowstr, ttnow, days);
  } else if (S_ISREG(s.st_mode) || S_ISLNK(s.st_mode)) {
    debug("%s is a file or symlink", fspc);
    processfile(fspc, nowstr, ttnow, days);
  } else {
    printf("%s: ignored - not directory, symlink or file", fspc);
  }
}

void printhelp() {
  printf("usage: %s -d days dir dir ...\n", PROGNAME);
  printf("       %s dir dir ...\n\n", PROGNAME);
  printf("Files are tagged with the current time and date if not previously tagged.\n");
  printf("If -d is specified, files tagged longer ago than the specified number of days\nare deleted.\n");
}

int main(int argc, char *argv[]) {
  int ch;
  int days = 0;
  while ((ch = getopt(argc, argv, "h?d:")) != -1) {
    switch (ch) {
      case 'h':
      case '?':
        printhelp();
        exit(0);
      case 'd':
        days = atoi(optarg);
        debug("days = %d", days);
        break;
    }
  }
  argc -= optind;
  argv += optind;
  if (argc < 1) {
    printhelp();
    exit(1);
  }
  
  // Work out the current UTC date and time as both string and time_t.
  char strnow[ATTR_SIZE+1];
  time_t ttnow = time(NULL);
  struct tm *utc = gmtime(&ttnow);
  strftime(strnow, ATTR_SIZE, ATTR_FORMAT, utc);
  debug("Now = %s", strnow);
  
  for (int i = 0; i < argc; i++) {
    char fspc[PATH_MAX];
    realpath(argv[i], fspc);
    processpath(fspc, strnow, ttnow, days);
  }
  return 0;
}
