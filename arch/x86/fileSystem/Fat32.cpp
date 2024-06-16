#include "Fat32.h"

uint16_s fat_size;
uint16_s fat_type;
uint16_s total_cluster;
uint16_s total_sectors;

uint16_s bytes_per_secors;
uint16_s secors_per_cluster;

uint16_s first_fat_sector;
uint16_s first_data_sector;
uint16_s ext_root_cluster;
Fat32Parameter_Block bootsect;

int Fat32::fat_init()
{
    AtaDriver ata_d ;
    if (!ata_d.is_drive_exist(0))
    {
        display << "Function FAT_initialize: drive not exist!";
        return NULL;
    }else
    {
        ata_d.identify(0);
    }
    
    uint16_s* cluster_data = ata_d.get_identify_buf();
    Fat32Parameter_Block* bootstruct;
    bootstruct->bytes_per_sector = cluster_data[5];
    bootstruct->sectors_per_track= cluster_data[6];
    bootstruct->sectors_per_cluster = 2;
    bootstruct->total_sectors_32 = cluster_data[61];
    bootstruct->reserved_sector_count = 2;
    bootstruct->table_count = 2;
    bootstruct->root_entry_count = 0;
    memcpy(&bootsect,bootstruct,512);
    fat_type = 32;
    total_cluster = bootstruct->total_sectors_32 / bootstruct->bytes_per_sector;
    first_fat_sector = bootstruct->reserved_sector_count;
    ext_root_cluster = 2;
    //bootstruct->bytes_per_sector = *((unsigned short*)(cluster_data + 11));
    //Fat32Parameter_Block* bootstruct = (Fat32Parameter_Block*)cluster_data;
    //total_sectors = bootstruct->total_sectors_32;
    //display<<"Function FAT_initialize: bytes_per_sector:" << (uint32)bootstruct->bytes_per_sector<<'\n';
    //fat_size = ((fat_extBS_32_t*)bootstruct->extended_section)->table_size_32;
    //display<<"Function FAT_initialize: sectors_per_track:" << (uint32)bootstruct->sectors_per_track<<'\n';
    //display<<"Function FAT_initialize: total_sectors_32:" << (uint32)bootstruct->total_sectors_32<<'\n';
    //display<<"Function FAT_initialize: reserved_sector_count:" << (uint32)bootstruct->reserved_sector_count<<'\n';
    
    //int root_dir_sectors = ((bootstruct->Nroot_dir_entries * 32) + (bootstruct->Bytes_per_sector - 1)) / bootstruct->Bytes_per_sector;
    //int data_sectors     = total_sectors - (bootstruct->Nreserved_sectors + (bootstruct->fat_number * fat_size) + root_dir_sectors);

    
    //total_cluster    = data_sectors / bootstruct->Nsectors_per_cluster;
	//first_data_sector = bootstruct->Nreserved_sectors + bootstruct->fat_number * bootstruct->dat_size + (bootstruct->Nroot_dir_entries * 32 + bootstruct->Bytes_per_sector - 1) / bootstruct->Bytes_per_sector;
	//if (total_cluster == 0) total_cluster  = bootstruct->total_sector / bootstruct->Nsectors_per_cluster;
	//fat_type = 32;
	//first_data_sector = bootstruct->Nreserved_sectors + bootstruct->fat_number * ((fat_extBS_32_t*)(bootstruct->extended_section))->table_size_32;
	//secors_per_cluster = bootstruct->Nsectors_per_cluster;
	//bytes_per_secors    = bootstruct->Bytes_per_sector;
	//ext_root_cluster    = ((fat_extBS_32_t*)(bootstruct->extended_section))->root_cluster;
	//memcpy(&bootsect, bootstruct, 512);
	//first_fat_sector    = bootstruct->Nreserved_sectors;

	kfree(cluster_data);
    return 0;
}


