/*
 *  Copyright (C) 2011 Ji YongGang <jungleji@gmail.com>
 *
 *  ChmSee is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.

 *  ChmSee is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.

 *  You should have received a copy of the GNU General Public License
 *  along with ChmSee; see the file COPYING.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <chm_lib.h>

#include "csChmfile.h"

struct extract_context
{
        const char *base_path;
};

#define UINT16ARRAY(x) ((unsigned char)(x)[0] | ((u_int16_t)(x)[1] << 8))
#define UINT32ARRAY(x) (UINT16ARRAY(x) | ((u_int32_t)(x)[2] << 16)      \
                        | ((u_int32_t)(x)[3] << 24))

static int file_exists(const char *path)
{
        struct stat statbuf;
        if (stat(path, &statbuf) != -1)
                return 1;
        else
                return 0;
}

static int rmkdir(char *path)
{
        char *i = strrchr(path, '/');

        if(path[0] == '\0'  ||  file_exists(path))
                return 0;

        if (i != NULL) {
                *i = '\0';
                rmkdir(path);
                *i = '/';
                mkdir(path, 0777);
        }

        if (file_exists(path))
                return 0;
        else
                return -1;
}

/*
 * Callback function for enumerate API
 */
static int _extract_callback(struct chmFile *h,
                             struct chmUnitInfo *ui,
                             void *context)
{
        LONGUINT64 ui_path_len;
        char buffer[32768];
        struct extract_context *ctx = (struct extract_context *)context;
        char *i;

        if (ui->path[0] != '/')
                return CHM_ENUMERATOR_CONTINUE;

        /* quick hack for security hole mentioned by Sven Tantau */
        if (strstr(ui->path, "/../") != NULL) {
                /* fprintf(stderr, "Not extracting %s (dangerous path)\n", ui->path); */
                return CHM_ENUMERATOR_CONTINUE;
        }

        if (snprintf(buffer, sizeof(buffer), "%s/%s", ctx->base_path, ui->path) > 1024)
                return CHM_ENUMERATOR_FAILURE;

        /* Get the length of the path */
        ui_path_len = strlen(ui->path)-1;

        /* Distinguish between files and dirs */
        if (ui->path[ui_path_len] != '/' ) {
                FILE *fout;
                LONGINT64 len, remain=ui->length;
                LONGUINT64 offset = 0;

                d(printf("_extract_callback >>> ui->path = %s\n", ui->path));
                if ((fout = fopen(buffer, "wb")) == NULL) {
                        /* make sure that it isn't just a missing directory before we abort */
                        char newbuf[32768];
                        strcpy(newbuf, buffer);
                        i = strrchr(newbuf, '/');
                        *i = '\0';
                        rmkdir(newbuf);
                        if ((fout = fopen(buffer, "wb")) == NULL)
                                return CHM_ENUMERATOR_FAILURE;
                }

                while (remain != 0) {
                        len = chm_retrieve_object(h, ui, (unsigned char *)buffer, offset, 32768);
                        if (len > 0) {
                                fwrite(buffer, 1, (size_t)len, fout);
                                offset += len;
                                remain -= len;
                        } else {
                                fprintf(stderr, "incomplete file: %s\n", ui->path);
                                break;
                        }
                }

                fclose(fout);
        } else {
                if (rmkdir(buffer) == -1)
                        return CHM_ENUMERATOR_FAILURE;
        }

        return CHM_ENUMERATOR_CONTINUE;
}

static u_int32_t
get_dword(const unsigned char *buf)
{
        u_int32_t result;

        result = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

        if (result == 0xFFFFFFFF)
                result = 0;

        return result;
}

static void
chm_system_info(struct fileinfo *info)
{
        FILE *fp;
        unsigned char buffer[4096];
        char system_file[1024];
        size_t size = 0, len = 0;
        int index = 0;
        unsigned char* cursor = NULL;
        u_int16_t value = 0;

        sprintf(system_file, "%s/#SYSTEM", info->bookfolder);
        d(printf("chm_system_info >>> system info file = %s\n", system_file));

        fp = fopen(system_file, "r");
        if (fp == NULL) {
                fprintf(stderr, "#SYSTEM file open failed.\n");
                return;
        }

        fread(buffer, sizeof(char), 4, fp); // Skip 4 bytes
        size = fread(buffer, sizeof(char), 4096, fp);

        if (!size)
                return;

        buffer[size - 1] = 0;

        for(;;) {
                if (index > size - 1 - (long)sizeof(u_int16_t))
                        break;

                cursor = buffer + index;
                value = UINT16ARRAY(cursor);
                d(printf("chm_system_info >>> value = %d\n", value));

                switch(value) {
                case 0:
                        index += 2;
                        cursor = buffer + index;

                        len = strlen((const char*)(buffer + index + 2));
                        info->hhc = (char *)malloc(len + 1);
                        strcpy(info->hhc, (const char*)(buffer + index + 2));
                        d(printf("chm_system_info >>> hhc = %s\n", info->hhc));

                        break;
                case 1:
                        index += 2;
                        cursor = buffer + index;

                        len = strlen((const char*)(buffer + index + 2));
                        info->hhk = (char *)malloc(len + 1);
                        strcpy(info->hhk, (const char*)(buffer + index + 2));
                        d(printf("chm_system_info >>> hhk = %s\n", info->hhk));

                        break;
                case 2:
                        index += 2;
                        cursor = buffer + index;

                        len = strlen((const char*)(buffer + index + 2));
                        info->homepage = (char *)malloc(len + 1);
                        strcpy(info->homepage, (const char*)(buffer + index + 2));
                        d(printf("chm_system_info >>> homepage = %s\n", info->homepage));

                        break;
                case 3:
                        index += 2;
                        cursor = buffer + index;

                        len = strlen((const char*)(buffer + index + 2));
                        info->bookname = (char *)malloc(len + 1);
                        strcpy(info->bookname, (const char*)(buffer + index + 2));
                        d(printf("chm_system_info >>> bookname = %s\n", info->bookname));

                        break;
                case 4:
                        index += 2;
                        cursor = buffer + index;

                        info->lcid = UINT32ARRAY(buffer + index + 2);

                        break;
                default:
                        index += 2;
                        cursor = buffer + index;
                }

                value = UINT16ARRAY(cursor);
                index += value + 2;
        }
}

