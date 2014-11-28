#define _GNU_SOURCE

#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {
  char gpt = 0;
  char filepos = 1;
  int actualArgc = argc;
  for(unsigned char n = 1; n < argc; n++ )            /* Scan through args. */
       if(argv[n][0] == '-'){
         actualArgc--;
         if(filepos == n)
          filepos++;
         if(argv[n][1] == 'g')
          gpt = 1;
       }
       
  if (actualArgc < 2) {
    fprintf(stderr, "You need to provide the name of a device to use\n");
    return 1;
  }
  

  int fd = open(argv[filepos], O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Could not open for read %s: %s\n", argv[filepos], strerror(errno));
    return 1;
  }

  printf("opened: %s\n", argv[filepos]);

  char buffer[1024 * 2];
  int data = read(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not read %s: %s\n", argv[filepos], strerror(errno));
    return 2;
  }
  close(fd);

  printf("read: %d bytes from %s\n", data, argv[filepos]);

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
  
  if (found == 0 || (found < sizeof(matches) / sizeof(matches[0]) && !gpt)) {
    fprintf(stderr, "Not enough matches were found, giving up.\nIf you have a gpt disk try running this with the -g flag.");
    fprintf(stderr, "(if you run this command more than once, it's good!\n");
    fprintf(stderr, "it means the first run succeeded)\n");
    return 10;
  }else if (gpt){
    if(found == 1)
      printf("Found mbr header of gpt disk.\nMake sure to also run this on the ef02 partition of this drive.");
    else if(found == 2)
      printf("Found ef02 header of gpt grub partition.\nMake sure to also run this on full drive.");
    gpt=1;
  }

  fd = open(argv[filepos], O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Could not open for write %s: %s\n", argv[filepos], strerror(errno));
    return 3;
  }

  data = write(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not write %s, good luck: %s\n", argv[filepos], strerror(errno));
    return 4;
  }

  if (close(fd) < 0) {
    fprintf(stderr, "Close failed! Good luck: %s\n", argv[filepos], strerror(errno));
    return 5;
  }
  
  printf("SUCCESS! (maybe! you won't know until you reboot!)\n");
  return 0;
}
