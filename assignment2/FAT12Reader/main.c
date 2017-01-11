#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMGSIZE 1440*1024

struct BPB
{
    int BytsPerSec, SecPerClus, RsvdSecCnt, NumFATs, RootEntCnt, FATSz16;
} bpb;

struct Entry
{
    char Name[9], Ext[4];
    int FstClus, Type; // 0 for file, 1 for folder
    struct Entry *Next, *Children;
} root;

struct Count
{
    char* Line;
    int NumFile, NumDir;
};

unsigned char image[IMGSIZE];
void my_print(char* c);
void my_print_char(char c);

void my_strlwr(char* str) {
    int i;
    for (i = 0; str[i] != 0; ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] += 'a' - 'A';
        }
    }
}

void my_itoa(int i, char* a){
    char const digit[] = "0123456789";
    char* p = a;
    int shifter = i;
    do {
        p++;
        shifter /= 10;
    } while (shifter);
    *p = 0;
    do {
        *(--p) = digit[i % 10];
        i /= 10;
    } while (i);
}

int get_int(int offset, int len)
{
    int result = 0, i;
    for (i = offset + len - 1; i >= offset; --i) {
        result = result * 256 + image[i];
    }
    return result;
}

int get_next_clus(int curr_clus)
{
    int clus_id = curr_clus / 2,
        fat_value = get_int(bpb.RsvdSecCnt * bpb.BytsPerSec + clus_id * 3, 3);
    if (curr_clus % 2 == 0) {
        return fat_value & 0x000fff;
    } else {
        return (fat_value & 0xfff000) / 0x1000;
    }
}

void load_img(const char* filename)
{
    FILE *f;
    f = fopen(filename, "rb");
    fread(image, IMGSIZE, 1, f);
    fclose(f);
}

void get_bpb()
{
    bpb.BytsPerSec = get_int(11, 2);
    bpb.SecPerClus = get_int(13, 1);
    bpb.RsvdSecCnt = get_int(14, 2);
    bpb.NumFATs = get_int(16, 1);
    bpb.RootEntCnt = get_int(17, 2);
    bpb.FATSz16 = get_int(22, 2);
}

void get_dir(struct Entry* entry)
{
    int clus = entry->FstClus, is_root = 1, i,
        offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec;
    if (clus >= 2) {
        // in data area
        is_root = 0;
        offset += bpb.RootEntCnt * 32 + (clus - 2) * bpb.BytsPerSec * bpb.SecPerClus;
    }
    for (i = offset; ; i += 32) {
        // finished?
        if (is_root == 1 && i >= offset + bpb.RootEntCnt * 32) {
            break;
        } else if (is_root == 0 && i >= offset + bpb.BytsPerSec * bpb.SecPerClus) {
            if ((clus = get_next_clus(clus)) >= 0xff7) {
                break;
            } else {
                i = offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec
                        + bpb.RootEntCnt * 32 + (clus - 2) * bpb.BytsPerSec * bpb.SecPerClus;
            }
        }
        // ignore invaild entries
        if (image[i] != '.' && image[i] != 0 && image[i] != 5 && image[i] != 0xE5 && image[i + 0xB] != 0xF) {
            struct Entry* new_entry = malloc(sizeof(struct Entry));
            // entry name
            int j;
            for (j = i; j < i + 8 && image[j] != 0x20; j++) {
                new_entry->Name[j-i] = image[j];
            }
            new_entry->Name[j-i] = 0;
            my_strlwr(new_entry->Name);
            // file or folder
            if (image[i + 0xB] != 0x10) {
                // file
                new_entry->Type = 0;
                for (j = i + 8; j < i + 0xB && image[j] != 0x20; j++) {
                    new_entry->Ext[j - i - 8] = image[j];
                }
                new_entry->Ext[j - i - 8] = 0;
                my_strlwr(new_entry->Ext);
            } else {
                // folder
                new_entry->Type = 1;
            }
            new_entry->FstClus = get_int(i + 26, 2);
            new_entry->Next = NULL;
            new_entry->Children = NULL;
            // insert it
            if (entry->Children == NULL) {
                entry->Children = new_entry;
            } else {
                struct Entry* ptr = entry->Children;
                while (ptr->Next != NULL) {
                    ptr = ptr->Next;
                }
                ptr->Next = new_entry;
            }
            // recursively get dir
            if (new_entry->Type == 1) {
                get_dir(new_entry);
            }
        }
    }
}

void print_dir(struct Entry* entry, char* fullpath)
{
    struct Entry* ptr = entry->Children;
    while (ptr != NULL) {
        if (ptr->Type == 1 && ptr->Children != NULL) {
            char* new_path = malloc(strlen(fullpath) + strlen(ptr->Name) + 2);
            strcpy(new_path, fullpath);
            strcat(new_path, ptr->Name);
            strcat(new_path, "/");
            print_dir(ptr, new_path);
            free(new_path);
        } else {
            my_print("\x1b[36m");
            my_print(fullpath);
            if (ptr->Type == 0) {
                my_print("\x1b[0m");
                my_print(ptr->Name);
                my_print(".");
                my_print(ptr->Ext);
            } else {
                my_print(ptr->Name);
                my_print("\x1b[0m");
            }
            my_print("\n");
        }
        ptr = ptr->Next;
    }
}

