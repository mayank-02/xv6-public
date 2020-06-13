#include "fcntl.h"
#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
	int fd1, fd2;
	int size, chunksize, count;
	char *str;

	if(argc != 2) {
		printf(1, "Usage: ./test-length <file-to-read-from>\n");
		exit();
	}

	// File to read from
	fd1 = open(argv[1], O_RDONLY);
	if(fd1 == -1) {
		printf(1, "Error while reading file\n");
		exit();
	}

  // File to write to
	fd2 = open("copy.txt", O_CREATE | O_WRONLY);
	if(fd2 == -1) {
		printf(1, "Error while creating file\n");
		exit();
	}

	// Size of file
	size = lseek(fd1, 0, SEEK_END);
	printf(2, "Size of given file is %d\n", size);

	// Set file to write to of the required size
	str = (char *)malloc(sizeof(char) * size);
	memset(str, 0, size);
	write(fd2, str, size);

	// Divide the given file into 10 chunks
	if(size % 10 == 0)
		chunksize = size / 10;
	else
		chunksize = (size / 10) + 1;

	
  // Order of chunks : 2, 1, 4, 3, 6, 5, 8, 7, 10, 9
  int i = 2;
	str = (char *) malloc(sizeof(char) * (chunksize + 1));
	while(i <= 10) {
    // Seek
		lseek(fd1, (i - 1) * chunksize, SEEK_SET);
    // Read
		count = read(fd1, str, chunksize);
    // Set null
		str[count] = '\0';
    // Seek
		lseek(fd2, (i - 1) * chunksize, SEEK_SET);
		// Write
    write(fd2, str, count);
    // Next i
		if(i % 2 == 0)
			i--;
		else
			i += 3;
	}
	
	close(fd1);
	close(fd2);
	exit();
}