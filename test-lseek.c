#include "fcntl.h"
#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
	int fd, offset;
	int length, count;
	char *str;

	if(argc != 5) {
		printf(1, "Usage: ./test-lseek <file-to-read-from> <offset> <length> <string-to-test-against>\n");
		exit();
	}
		
	// File to read from
	fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		printf(1, "Error while reading file\n");
		exit();
	}

	// Seek to offset
	lseek(fd, atoi(argv[2]), SEEK_SET);

	// Read required bytes
	length = atoi(argv[3]);
	str = (char *) malloc(sizeof(char) * (length + 1));
	count = read(fd, str, length);
	str[count] = '\0';

	// Test
	if(strcmp(argv[4], str) == 0)
		printf(1, "Test Success\n");
	else
		printf(1, "Test Failed\n");

	free(str);
	close(fd);
	exit();
}
