#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IMGSIZE 1440*1024

unsigned char image[IMGSIZE];

struct BPB
{
    int BytsPerSec;
    int SecPerClus;
    int RsvdSecCnt;
    int NumFATs;
    int RootEntCnt;
    int FATSz16;
} bpb;

struct Entry
{
    char Name[9];
    char Ext[4];
    int Type; // 0 for file, 1 for folder
    int FstClus;
    struct Entry* Next;
    struct Entry* Children;
} root;

int get_int(int offset, int len)
{
    int result = 0;
    for (int i = offset + len - 1; i >= offset; --i) {
        result = result * 256 + image[i];
    }
    return result;
}

int get_next_clus(int curr_clus)
{
    int clus_id = curr_clus / 2;
    int fat_value = get_int(bpb.RsvdSecCnt * bpb.BytsPerSec + clus_id * 3, 24);
    if (curr_clus % 2 == 0) {
        return fat_value & 0x000fff;
    } else {
        return fat_value & 0xfff000 / 0x1000;
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
    int is_root = 1;
    int offset = (bpb.RsvdSecCnt + bpb.FATSz16 * bpb.NumFATs) * bpb.BytsPerSec;
    if (entry->FstClus >= 2) {
        // data area
        is_root = 0;
        offset += bpb.RootEntCnt * 32 + (entry->FstClus - 2) * bpb.BytsPerSec;
    }
    int finished = 0;
    for (int i = offset; finished == 0; i += 32) {
        if (image[i] == 0) {
            break;
        }
        struct Entry* new_entry = malloc(sizeof(struct Entry));
        // entry name
        int j;
        for (j = i; j < i + 8 && image[j] != 0x20; j++) {
            new_entry->Name[j-i] = image[j];
        }
        new_entry->Name[j-i] = 0;
        if (strcmp(new_entry->Name, ".") != 0 && strcmp(new_entry->Name, "..") != 0) {
            // file or folder
            if (image[i + 0xB] != 0x10) {
                // file
                new_entry->Type = 0;
                for (j = i + 8; j < i + 0xB && image[j] != 0x20; j++) {
                    new_entry->Ext[j - i - 8] = image[j];
                }
                new_entry->Ext[j - i - 8] = 0;
            } else {
                // folder
                new_entry->Type = 1;
            }
            strlwr(new_entry->Name);
            strlwr(new_entry->Ext);
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
            if (new_entry->Type == 1) {
                get_dir(new_entry);
            }
        }
        // finished?
        if (is_root == 1 && i >= offset + bpb.RootEntCnt * 32) {
            finished = 1;
        } else if (is_root == 0 && i >= offset + bpb.BytsPerSec * bpb.SecPerClus) {
            finished = 1;
        }
    }
}

void print_dir(struct Entry* entry, char* fullpath)
{
    struct Entry* ptr = entry->Children;
    while (ptr != NULL) {
        printf("%s%s", fullpath, ptr->Name);
        if (ptr->Type == 1) {
            printf("/\n");
            char* new_path = malloc(strlen(fullpath) + strlen(ptr->Name) + 2);
            strcpy(new_path, fullpath);
            strcat(new_path, ptr->Name);
            strcat(new_path, "/");
            print_dir(ptr, new_path);
            free(new_path);
        } else {
            printf(".%s\n", ptr->Ext);
        }
        ptr = ptr->Next;
    }
}

int main()
{
    load_img("test.img");
    get_bpb();
    get_dir(&root);
    print_dir(&root, "");
    return 0;
}