/*from https://wiki.osdev.org/FAT#FAT_32 (FAT 32 and exFAT)*/
int Fat32::fat_read(uint16_s numCluster)
{
    if (numCluster < 2 || numCluster > total_cluster)
    {
        display << "Function FATRead: invalid cluster number!\n";
        return -1;
    }
    AtaDriver ata_d;
    if (fat_type == 32)
    {
        uint16_s cluster_size = bytes_per_secors * secors_per_cluster;
        uint16_s fat_offset = numCluster * 4;
        uint16_s fat_sector = first_fat_sector + (fat_offset / cluster_size);
        uint16_s ent_offset = fat_offset % cluster_size;
        uint16_s* data_clusters = (uint16_s *)ata_d.ata_read_sector(fat_sector, numCluster);

        if (data_clusters == NULL)
        {
            display<<"Function FAT_read: Could not read sector that contains FAT32 table entry needed.\n";
            return -1;
        }
        uint16_s table_value = data_clusters[ent_offset];
        table_value &= 0x0FFFFFFF;//ignore hight 4bits

        kfree(data_clusters);

        return table_value;

    }
    display << "Function FAT_read: Invalid fat_type value";
    return -1;
}

int Fat32::fat_write(uint16_s numCluster, uint16_s table_value)
{
    if (numCluster < 2 || numCluster > total_cluster)
    {
        display << "Function FATRead: invalid cluster number!\n";
        return -1;
    }
    AtaDriver ata_d;
    if (fat_type == 32)
    {
        uint16_s cluster_size = bytes_per_secors * secors_per_cluster;
        uint16_s fat_offset = numCluster * 4;
        uint16_s fat_sector = first_fat_sector + (fat_offset / cluster_size);
        uint16_s ent_offset = fat_offset % cluster_size;

        uint16_s* data_clusters = (uint16_s *)ata_d.ata_read_sector(fat_sector, numCluster);

        if (data_clusters == NULL)
        {
            display<<"Function FAT_write: Could not read sector that contains FAT32 table entry needed.\n";
            return -1;
        }
        data_clusters[ent_offset] = table_value;
        if (ata_d.ata_write_sector(fat_sector,secors_per_cluster,(const char *)data_clusters)!=1)
        {
            display << "Function FAT_write: Could not write new FAT32 cluster number to sector.\n";
            return -1;
        }
        
        kfree(data_clusters);
        return 0;
    }
    display << "Function FAT_read: Invalid fat_type value";
    return -1;
}

uint16_s Fat32::allocateFreeFat()
{
    uint16_s cluster = 2;
	uint16_s clusterStatus = FREE_CLUSTER_32;
    while (cluster < total_cluster)
    {
        clusterStatus = fat_read(cluster);
        if (free_cluster_fat(clusterStatus) == true)
        {
            //cluster found, allocate it.
			if (fat_write(cluster, END_CLUSTER_32) == 0)
				return cluster;
			else
			{
				display<<"Function allocateFreeFAT: Error occurred with FATWrite, aborting operations...\n";
				return BAD_CLUSTER_32;
			}
		}
		else if (clusterStatus < 0)
		{
			display<<"Function allocateFreeFAT: Error occurred with FATRead, aborting operations...\n";
			return BAD_CLUSTER_32;
		}

		cluster++; //cluster is taken, check the next one
        
    }
    
    return BAD_CLUSTER_32;
}

int Fat32::deallocateCluster(uint16_s cluster_number)
{
    uint16_s status = fat_read(cluster_number);
    if (free_cluster_fat(status) == true) 
        return 0;
    else if (status < 0)
    {
        display << "Function FAT_cluster_deallocate: Error occurred with FAT_read, aborting operations...\n";
        return -1;
    }
    if (setClusterFree(cluster_number) == 0) return 0;
    else
    {
        display << "Function FAT_cluster_deallocate: Error occurred with FAT_write, aborting operations...\n";
        return -1;
    }
    
}

bool Fat32::fat_cluster_end(uint16_s cluster)
{
    if (cluster == END_CLUSTER_32)
        return true;
    
    return false;
}