static void
chm_windows_info(struct fileinfo *info)
{
        FILE *fp;
        unsigned char buffer[4096];
        char windows_file[1024], strings_file[1024];
        size_t size = 0, len = 0;
        u_int32_t entries, entry_size;
        u_int32_t hhc, hhk, bookname, homepage;

        sprintf(windows_file, "%s/#WINDOWS", info->bookfolder);
        d(printf("chm_windows_info >>> windows info file = %s\n", windows_file));

        fp = fopen(windows_file, "r");

        if (fp == NULL) {
                fprintf(stderr, "Open windows info file failed.\n");
                return;
        }

        size = fread(buffer, sizeof(char), 8, fp);

        if (size < 8)
                return;

        entries = get_dword(buffer);
        if (entries < 1)
                return;

        entry_size = get_dword(buffer + 4);

        size = fread(buffer, sizeof(char), entry_size, fp);
        if (size < entry_size)
                return;

        hhc = get_dword(buffer + 0x60);
        hhk = get_dword(buffer + 0x64);
        homepage = get_dword(buffer + 0x68);
        bookname = get_dword(buffer + 0x14);

        d(printf("chm_windows_info >>> hhc = %x\n", hhc));
        d(printf("chm_windows_info >>> hhk = %x\n", hhk));
        d(printf("chm_windows_info >>> homepage = %x\n", homepage));
        d(printf("chm_windows_info >>> bookname = %x\n", bookname));

        fclose(fp);

        sprintf(strings_file, "%s/#STRINGS", info->bookfolder);
        d(printf("chm_windows_info >>> strings info file = %s\n", strings_file));

        fp = fopen(strings_file, "r");

        if (fp == NULL) {
                fprintf(stderr, "Open strings info file failed.\n");
                return;
        }

        size = fread(buffer, sizeof(char), 4096, fp);

        if (size == 0)
                return;

        if (!info->hhc && hhc) {
                len = strlen((const char*)(buffer + hhc));
                info->hhc = (char *)malloc(len + 1);
                strcpy(info->hhc, (const char*)(buffer + hhc));
        }
        if (!info->hhk && hhk) {
                len = strlen((const char*)(buffer + hhk));
                info->hhk = (char *)malloc(len + 1);
                strcpy(info->hhk, (const char*)(buffer + hhk));
        }
        if (!info->homepage && homepage) {
                len = strlen((const char*)(buffer + homepage));
                info->homepage = (char *)malloc(len + 1);
                strcpy(info->homepage, (const char*)(buffer + homepage));
        }
        if (!info->bookname && bookname) {
                len = strlen((const char*)(buffer + bookname));
                info->bookname = (char *)malloc(len + 1);
                strcpy(info->bookname, (const char*)(buffer + bookname));
        }

        fclose(fp);
}

long
extract_chm(const char *filename, const char *base_path)
{
        struct chmFile *handle;
        struct extract_context ec;

        handle = chm_open(filename);

        if (handle == NULL) {
                fprintf(stderr, "Cannot open chmfile: %s", filename);
                return -1;
        }

        ec.base_path = base_path;

        if (!chm_enumerate(handle,
                           CHM_ENUMERATE_NORMAL | CHM_ENUMERATE_SPECIAL,
                           _extract_callback,
                           (void *)&ec)) {
                fprintf(stderr, "Extract chmfile failed: %s", filename);
        }

        chm_close(handle);

        return 0;
}

void
chm_fileinfo(struct fileinfo *info)
{
        chm_system_info(info);
        chm_windows_info(info);

        d(printf("chm_fileinfo >>> hhc = %s\n", info->hhc));
        d(printf("chm_fileinfo >>> hhk = %s\n", info->hhk));
        d(printf("chm_fileinfo >>> homepage = %s\n", info->homepage));
        d(printf("chm_fileinfo >>> bookname = %s\n", info->bookname));
        d(printf("chm_fileinfo >>> lcid = %x\n", info->lcid));
}
