#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <string.h>
// #include <libelf.h>

#include "libelfmb.h"

#define ELFMB_DEBUG

/*
int read_elf_header(FILE *file,
					 Elf32_Ehdr *ehdr)
{
	int read_num;
	read_num = fread(ehdr, sizeof(Elf32_Ehdr), 1, file);
	if (read_num != 1) {
		return 3;
	}
	return 0;
}

int read_section_header(FILE *file,
						Elf32_Ehdr *ehdr,
						int idx,
						Elf32_Shdr *shdr)
{
	if (ehdr->e_shnum <= idx) {
		return 1;
	}
	int ret;
	ret = fseek(file,
				ehdr->e_shoff + ehdr->e_shentsize * idx,
				SEEK_SET);
	if (ret != 0) {
		return 2;
	}
	int read_num;
	read_num = fread(shdr, sizeof(Elf32_Shdr), 1, file);
	if (read_num != 1) {
		return 3;
	}
	return 0;
}

int read_sym_entry(FILE *file, Elf32_Shdr *shdr, int idx, Elf32_sym *sym)
{
	if (shdr->sh_size / sizeof(Elf32_sym) <= idx) {
		return 1;
	}
	int ret;
	ret = fseek(file, shdr->sh_offset + idx * sizeof(Elf32_sym), SEEK_SET);
	if (ret != 0) {
		return 2;
	}
	int read_num;
	read_num = fread(sym, sizeof(Elf32_sym), 1, file);
	if (read_num != 1) {
		return 3;
	}
	return 0;
}

int match_name(char *name_table, int idx, char *name)
{ //1: true, 0: false;
	int i = 0;
	while (name_table[idx] != '\0' &&
		   name[i] != '\0') {
		if (name_table[idx] != name[i]) {
			return 0;
		}
		idx++;
		i++;
	}
	if (name_table[idx] != '\0' || name[i] != '\0') {
		return 0;
	}
	return 1;
}

void print_name(char *name_table, int idx)
{
	while (name_table[idx] != '\0') {
		printf("%c", name_table[idx]);
		idx++;
	}
	printf("\n");
}

unsigned int get_main_entry(char *file_name)
{
    Elf32_Ehdr *hdr;
    hdr = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    if (hdr == NULL) {
        printf("Malloc errors.\n");
        return 0;
    }
    FILE *stream;
    stream = fopen(file_name, "r");
    int ret;
    int read_num;

    read_elf_header(stream, hdr);

    Elf32_Shdr *shdr;
    shdr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr));
    read_section_header(stream, hdr, hdr->e_shstrndx, shdr);

    char *section_name;
    section_name = (char *)malloc(shdr->sh_size * sizeof(char));
    ret = fseek(stream, shdr->sh_offset, SEEK_SET);
    read_num = fread(section_name, sizeof(char), shdr->sh_size, stream);

    char *sym_name;
    Elf32_Shdr *sym_header;
    Elf32_Shdr *text_header;
    int strtab_size;
    int i = 0;
    for (i = 0; i < hdr->e_shnum; i++) {
        read_section_header(stream, hdr, i, shdr);
        if (match_name(section_name, shdr->sh_name, ".strtab") == 1) {
            strtab_size = shdr->sh_size;
            sym_name = (char *)malloc(shdr->sh_size * sizeof(char));
            ret = fseek(stream, shdr->sh_offset, SEEK_SET);
            read_num = fread(sym_name, sizeof(char), shdr->sh_size, stream);
        }
        if (match_name(section_name, shdr->sh_name, ".symtab") == 1) {
            sym_header = shdr;
            shdr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr));
        }
        if (match_name(section_name, shdr->sh_name, ".text") == 1) {
            text_header = shdr;
            shdr = (Elf32_Shdr *)malloc(sizeof(Elf32_Shdr));
        }
    }

    Elf32_sym *sym;
    sym = (Elf32_sym *)malloc(sizeof(Elf32_sym));
    for (i = 0; i < sym_header->sh_size / sizeof(Elf32_sym); i++) {
        read_sym_entry(stream, sym_header, i, sym);
        if (match_name(sym_name, sym->st_name, "main") == 1) {
            break;
        }
    }
    //print_name(sym_name, sym->st_name);
    //printf("value: %x\n", sym->st_value);
    //printf("size: %x\n", sym->st_size);
    fclose(stream);


    return sym->st_value - text_header->sh_addr + text_header->sh_offset;
}
//Compiler options for off_t over 2G
//-D_FILE_OFFSET_BITS=64
int load_elf(char *file_name, unsigned int addr)
{
    FILE *elf_stream;
    int outfd;
    void *ptr; 
    int file_size = -1;
    struct stat statbuff;
    if (stat(file_name, &statbuff) < 0) {
        perror("Got file size error.");
        return -1;
    }
    file_size = statbuff.st_size;
    elf_stream = fopen(file_name, "r");
    outfd = open(DEVMEM, O_RDWR);
    if (outfd < 1) {
        fclose(elf_stream);
        perror("devmem open failed.");
        return -1;
    }
    ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, outfd, addr);
    //ptr = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, outfd, 0);
    //addr is off_t and has to be aligned with page size;
    //printf("ptr addr: %x\n", ptr);
    if (ptr == (void *)-1) {
        fclose(elf_stream);
        perror("mmap failed.");
        return -1;
    }
    int read_num = -1;
    // char tempchar[4];
    // read_num = fread(tempchar, sizeof(char), 4, elf_stream);
    // printf("%X\n", tempchar[0]);
    // printf("%X\n", tempchar[1]);
    // printf("%X\n", tempchar[2]);
    // printf("%X\n", tempchar[3]);
    
    read_num = fread(ptr, sizeof(char), file_size, elf_stream);
    //read_num = fread(ptr + addr, sizeof(char), file_size, elf_stream);
    if (read_num != file_size) {
        munmap(ptr, file_size);
        fclose(elf_stream);
        perror("read elf failed.");
        return -1;
    }
    munmap(ptr, file_size);
    fclose(elf_stream);
    return file_size;
}
*/