bool Fat32::fat_cluster_bad(uint16_s cluster)
{
    if (cluster == BAD_CLUSTER_32)
        return true;    
    return false;
}

bool Fat32::free_cluster_fat(uint16_s cluster)
{
    if(cluster == FREE_CLUSTER_32)
        return true;
    return false;
}

uint16_s* Fat32::cluster_read(uint16_s cluster_number)
{
    if (cluster_number < 2 || cluster_number > total_cluster)
    {
        display << "Function cluster_read: invalid cluster number!\n";
        return NULL;
    }
    AtaDriver ata_d;

   uint16_s start_sec = (cluster_number - 2) * secors_per_cluster + first_data_sector;
   uint16_s* data_clusster = (uint16_s *)ata_d.ata_read_sector(start_sec,secors_per_cluster);
    if (data_clusster == NULL)
    {
        display<<"Function FAT_write: Could not read sector that contains FAT32 table entry needed.\n";
        return NULL;
    }
    return data_clusster;
}

int Fat32::cluster_write(void* write_data, uint16_s cluster_number)
{
    if (cluster_number < 2 || cluster_number > total_cluster)
    {
        display << "Function cluster_write: invalid cluster number!\n";
        return -1;
    }
    AtaDriver ata_d;

    uint16_s start_sec = (cluster_number - 2) * secors_per_cluster + first_data_sector;
    if (ata_d.ata_write_sector(start_sec, secors_per_cluster, (const char *)write_data)!=1)
    {
        display << "Function cluster_write: An error occured with ata_write_sector, the area in sector";
        return -1;
    }
    
    return 0;
}

int Fat32::setClusterFree(uint16_s cluster_number)
{
    if (cluster_number == FREE_CLUSTER_32)
    return fat_write(cluster_number, FREE_CLUSTER_32);
    
}

int Fat32::setClusterEnd(uint16_s cluster_number)
{
    if (cluster_number == END_CLUSTER_32)
    return fat_write(cluster_number, END_CLUSTER_32);
}

int Fat32::dirList(const uint16_s cluster, uint8 dirrAttributes, bool exclusive)
{
    uint16_s* data_clusster = cluster_read(cluster);
    dir_entry_t* metadata_file = (dir_entry_t*)data_clusster;
    uint16_s meta_pointer_it_count = 0;
    if (data_clusster == NULL)
    {
        display<<"Function dirList: Could not read sector that contains FAT32 table entry needed.\n";
        return -1;
    }
    while (1)
    {
        if (metadata_file->name[0] == ENTRY_END)
            break;
        if (strncmp(metadata_file->name,"..",2)== 0||strncmp(metadata_file->name,".",1)== 0)
        {
            metadata_file++;
            meta_pointer_it_count++;
        }
        else if (metadata_file->name[0] == ENTRY_FREE || (metadata_file->dir_attrs & FILE_LONG_NAME )==FILE_LONG_NAME)
        {
            if (meta_pointer_it_count < bytes_per_secors * secors_per_cluster / sizeof(dir_entry_t) < 1)
            {
                metadata_file++;
                meta_pointer_it_count++;
            }
            else
            {
                uint16_s next_cluster = fat_read(cluster);
                if (fat_cluster_end(next_cluster)==true)
                break;
                else if (next_cluster < 0)
                {
                    display << "Function directoryList: FATRead encountered an error. Aborting...\n";
                    return -1;
                }
                else
                dirList(cluster, dirrAttributes, exclusive);
                
            }
            
        }
        else
        {
            char conver[13];
            convertFromFATFormat((char*)metadata_file->name, conver);
            display << conver;
            display << "\t";
            if ((metadata_file->dir_attrs & ATTR_DIRECTORY)!=ATTR_DIRECTORY)
                display << metadata_file->byte_size_file;
            else
                display << "\t";
            display << "\t";

            if ((metadata_file->dir_attrs & ATTR_DIRECTORY)==ATTR_DIRECTORY)
                display << "DIR";
            display << "\n";
            
            metadata_file++;
            meta_pointer_it_count++;
        }
    }
    
    return 0;
}

