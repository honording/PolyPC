#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include "libelf.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


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
    stream = fopen("/home/hding/mb.elf", "r");
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


    return sym->st_value;
}
