#ifndef EXT2_H
#define EXT2_H
#include "../inc/types.h"

struct super_block
{
    uint32  inodes_count;
    uint32  blocks_count;
    uint32  r_blocks_count;
    uint32  free_blocks_count;
    uint32  free_inodes_count;
    uint32  first_data_block;
    uint32  log_block_size;
    uint32  log_frag_size;
    uint32  blocks_per_group;
    uint32  frags_per_group;
    uint32  inodes_per_group;

    uint32  mtime;
    uint32  wtime;

    uint16  mnt_count;
    uint16  max_mnt_count;
    uint16  magic;
    uint16  state;
    uint16  errors;
    uint16  minor_rev_level;
    
    uint32  last_check;
    uint32  check_interval;
    uint32  creator_os;
    uint32  rev_level;

    uint16  def_resuid;
    uint16  def_resgid;

}__attribute__ ((packed)) ;

struct ext2_bgdt
{
    uint32  block_bitMap;
    uint32  inode_bitMap;
    uint32  inode_table;

    uint16  free_blocks_count;
    uint16  free_inodes_count;
    uint16  used_dirs_count;
    uint16  pad[7];
}__attribute__ ((packed));

struct ext2_direntry
{
    uint32  inode;
    uint16  rec_len;
    uint8   name_len;
    uint8   file_type;
    char    name[];
}__attribute__ ((packed));

struct ext2_inode
{
    uint16  mode;
    uint16  uid;
    uint32  size;
    uint32  aTime;
    uint32  cTime;
    uint32  mTime;
    uint32  dTime;
    uint16  gid;
    uint16  links_count;
    uint32  blocks;
    uint32  flags;
    uint32  osd1;
    uint32  block[15];
    uint32  generation;
    uint32  file_acl;
    uint32  dir_acl;
    uint32  faddr;
    char    osd2[12];
}__attribute__ ((packed));

class ext2
{
private:
    /* data */
public:

};
#endif