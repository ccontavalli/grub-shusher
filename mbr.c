#define _GNU_SOURCE

#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {
  if (argc < 2) {
    fprintf(stderr, "You need to provide the name of a device to use\n");
    return 1;
  } 


  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Could not open for read %s: %s\n", argv[1], strerror(errno));
    return 1;
  }

  printf("opened: %s\n", argv[1]);

  char buffer[1024 * 2];
  int data = read(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not read %s: %s\n", argv[1], strerror(errno));
    return 2;
  }
  close(fd);

  printf("read: %d bytes from %s\n", data, argv[1]);

  # define DECLARE_MATCH(str) { str, sizeof(str) - 1 }
  struct {
    const char* match;
    const int length;
  } matches[] = {
    /* diskboot.S, line ~349 */
    DECLARE_MATCH("loading\0.\0\r\n\0Geom\0Read"),
    DECLARE_MATCH(".\0\r\n\0Geom\0Read"),
    /* boot.S, line ~391 */
    DECLARE_MATCH("GRUB \0Geom\0Hard Disk\0"),
    /* DECLARE_MATCH("Welcome to GRUB!"), */
  };

  int found = 0;
  for (int i = 0; i < sizeof(matches) / sizeof(matches[0]); ++i) {
    const char* match = matches[i].match;
    const int length = matches[i].length;

    void* ptr = memmem(buffer, data, match, length);
    if (!ptr) {
      printf("match[%d]: NOT FOUND\n", i);
      continue;
    }

    size_t offset = ptr - (void*)buffer;
    printf("match[%d]: found at %ld, \"%s\", '%c'\n",
           i, offset, buffer + offset, buffer[offset]);

    ++found;
    buffer[offset] = '\0';
  }

  if (found != sizeof(matches) / sizeof(matches[0])) {
    fprintf(stderr, "Not all required matches were found, giving up.\n");
    fprintf(stderr, "(if you run this command more than once, it's good!\n");
    fprintf(stderr, "it means the first run succeeded)\n");
    return 10;
  }

  fd = open(argv[1], O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Could not open for write %s: %s\n", argv[1], strerror(errno));
    return 3;
  }

  data = write(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not write %s, good luck: %s\n", argv[1], strerror(errno));
    return 4;
  }

  if (close(fd) < 0) {
    fprintf(stderr, "Close failed! Good luck: %s\n", argv[1], strerror(errno));
    return 5;
  }

  printf("SUCCESS! (maybe! you won't know until you reboot!)\n");
  return 0;
}