int elf_valid(Elf32_Ehdr *ehdr) 
{
    if (ehdr->e_ident[EI_MAG0] == 0x7F &&
        ehdr->e_ident[EI_MAG1] == 'E' &&
        ehdr->e_ident[EI_MAG2] == 'L' &&
        ehdr->e_ident[EI_MAG3] == 'F') {
        return 1;
    }
#ifndef ELFMB_DEBUG
    printf("0x%2X, %c, %c, %c.\n", ehdr->e_ident[EI_MAG0],
                                   ehdr->e_ident[EI_MAG1],
                                   ehdr->e_ident[EI_MAG2],
                                   ehdr->e_ident[EI_MAG3]);
#endif
    return 0;
}

int match_name(char *name, Elf32_Sym *syms, char *strings, int num) 
{
    int i;
    for (i = 0; i < num; i++) {
        if (strcmp(name, strings + syms[i].st_name) == 0) {
            return syms[i].st_value;
        }
    }
    return 0;
}

int elf_loader(char *file_name, 
               unsigned int addr, 
               unsigned int elf_start_addr, 
               elf_info_t *elf_info) 
{
    Elf32_Ehdr *ehdr    =   NULL;
    // printf("Begin to read elf size\n");
    /*
    struct stat statbuff;
    if (stat(file_name, &statbuff) < 0) {
        perror("elf_loader: elf file size got error.");
        return -1;
    }
    // printf("Begin to open elf file.\n");
    int file_size = statbuff.st_size;
    if (file_size <= 0) {
        perror("elf_loader: file_size is wrong.");
        return -1;
    }
    */
    // printf("elf_loader: file path: %s\n", file_name);
    int file_size = 0;
    FILE *elf_stream    =   NULL;
    elf_stream = fopen(file_name, "r");
    if (elf_stream == NULL) {
        perror("elf_loader: elf file open error.");
        return -1;
    }
    fseek(elf_stream, 0L, SEEK_END);
    file_size = ftell(elf_stream);
    fseek(elf_stream, 0L, SEEK_SET);
    if (file_size <= 0) {
        perror("elf_loader: file_size is wrong.");
        return -1;
    }    
    // printf("elf_stream %x\n", elf_stream);
    printf("Begin to allocate buf\n");
    char *buf = (char *)malloc(sizeof(char) * file_size);
    if (buf == NULL) {
        perror("elf_loader: buf memory allocate error.");
        fclose(elf_stream);
        return -1;
    }
    printf("Begin to read elf to buf:%d\n", file_size);
    int read_num = fread(buf, sizeof(char), file_size, elf_stream);
    printf("After read elf to buf:%d\n", read_num);
    if (read_num != file_size) {
        printf("read error.\n");
        perror("elf_loader: elf file read error.");
        fclose(elf_stream);
        free(buf);
        return -1;
    }
    // printf("elf_stream %x\n", elf_stream);
    fclose(elf_stream); //right place
    printf("Begin to check elf\n");
    ehdr = (Elf32_Ehdr *)buf;
    if (!elf_valid(ehdr)) {
        perror("elf_loader: invalid elf format.");
        free(buf);
        return -1;
    }
    printf("Begin to open devmem\n");
    int devmemfd = open(DEVMEM, O_RDWR);
    if (devmemfd < 1) {
        perror("elf_loader: devmem open failed.");
        free(buf);
        return -1;
    }
    printf("Begin to mmap\n");
    char *exec = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, devmemfd, addr);
    if (!exec) {
        perror("elf_loader: mmap devmem failed.");
        close(devmemfd);
        free(buf);
        return -1;        
    }
    memset(exec, 0x0, file_size);

    Elf32_Phdr *phdr    =   NULL;
    char *elf_offset    =   NULL;
    char *ram_offset    =   NULL;
    int i;
    phdr = (Elf32_Phdr *)(buf + ehdr->e_phoff);
    printf("Begin to load elf\n");
    unsigned int max_ram_range = 0;
    for(i = 0; i < ehdr->e_phnum; i++) {
            if(phdr[i].p_type != PT_LOAD) {
                    continue;
            }
            if(phdr[i].p_filesz > phdr[i].p_memsz) {
                    perror("elf_loader: p_filesz > p_memsz.");
                    munmap(exec, file_size);
                    close(devmemfd);
                    free(buf);
                    return -1;
            }
            if(!phdr[i].p_filesz) {
                    continue;
            }
            if(phdr[i].p_vaddr != elf_start_addr) {
                continue;
            }

            // p_filesz can be smaller than p_memsz,
            // the difference is zeroe'd out.
            // each program segment is consecutieve.

            // the following is for the elf virtual address begins from
            // 0x00000000
            elf_offset = buf + phdr[i].p_offset;
            // ram_offset = exec + phdr[i].p_vaddr;
            // if (phdr[i].p_vaddr + phdr[i].p_memsz > max_ram_range) {
            //     max_ram_range = phdr[i].p_vaddr + phdr[i].p_memsz;
            // }
            ram_offset = exec;
            if (phdr[i].p_filesz > max_ram_range) {
                max_ram_range = phdr[i].p_filesz;
            }
            printf("file offset:%x, file size:%x.\n", phdr[i].p_offset,
                                                      phdr[i].p_filesz);
            memmove(ram_offset, elf_offset, phdr[i].p_filesz);

            // if(!(phdr[i].p_flags & PF_W)) {
            //         // Read-only.
            //         mprotect((unsigned char *) taddr,
            //                   phdr[i].p_memsz,
            //                   PROT_READ);
            // }

            // if(phdr[i].p_flags & PF_X) {
            //         // Executable.
            //         mprotect((unsigned char *) taddr,
            //                 phdr[i].p_memsz,
            //                 PROT_EXEC);
            // }
    }
    printf("Begin to munmap\n");
    munmap(exec, file_size);
    close(devmemfd);
    printf("Begin to locate main\n");
    // find "main" entry address;
    Elf32_Shdr *shdr            =   NULL;
    Elf32_Sym  *syms            =   NULL;
    char *strings               =   NULL;
    unsigned int main_entry     =   0;
    unsigned int stack_addr     =   0;
    shdr = (Elf32_Shdr *)(buf + ehdr->e_shoff);
    for (i = 0; i < ehdr->e_shnum; i++) {
        // printf("i:%d\n", i);
        if (shdr[i].sh_type == SHT_SYMTAB) {
            syms = (Elf32_Sym *)(buf + shdr[i].sh_offset);
            strings = (char *)(buf + shdr[shdr[i].sh_link].sh_offset);
            main_entry = match_name("main", syms, strings, shdr[i].sh_size / sizeof(Elf32_Sym));
            stack_addr = match_name("_stack", syms, strings, shdr[i].sh_size / sizeof(Elf32_Sym));
            if (stack_addr == 0) {
                stack_addr = match_name("__stack", syms, strings, shdr[i].sh_size / sizeof(Elf32_Sym));
            }
            break;
        }
    }
    free(buf);
    if (main_entry == 0) {
        perror("elf_loader: main entry get error.");
        return -1;
    }
    printf("before return from elf_loader:%X\n", main_entry);
    elf_info->main_addr     = main_entry;
    elf_info->stack_addr    = stack_addr;
    elf_info->thread_size   = max_ram_range;
    return 1;
}