int Fat32::dirSearch(const char *filepart, const uint16_s cluster, dir_entry_t *file, uint16_s *entryOffset)
{
    if (cluster < 2 || cluster > total_cluster)
    {
        display << "Function dirSearch: invalid cluster number!\n";
        return -1;
    }
    char searchName[13] = {'\0'};
    strcpy(searchName,filepart);
    if (checkNameFormat(searchName)!=0)
        convertToFATFormat(searchName);
    uint16_s* data_clusster = cluster_read(cluster);
    if (data_clusster == NULL)
    {
        display<<"Function dirSearch: Could not read sector that contains FAT32 table entry needed.\n";
        return -1;
    }
    dir_entry_t* metadata_file = (dir_entry_t*)data_clusster;
    uint16_s meta_pointer_it_count = 0;
    while (1)
    {
        if (metadata_file->name[0] == ENTRY_END)
            break;
        else if (strncmp((char*)metadata_file->name, searchName, 11) != 0)
        {
            if (meta_pointer_it_count < bytes_per_secors * secors_per_cluster / sizeof(dir_entry_t) < 1)
            {
                metadata_file++;
                meta_pointer_it_count++;
            }
            else
            {
                uint16_s next_cluster = fat_read(cluster);
                next_cluster = allocateFreeFat();
                if(fat_cluster_end(next_cluster) == true) break;
                else if (next_cluster < 0)
                {
                    display << "Function directorySearch: FATRead encountered an error. Aborting...\n";
                    kfree(data_clusster);
                    return -1;
                }
                else
                    return dirSearch(filepart, cluster, file, entryOffset);
            }
            
        }
        else
        {
            if (file != NULL)
                memcpy(file, filepart, sizeof(dir_entry_t));
            if (entryOffset != NULL)
                *entryOffset = meta_pointer_it_count;
        
            return 0;
        }
        
    }
    return -2;
}

int Fat32::dirAdd(const uint16_s cluster, dir_entry_t *file_to_add)
{
    AtaDriver ata_d;
    uint16_s* data_clusster = cluster_read(cluster);
    if (data_clusster == NULL)
    {
        display<<"Function dirAdd: Could not read sector that contains FAT32 table entry needed.\n";
        return -1;
    }
    dir_entry_t* metadata_file = (dir_entry_t*)data_clusster;
    uint16_s meta_pointer_it_count = 0;
    while (1)
    {
        if (metadata_file->name[0] != ENTRY_FREE && metadata_file->name[0] != ENTRY_END)
        {
            if (meta_pointer_it_count < bytes_per_secors * secors_per_cluster / sizeof(dir_entry_t) < 1)
            {
                metadata_file++;
                meta_pointer_it_count++;
            }
            else
            {
                uint16_s next_cluster = fat_read(cluster);
                if (fat_cluster_end(next_cluster) == true)
                {
                    next_cluster = allocateFreeFat();
                    if (fat_cluster_bad(next_cluster) == true)
                    {
                        display << "Function dirAdd: allocation of new cluster failed. Aborting...\n";
                        kfree(data_clusster);
                        return -1;
                    }
                    if (fat_write(cluster, next_cluster) != 0)
                    {
                        display << "Function dirAdd: extension of the cluster chain with new cluster failed. Aborting...\n";
                        kfree(data_clusster);
                        return -1;
                    }
                    
                }
                kfree(data_clusster);
                dirAdd(next_cluster, file_to_add);
            }
            
        }else
        {
            file_to_add->creation_date = 0x00;
            file_to_add->creation_time = 0x00;
            file_to_add->times_tenths = 0x00;
            file_to_add->last_access = file_to_add->creation_date;
            file_to_add->time_last_operation = file_to_add->creation_date;
            file_to_add->date_last_operation = file_to_add->creation_time;

            uint16_s cluster_to_file = allocateFreeFat();
            display << "Function dirAdd: the new cluster for the file is ";
            if (fat_cluster_bad(cluster_to_file)== true)
            {
                display << "Function FAT_directory_add: allocation of new cluster failed. Aborting...\n";
                kfree(data_clusster);
                return -1;
            }
            file_to_add->low_word = cluster_to_file & 0xF;
            file_to_add->height_word = cluster_to_file >> 0x10;	
			memcpy(metadata_file, file_to_add, sizeof(dir_entry_t));

            if (cluster_write(data_clusster, cluster) != 0) {
					display<<"Function FAT_directory_add: Writing new directory entry failed. Aborting...\n";
					kfree(data_clusster);
					return -1;
				}
            kfree(data_clusster);
			return 0;
        }
        
        
    }
    kfree(data_clusster);
	return -1;
}

