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
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;

	unsigned char		extended_section[54];
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
    uint16_s creation_date;
    uint16_s last_access;
    uint16_s height_word;
    uint16_s time_last_operation;
    uint16_s date_last_operation;
    uint16_s low_word;
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
    struct FATDirectuint16ory* subDirectory;

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
    int fat_read(uint16_s numCluster);
    int fat_write(uint16_s numCluster,uint16_s table_value);
    uint16_s allocateFreeFat();
    int deallocateCluster(uint16_s cluster_number);
    bool fat_cluster_end(uint16_s cluster);
    bool fat_cluster_bad(uint16_s cluster);
    bool free_cluster_fat(uint16_s cluster);

    //cluster
    uint16_s* cluster_read(uint16_s cluster_number);
    int cluster_write(void* write_data,uint16_s cluster_number);
    int setClusterFree(uint16_s cluster_number);
    int setClusterEnd(uint16_s cluster_number);

    //dir
    int dirList(const uint16_s cluster,uint8 dirrAttributes, bool exclusive);
    int dirSearch(const char* filepart,const uint16_s cluster ,dir_entry_t* file, uint16_s* entryOffset);
    int dirAdd(const uint16_s cluster, dir_entry_t* file_to_add);

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
