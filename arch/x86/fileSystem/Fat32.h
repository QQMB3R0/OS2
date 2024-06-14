#ifndef FAT32_H
#define FAT32_H
#include "../inc/types.h"
#include "../io/GlobalObj.h"
#include "../drivers/ata.h"
#include "../memory/memory.h"
#define min(a,b)    ((a) < (b) ? (a) : (b))
#define	ATTR_READ_ONLY 0x01
#define	ATTR_HIDDEN 0x02
#define	ATTR_SYSTEM 0x04
#define	ATTR_VOLUME_ID 0x08
#define FILE_LONG_NAME (ATTR_READ_ONLY|ATTR_HIDDEN|ATTR_SYSTEM|ATTR_VOLUME_ID)
#define GET_CLUSTER_FROM_ENTRY(x) (x.low_word | (x.height_word << (32 / 2)))
#define	ATTR_DIRECTORY 0x10
#define	ATTR_ARCHIVE 0x20
#define	ATTR_LONG_NAME 0x0F
#define SECTORSIZE 512
#define DISK_READ_LOCATION 0x40000
#define END_CLUSTER_32 0x0FFFFFF8
#define BAD_CLUSTER_32 0x0FFFFFF7
#define FREE_CLUSTER_32 0x00000000
#define ENTRY_FREE 0xE5
#define ENTRY_END 0x00
#define BAD_CHARACTER 0x02
#define MANY_DOTS 0x10
#define NOT_CONVER_YET 0x08
#define LOWERCASE_ISSUE 0x01
struct Fat32Parameter_Block
{
    uint8  jmp[3];
    uint64 OEM_identifier;
    uint16 Bytes_per_sector;
    uint8  Nsectors_per_cluster;
    uint16 Nreserved_sectors;
    uint8  fat_number;
    uint16 Nroot_dir_entries;
    uint16 total_sector_16;
    uint8  media;
    uint16 dat_size;
    uint16 sectors_per_track;
    uint16 Nheads;
    uint32 hidden_sector;
	uint32 total_sector;

    unsigned char extended_section[54];
}__attribute__((packed));

typedef struct fat_extBS_32
{
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}
__attribute__((packed))
fat_extBS_32_t;
/*https://samag.ru/archive/article/245 :Форматы имен файлов*/
typedef struct dir_entry {
    char name[11];
    uint8 dir_attrs;
    uint8 reserved;
    uint8 times_tenths;
    uint8 creation_time;
    uint16 creation_date;
    uint16 last_access;
    uint16 height_word;
    uint16 time_last_operation;
    uint16 date_last_operation;
    uint16 low_word;
    uint32 byte_size_file;

}__attribute__((packed)) 
    dir_entry_t;
typedef struct Filestruct
{
    dir_entry_t file_meta;
    char* extesion;
    void* data_pointer;
    uint32 content;
    uint32* data;
    int size;
    char* name;
    struct Filestruct* next; 
}File;
typedef struct FATDirectory
{
    dir_entry_t file_meta;
    uint32 content;
    char* name;
    void* data_pointer;
    struct FATDirectory* next; 
    struct Filestruct* files; 
    struct FATDirectory* subDirectory;

//    struct FATDirectory* subDirectory;
} Directory;
typedef struct FATContent
{
    Directory* directory;
	File* file;

	uint32 parent_cluster;

}Content;

// REFACTOR
// I want to get rid of this from the header. This should be internal
// implementation, but for now, it's too convenient for stdio.c impl.

// EOC = End Of Chain
#define EOC 0x0FFFFFF8

struct f32 {
    //FILE *f;
    uint32 *FAT;
    struct Fat32Parameter_Block bpb;
    uint32 partition_begin_sector;
    uint32 fat_begin_sector;
    uint32 cluster_begin_sector;
    uint32 cluster_size;
    uint32 cluster_alloc_hint;
};

class Fat32
{
public:
    //fat
    int fat_read(uint16 numCluster);
    int fat_write(uint16 numCluster,uint16 table_value);
    uint16 allocateFreeFat();
    int deallocateCluster(uint16 cluster_number);
    bool fat_cluster_end(uint16 cluster);
    bool fat_cluster_bad(uint16 cluster);
    bool free_cluster_fat(uint16 cluster);

    //cluster
    uint16* cluster_read(uint16 cluster_number);
    int cluster_write(void* write_data,uint16 cluster_number);
    int setClusterFree(uint16 cluster_number);
    int setClusterEnd(uint16 cluster_number);

    //dir
    int dirList(const uint16 cluster,uint8 dirrAttributes, bool exclusive);
    int dirSearch(const char* filepart,const uint16 cluster ,dir_entry_t* file, uint16* entryOffset);
    int dirAdd(const uint16 cluster, dir_entry_t* file_to_add);

    //file
    Content* getFile(const char* filePath);
    int writeFile(Content* content, char* data);
    int setFile(const char* filePath, Content* content);

    unsigned short CurrentTime();
    unsigned char CurrentTimeTenths();
    unsigned short CurrentDate();
    unsigned char ChekSum(unsigned char *pFcbName);

    void convertFromFATFormat(char* in, char* out);
    char* convertToFATFormat(char* in);
    short checkNameFormat(const char* name);
    void addclustertocontent(Content* content);
    Content* FAT_create_content(char* name, bool directory, char* extension);

    dir_entry_t* FAT_create_entry(const char* name, const char* ext, bool isDir, uint32 firstCluster, uint32 filesize);


    int fat_init();
};
#endif