Content *Fat32::getFile(const char *filePath)
{
    	Content* fatContent = (Content*)kmalloc(sizeof(Content));

		fatContent->directory = NULL;
		fatContent->file 	  = NULL;

		char fileNamePart[256];
		unsigned short start = 0;
		unsigned int active_cluster;
		if (fat_type == 32) active_cluster = ext_root_cluster;
		else {
            display << "Function getFile: FAT16 and FAT12 are not supported!\n";
			return NULL;
		}
		dir_entry_t content_meta;
		for (unsigned int iterator = 0; iterator <= strlen(filePath); iterator++) 
        {
			if (filePath[iterator] == '\\' || filePath[iterator] == '\0') {
				memset(fileNamePart, '\0', 256);
				memcpy(fileNamePart, filePath + start, iterator - start);

				int retVal = dirSearch(fileNamePart, active_cluster, &content_meta, NULL);
				if (retVal == -2) {
					return NULL;
				}

				else if (retVal == -1) {
					display << "Function getFile: An error occurred in FAT_directory_search. Aborting...\n";
					return NULL;
				}

				start = iterator + 1;
				active_cluster = GET_CLUSTER_FROM_ENTRY(content_meta);
				if (filePath[iterator] != '\0') fatContent->parent_cluster = active_cluster;
			}
        }
        if ((content_meta.dir_attrs & ATTR_DIRECTORY) != ATTR_DIRECTORY) {
			fatContent->file 		       = (File *)kmalloc(sizeof(File));
			fatContent->file->name         = (char*)kmalloc(8);
			fatContent->file->extesion    = (char*)kmalloc(4);
			fatContent->file->next         = NULL;

			uint32* content = NULL;
			int content_size = 0;
			
			int cluster = GET_CLUSTER_FROM_ENTRY(content_meta);
			while (cluster < END_CLUSTER_32) {
				uint32* new_content = (uint32*)krealloc(content, (content_size + 1) * sizeof(uint32));
				if (new_content == NULL) {
					kfree(content);
					return NULL;
				}

				new_content[content_size] = cluster;

				content = new_content;
				content_size++;

				cluster = fat_read(cluster);
				if (cluster == BAD_CLUSTER_32) {
					display <<"Function getFile: the cluster chain is corrupted with a bad cluster. Aborting...\n";
					kfree(content);
					return NULL;
				} 
				
				else if (cluster == -1) {
					display<<"Function getFile: an error occurred in FAT_read. Aborting...\n";
					kfree(content);
					return NULL;
				}
			}
			
			fatContent->file->data = (uint32*)kmalloc(content_size * sizeof(uint32));
			memcpy(fatContent->file->data, content, content_size * sizeof(uint32));
			fatContent->file->size = content_size;
			kfree(content);

			fatContent->file->file_meta = content_meta;

			char* name = (char*)kmalloc(13);
			char* name_pointer = name;

			strcpy(name, fatContent->file->file_meta.name);
			strncpy(fatContent->file->name, strtok(name, " "), 8);
			strncpy(fatContent->file->extesion, strtok(NULL, " "), 4);

			kfree(name_pointer);
			return fatContent;
        }
        else
        {
            display << "Function getFile: it is a Directory\n";
            //fatContent->directory = (Directory*)kmalloc(sizeof(Directory));
            //fatContent->directory->file_meta 	= content_meta;
			//fatContent->directory->name             = (char*)kmalloc(11);
			//fatContent->directory->files            = NULL;
			//fatContent->directory->subDirectory     = NULL;
			//fatContent->directory->next             = NULL;
			//fatContent->directory->data_pointer     = NULL;
			
			//char* name = (char*)kmalloc(13);
			//char* name_pointer = name;
			
			//strcpy(name, content_meta.name);
			//strncpy(fatContent->directory->name, strtok(name, " "), 11);

			//kfree(name_pointer);
			
			//return fatContent;

        }
        
}

