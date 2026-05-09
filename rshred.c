#include<stdio.h>
#include<stdlib.h>
#include<getopt.h>
#include<sys/stat.h>
#include<time.h>
#include<dirent.h>
#include<string.h>
//#define DEBUG 1

static unsigned char count = 33;
void help(const char * nameprog) {
	printf("%s [options] file/dir\n", nameprog);
	puts(		
			"-c count [default 33]\n"
			"-h help\n"
			"-r recursive\n"
	);
}
typedef enum {false=0,true=1} bool;
enum type_path {
	dir,
	file
};

enum type_path 
getType(const char path[]) {
	struct stat sb;
	if (stat(path, &sb) == -1) {
		perror("stat");
		puts("Try as file");
		return file;
	}
	if (S_ISDIR(sb.st_mode) ) {
		return dir;
	}
	return file;
}
int file_rewrite(const char * path) {

		size_t s = 0;
		FILE * f = fopen(path, "a+");
		if (!f) {
			return fprintf(stderr,"can't open file for get size");
		}
		s = ftell(f);
		fclose(f);
#ifdef DEBUG
		printf("Size file:%ld\n", s);
#endif
		if (s == 0) {
			return fprintf(stderr, "can't open file or file is nullsize\n");
		}
		f = fopen(path, "r+");
		if (!f) return fprintf(stderr, "can't open file for rewriting\n");
		for (int i = count;i--;) {
			unsigned char bytes[s];
			for (int x = 0;x< sizeof(bytes);x++) {
				bytes[x] = rand() % 255;
			}
			//bytes[s] = '\0';
#ifdef DEBUG
			printf("bytes: %s\n", bytes);
#endif
			size_t count_writed = fwrite(bytes, 1, sizeof(bytes) , f);
			if (count_writed != s) {
				fprintf(stderr, "WARNING: not writed size of file %zu:%zu\n", count_writed, s);
			}
			fseek(f, 0, SEEK_SET);
		}
		return 0;
}
int dir_rewrite(const char * path) {
		unsigned int count_errors = 0;
		DIR * dirp = opendir(path);
		if (!dirp) return fprintf(stderr, "Can't open dir\n");
	        struct dirent *entry;
		char p[1024];
		while ( (entry = readdir(dirp)) != NULL ) {
			if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
			snprintf(p, sizeof(p), "%s/%s", path, entry->d_name);
#ifdef DEBUG
			printf("path: %s\n", p);
#endif	
			if (getType(p) == file) {
				if (file_rewrite(p)) {
					fprintf(stderr, "WARNING: Can't rewrite file %s\n", p);
					count_errors++;
				}
			} else {
				return dir_rewrite(p);
			}

		}	
		return count_errors;
}
int 
main(int argC, char ** argm, char **env) {
	if (argC == 1) {
		help(argm[0]);
		return -1;
	}
	bool is_recursive = false;
	char ch;
	while ((ch = getopt(argC, argm, "hrc:")) != -1) {
		switch(ch) {
			case 'r':
				is_recursive = true;
				break;
			case 'c':
			     count = atoi(optarg) ;
			     break;
			case 'h':
			default:
				help(argm[0]);
				return fprintf(stderr, "%c unknown parametr\n", ch);
				break;
		}
	}
	srand(time(NULL));
	enum type_path t = getType(argm[argC-1]);
	const char * path = argm[argC-1];
	if (t == file) {
		if (file_rewrite(path)) {
			return fprintf(stderr, "error when rewriting file");
		}
	} else {
		return dir_rewrite(path);
	}
	return 0;

}
