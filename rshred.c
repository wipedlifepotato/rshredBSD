#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

static unsigned char count = 33;

void help(const char * nameprog) {
    printf("%s [options] file/dir\n", nameprog);
    puts("-c count [default 33]\n-h help\n-r recursive\n");
}

typedef enum {false=0, true=1} bool;

enum type_path { dir, file, err };

enum type_path getType(const char path[]) {
    struct stat sb;
    if (stat(path, &sb) == -1) return err;
    if (S_ISDIR(sb.st_mode)) return dir;
    return file;
}

int file_rewrite(const char * path) {
    FILE * f = fopen(path, "r+");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    if (s == 0) {
        fclose(f);
        return 0;
    }

    unsigned char *bytes = malloc(s);
    if (!bytes) {
        fclose(f);
        return -1;
    }

    for (int i = 0; i < count; i++) {
        for (size_t x = 0; x < s; x++) bytes[x] = rand() % 256;
        fseek(f, 0, SEEK_SET);
        fwrite(bytes, 1, s, f);
        fflush(f);
    }

    free(bytes);
    fclose(f);
    return 0;
}

int dir_rewrite(const char * path, bool recursive) {
    DIR * dirp = opendir(path);
    if (!dirp) return -1;

    struct dirent *entry;
    char p[1024];
    int errors = 0;

    while ((entry = readdir(dirp)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        snprintf(p, sizeof(p), "%s/%s", path, entry->d_name);
        enum type_path t = getType(p);
        if (t == file) {
            if (file_rewrite(p)) errors++;
        } else if (t == dir && recursive) {
            errors += dir_rewrite(p, true);
        }
    }
    closedir(dirp);
    return errors;
}

int main(int argc, char ** argv) {
    if (argc == 1) {
        help(argv[0]);
        return -1;
    }

    bool is_recursive = false;
    int ch;

    while ((ch = getopt(argc, argv, "hrc:")) != -1) {
        switch(ch) {
            case 'r': is_recursive = true; break;
            case 'c': count = (unsigned char)atoi(optarg); break;
            case 'h':
            default: help(argv[0]); return 0;
        }
    }

    if (optind >= argc) return -1;

    srand(time(NULL));
    const char * path = argv[optind];
    enum type_path t = getType(path);

    if (t == file) return file_rewrite(path);
    if (t == dir) return dir_rewrite(path, is_recursive);
    
    return -1;
}