int Fat32::writeFile(Content *content, char *data)
{
	//=====================
		// CONTENT META SAVING

			dir_entry_t content_meta;
			if (content->directory != NULL) content_meta = content->directory->file_meta;
			else if (content->file != NULL) content_meta = content->file->file_meta;

		// CONTENT META SAVING
		//=====================
		// EDIT DATA
			
			unsigned int cluster = GET_CLUSTER_FROM_ENTRY(content_meta);
			unsigned int dataLeftToWrite = strlen(data);
			unsigned int allData = dataLeftToWrite;

			while (cluster <= END_CLUSTER_32) {
				unsigned int dataWrite = 0;
				
				if (dataLeftToWrite >=bytes_per_secors  * secors_per_cluster) dataWrite = bytes_per_secors  * secors_per_cluster + 1;
				else dataWrite = dataLeftToWrite;

				char* sector_data = (char*)kmalloc(dataWrite + 1);
				memset(sector_data, 0, dataWrite + 1);
				strncpy(sector_data, data, dataWrite);

				data += dataWrite;

				if (fat_cluster_bad(cluster) == true) {
					display<<"Function FAT_write_content: the cluster chain is corrupted with a bad cluster. Aborting...\n";
					return -1;
				}
				else if (cluster == -1 ) {
					display << "Function FAT_write_content: an error occurred in FAT_read. Aborting...\n";
					return -1;
				}

				if (dataLeftToWrite > 0) addclustertocontent(content);
				else if (dataLeftToWrite <= 0) {
					unsigned int prevCluster = cluster;
					unsigned int endCluster  = cluster;
					while (cluster < END_CLUSTER_32) {
						prevCluster = cluster;
						cluster = fat_read(cluster);
						
						if (fat_cluster_bad(cluster) == true) {
							display << "Function FAT_write_content: allocation of new cluster failed. Aborting...\n";
							return -1;
						}

						if (deallocateCluster(prevCluster) != 0) {
							display << "Deallocation problems.\n";
							break;
						}
					}
					
					if (fat_cluster_end(endCluster) != true) 
						setClusterEnd(endCluster);
					
					break;
				}

				char* previous_data = (char*)cluster_read(cluster);
				if (strstr(previous_data, sector_data) != 0) {
                    char clear[secors_per_cluster];
		            memset(clear, 0, secors_per_cluster);
                    uint16_s start_sect = (cluster - 2) * (unsigned short)secors_per_cluster + first_data_sector;
                    AtaDriver atad;
                    atad.ata_write_sector(start_sect, secors_per_cluster, (const char *)clear);
					if (cluster_write(sector_data, cluster) != 0) {
						display << "Function FAT_write_content: FAT_cluster_write encountered an error. Aborting...\n";
						kfree(previous_data);
						return -1;
					}

					kfree(previous_data);
				}

				dataLeftToWrite -= dataWrite;
				if (dataLeftToWrite == 0) setClusterEnd(cluster);
				
				if (dataLeftToWrite < 0) {
					display << "Function FAT_write_content: An undefined value has been detected. Aborting...\n";
					return -1;
				}
			}

			return 0;
		
		// EDIT DATA
		//=====================

		return 0;
    }