void print_file(struct Entry* entry) {
    int clus = entry->FstClus;
    while (1) {
        int i, offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec
                + bpb.RootEntCnt * 32 + (clus - 2) * bpb.BytsPerSec * bpb.SecPerClus;
        for (i = 0; i < bpb.BytsPerSec * bpb.SecPerClus; ++i) {
            my_print_char(image[offset + i]);
        }
        if ((clus = get_next_clus(clus)) >= 0xff7) {
            break;
        }
    }
}

int find_file(struct Entry* entry, char* path, char* target)
{
    struct Entry* ptr = entry->Children;
    int result = 0;
    while (ptr != NULL) {
        char* new_path = malloc(strlen(path) + strlen(ptr->Name) + strlen(ptr->Ext) + 2);
        strcpy(new_path, path);
        strcat(new_path, ptr->Name);
        if (ptr->Type == 1) {
            if (strcmp(new_path, target) == 0) {
                strcat(new_path, "/");
                print_dir(ptr, new_path);
                free(new_path);
                return 1;
            }
            strcat(new_path, "/");
            result += find_file(ptr, new_path, target);
        } else {
            strcat(new_path, ".");
            strcat(new_path, ptr->Ext);
            if (strcmp(new_path, target) == 0) {
                print_file(ptr);
                free(new_path);
                return 1;
            }
        }
        free(new_path);
        ptr = ptr->Next;
    }
    return result;
}

struct Count* count_dir(struct Entry* entry, int depth)
{
    struct Count* count = malloc(sizeof(struct Count));
    count->Line = malloc(1);
    count->Line[0] = 0;
    count->NumFile = 0;
    count->NumDir = 0;
    struct Entry* ptr = entry->Children;
    while (ptr != NULL) {
        if (ptr->Type == 1) {
            struct Count* new_count = count_dir(ptr, depth + 1);
            count->NumFile += new_count->NumFile;
            count->NumDir += new_count->NumDir + 1;
            char* new_line = malloc(strlen(count->Line) + strlen(new_count->Line) + 1);
            strcpy(new_line, count->Line);
            strcat(new_line, new_count->Line);
            free(new_count->Line);
            free(new_count);
            free(count->Line);
            count->Line = new_line;
        } else {
            count->NumFile++;
        }
        ptr = ptr->Next;
    }
    char* new_line = malloc(strlen(count->Line) + 128);
    new_line[0] = 0;
    int i;
    for (i = 0; i < depth; ++i) {
        strcat(new_line, "  ");
    }
    strcat(new_line, entry->Name);
    strcat(new_line, " : ");
    my_itoa(count->NumFile, new_line + strlen(new_line));
    strcat(new_line, " files, ");
    my_itoa(count->NumDir, new_line + strlen(new_line));
    strcat(new_line, " directories\n");
    strcat(new_line, count->Line);
    free(count->Line);
    count->Line = new_line;
    return count;
}

int find_dir(struct Entry* entry, char* path, char* target)
{
    struct Entry* ptr = entry->Children;
    int result = 0;
    while (ptr != NULL) {
        if (ptr->Type == 1) {
            char* new_path = malloc(strlen(path) + strlen(ptr->Name) + strlen(ptr->Ext) + 2);
            strcpy(new_path, path);
            strcat(new_path, ptr->Name);
            if (strcmp(new_path, target) == 0) {
                struct Count* count = count_dir(ptr, 0);
                my_print(count->Line);
                free(count->Line);
                free(count);
                free(new_path);
                return 1;
            }
            strcat(new_path, "/");
            result += find_dir(ptr, new_path, target);
            free(new_path);
        }
        ptr = ptr->Next;
    }
    return result;
}

int main()
{
    load_img("test.img");
    get_bpb();
    get_dir(&root);
    print_dir(&root, "");
    char command[1024];
    while (1) {
        my_print("\n> Command: ");
        fgets(command, 1024, stdin);
        int len = strlen(command);
        if (command[len - 1] == '\n') {
            command[len - 1] = 0;
        }
        my_strlwr(command);
        len = strlen(command);
        if (command[len - 1] == '/') {
            command[len - 1] = 0;
        }
        if (strcmp(command, "exit") == 0) {
            return 0;
        }
        if (strstr(command, "count ") == command) {
            int result = find_dir(&root, "", command + 6);
            if (result <= 0) {
                my_print(command + 6);
                my_print(" is not a directory!\n");
            }
        } else {
            int result = find_file(&root, "", command);
            if (result <= 0) {
                my_print("Unknown file\n");
            }
        }
    }
    return 0;
}
