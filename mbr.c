#define _GNU_SOURCE

#include <stdio.h>

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

int main(int argc, char** argv) {
  const char* help = 
      "Usage: ./mbr [-gh] <device/partition file>\n"
      "\tRequires rw permissions of target file (root, most likely).\n\n"
      "\t-g: GPT partition - keep going even if not all patches can be found.\n"
      "\t-h (or -v): Print this message\n";

  char gpt = 0;
  int filepos = 1;
  int actualArgc = argc;
  for(unsigned char n = 1; n < argc; n++ )            /* Scan through args. */
       if(argv[n][0] == '-'){
         actualArgc--;
         if(filepos == n)
          filepos++;
         unsigned char x = 1;
         while(!gpt){
          switch (argv[n][x]){
            case 'h':
            case 'v':
              printf(help);
              return 0;

            case 'g':
              gpt = 1;
              break;
          }
            
          x++;
         }
       }
       
  if (actualArgc < 2) {
    fprintf(stderr, "ERROR: You must specify a file/partition.\n%s", help);
    return 1;
  }

  const char* partition = argv[filepos];
  int fd = open(partition, O_RDONLY);
  if (fd < 0) {
    fprintf(stderr, "Could not open for read %s: %s\n",
            partition, strerror(errno));
    return 1;
  }

  printf("opened: %s\n", partition);

  char buffer[1024 * 2];
  int data = read(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not read %s: %s\n",
            partition, strerror(errno));
    return 2;
  }
  close(fd);

  printf("read: %d bytes from %s\n", data, partition);

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

    size_t offset = (char*)ptr - buffer;
    printf("match[%d]: found at %ld, \"%s\", '%c'\n",
           i, offset, buffer + offset, buffer[offset]);

    ++found;
    buffer[offset] = '\0';
  }

  /* If we did not find all the required matches on a non gpt partition,
   * give up immediately, exit with status 10. */
  if (found < (sizeof(matches) / sizeof(matches[0])) && !gpt) {
    fprintf(stderr, "Not enough matches were found, giving up.\n");
    fprintf(stderr, "If you have a gpt disk, run this with the -g flag.\n");
    fprintf(stderr, "(if you run this command more than once, it's good!\n");
    fprintf(stderr, "it means the first run succeeded)\n");
    return 10;
  }
    
  /* On GPT partitions, it is ok to have partial matches. Keep going, but
   * warn the user about what to do next. */
  if (gpt) {
    switch (found) {
      case 1:
        printf("Found mbr header of gpt disk.\n"
               "Make sure to also run this on the ef02 partition of this drive.\n");
        break;

      case 2:
        printf("Found ef02 header of gpt grub partition.\n"
               "Make sure to also run this on full drive.\n");
        break;
    }
  } 

  fd = open(partition, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Could not open for write %s: %s\n",
            partition, strerror(errno));
    return 3;
  }

  data = write(fd, buffer, sizeof(buffer));
  if (data < 0) {
    fprintf(stderr, "Could not write %s, good luck: %s\n",
            partition, strerror(errno));
    return 4;
  }

  if (close(fd) < 0) {
    fprintf(stderr, "Close failed! Good luck: %s - %s\n",
            partition, strerror(errno));
    return 5;
  }

  printf("PATCHED SUCCESSFULLY!\n");
  printf("(the message should be gone next time you reboot, good luck!)\n");
  return 0;
}