int Fat32::setFile(const char *filePath, Content *content)
{
		// CONTENT META SAVING

			dir_entry_t content_meta;
			if (content->directory != NULL) content_meta = content->directory->file_meta;
			else if (content->file != NULL) content_meta = content->file->file_meta;

		// CONTENT META SAVING
		//====================
			
		char fileNamePart[256];
		unsigned short start = 0;

		//====================
		//	FINDING DIR BY PATH

			Content* directory = getFile(filePath);
			if (directory->directory == NULL) return -1;

			dir_entry_t file_info = directory->directory->file_meta;
			unsigned int active_cluster = GET_CLUSTER_FROM_ENTRY(file_info);

		//	FINDING DIR\FILE BY PATH
		//====================
		// CHECK IF FILE EXIST
		// A.i.: directory to receive the file is now found, and its cluster 
		// is stored in active_cluster. Search the directory to ensure the 
		// specified file name is not already in use
		
			char output[13];
			convertFromFATFormat((char*)content_meta.name, output);
			int retVal = dirSearch(output, active_cluster, NULL, NULL);
			if (retVal == -1) {
				display<<"Function putFile: directorySearch encountered an error. Aborting...\n";
				return -1;
			}
			else if (retVal != -2) {
				display<<"Function putFile: a file matching the name given already exists. Aborting...\n";
				return -3;
			}

		// CHECK IF FILE EXIST
		//====================

		if (dirAdd(active_cluster, &content_meta) != 0) {
			display<<"Function FAT_put_content: FAT_directory_add encountered an error. Aborting...\n";
			return -1;
		}
    return 0;
}

void Fat32::convertFromFATFormat(char *in, char *out)
{
    if (in[0] == '.')
    {
        if (in[1]=='.')
        {
            strcpy(out, "..");
            return;
        }
        strcpy(out,".");
        return;
    }
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        if (in[i] == 0x20)// '\t'
        {
            out[i] == '.';
            break;
        }
        out[i] = in[i];
    }
    if (i == 8)
    {
        out[i] = '.';
    }
    int i2 = 0;
    for (i2 = 8; i2 < 11; i2++)
    {
        i++;
        if (in[i2] == 0x20)
        {
            if (i2 == 8)
                i -= 2;
            break;
        }
        out[i] = in[i2];
    }
    ++i;
	while (i < 12)
	{
		out[i] = ' ';
		++i;
	}

	out[12] = '\0'; //ensures proper termination regardless of program operation previously
	return;
}

char* Fat32::convertToFATFormat(char *in)
{
    uint16_s count = 0;
    upper_case(in);
    char Name[13] = {'\0'};
    uint16_s dotPos = 0;
    while (count <= 8)
    {
        if (in[count] == '.' || in[count] == '\0')
        {
            dotPos = count;
            count++;
            break;
        }
        else{
            Name[count] = in[count];
            count++;
        }
    }
    if (count > 9)
    {
        count = 8;
        dotPos = 8;
    }
    uint16_s exCount = 8;
    while (exCount < 11)
    {
        if (in[exCount] != '\0')
            Name[exCount] = in[count];
        else
        Name[exCount] = ' ';

        exCount++;
        count++;
    }
    count = dotPos;
    while (count < 8)
    {
        Name[count] = ' ';
        count++;
    }
    strcpy(in,Name);
    return in;
}

short Fat32::checkNameFormat(const char *name)
{
    short isCorrect = 0;
    int i = 0;

    for ( i = 0; i < 11; i++)
    {
        if (name[i] < 0x20 && name[i]!= 0x05)
            isCorrect|=BAD_CHARACTER;
        switch (name[i])
        {
        case 0x2E:
            if ((isCorrect & NOT_CONVER_YET)==NOT_CONVER_YET)
                isCorrect|=MANY_DOTS;
            isCorrect^=NOT_CONVER_YET;
            break;
        case 0x22:
		case 0x2A:
		case 0x2B:
		case 0x2C:
		case 0x2F:
		case 0x3A:
		case 0x3B:
		case 0x3C:
		case 0x3D:
		case 0x3E:
		case 0x3F:
		case 0x5B:
		case 0x5C:
		case 0x5D:
		case 0x7C:
			isCorrect = isCorrect | BAD_CHARACTER;
		}

		if (name[i] >= 'a' && name[i] <= 'z')
			isCorrect = isCorrect | LOWERCASE_ISSUE;

    }
    return isCorrect;
}

void Fat32::addclustertocontent(Content *content)
{
    dir_entry_t content_meta;
	if (content->directory != NULL) content_meta = content->directory->file_meta;
	else if (content->file != NULL) content_meta = content->file->file_meta;
	uint16_s cluster = GET_CLUSTER_FROM_ENTRY(content_meta);
	while (fat_cluster_end(cluster) == false) {
		if (fat_cluster_bad(cluster)==true)
            break;
		if (cluster == -1 )        
            break;
		cluster = fat_read(cluster);
	}

	if (fat_cluster_end(cluster) == true) {
		uint16_s newCluster = allocateFreeFat();

			if ((newCluster == BAD_CLUSTER_32 && fat_type == 32)) //allocation error
			{
				display << "Function putFile: allocateFreeFAT encountered an error. Aborting...\n";
			}
			if (fat_write(cluster, newCluster) != 0)
			{
				display << "Function putFile: FATWrite encountered an error. Aborting...\n";
			}
		}

}

Content *Fat32::FAT_create_content(char *name, bool directory, char *extension)
{
    	Content* content = (Content*)kmalloc(sizeof(Content));
		
		content->directory = NULL;
		content->file = NULL;

		if (strlen(name) > 11 || strlen(extension) > 4) {
			display << "Uncorrect name or ext lenght.\n";
			return NULL;
		}
		
		if (directory == true) {
			content->directory = (Directory*)kmalloc(sizeof(Directory));
			content->directory->files        = NULL;
			content->directory->subDirectory = NULL;
			content->directory->next         = NULL;
			content->directory->data_pointer = NULL;

			content->directory->name = (char*)kmalloc(12);
			strncpy(content->directory->name, name, 11);
			
			content->directory->file_meta = *FAT_create_entry(name, NULL, true, allocateFreeFat(), 0);
		}
		else {
			content->file = (File*)kmalloc(sizeof(File));
			content->file->data_pointer = NULL;
			content->file->data         = NULL;
			content->file->next         = NULL;

			content->file->name = (char*)kmalloc(12);
			content->file->extesion = (char*)kmalloc(5);
			strncpy(content->file->name, name, 11);
			strncpy(content->file->extesion, extension, 4);
			
			content->file->file_meta = *FAT_create_entry(name, extension, false, allocateFreeFat(), 1);
		}

		return content;

    return nullptr;
}

dir_entry_t *Fat32::FAT_create_entry(const char *name, const char *ext, bool isDir, uint32 firstCluster, uint32 filesize)
{
    	dir_entry_t* data =(dir_entry_t*) kmalloc(sizeof(dir_entry_t));

		data->reserved 			= 0; 
		data->times_tenths 	    = 0;
		data->creation_time 	= 0;
		data->creation_date 	= 0;
		data->date_last_operation = 0;

		char* file_name = (char*)kmalloc(25);
		strcpy(file_name, name);
		if (ext) {
			strcat(file_name, ".");
			strcat(file_name, ext);
		}
		
		data->low_word 	= firstCluster;
		data->height_word = firstCluster >> 16;  

		if(isDir == true) {
			data->byte_size_file  = 0;
			data->dir_attrs = 0x10; //FILE_DIRECTORY
		} else {
			data->byte_size_file  = filesize;
			data->dir_attrs = 0x20;//FILE_ARCHIVE;
		}

		data->creation_date = 0x00;
		data->creation_time = 0x00;
		data->times_tenths = 0x00;

		if (checkNameFormat(file_name) != 0)
			convertToFATFormat(file_name);

		strncpy(data->name, file_name, min(11, strlen(file_name)));
		kfree(file_name);

		return data; 
}
