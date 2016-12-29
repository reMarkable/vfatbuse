/////////////////////////////////////////////////////////////////////////
// $Id: vvfat.h 11879 2013-10-13 14:33:55Z vruppert $
/////////////////////////////////////////////////////////////////////////
//
// Virtual VFAT image support (shadows a local directory)
// ported from QEMU block driver with some additions (see vvfat.cc)
//
// Copyright (c) 2004,2005  Johannes E. Schindelin
// Copyright (C) 2010-2012  The Bochs Project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
/////////////////////////////////////////////////////////////////////////

#ifndef BX_VVFAT_H
#define BX_VVFAT_H
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#define FMT_LL "%ll"

#define REDOLOG_TYPE "Redolog"
#define REDOLOG_SUBTYPE_UNDOABLE "Undoable"
#define REDOLOG_SUBTYPE_VOLATILE "Volatile"
#define REDOLOG_SUBTYPE_GROWING  "Growing"
#define REDOLOG_PAGE_NOT_ALLOCATED (0xffffffff)

// hdimage format check return values
#define HDIMAGE_FORMAT_OK      0
#define HDIMAGE_SIZE_ERROR    -1
#define HDIMAGE_READ_ERROR    -2
#define HDIMAGE_NO_SIGNATURE  -3
#define HDIMAGE_TYPE_ERROR    -4
#define HDIMAGE_VERSION_ERROR -5

#define VOLATILE_REDOLOG_EXTENSION ".XXXXXX"
#define VOLATILE_REDOLOG_EXTENSION_LENGTH (strlen(VOLATILE_REDOLOG_EXTENSION))
#define HDIMAGE_HAS_GEOMETRY  2

// SPARSE IMAGES HEADER
#define SPARSE_HEADER_MAGIC  (0x02468ace)
#define SPARSE_HEADER_VERSION  2
#define SPARSE_HEADER_V1       1
#define SPARSE_HEADER_SIZE        (256) // Plenty of room for later
#define SPARSE_PAGE_NOT_ALLOCATED (0xffffffff)

#define BX_PATHNAME_LEN 512
#define htod16(val) (val)
#define dtoh16(val) (val)
#define htod32(val) (val)
#define dtoh32(val) (val)
#define htod64(val) (val)
#define dtoh64(val) (val)

typedef uint8_t Bit8u;
typedef int8_t Bit8s;
typedef uint16_t Bit16u;
typedef int16_t Bit16s;
typedef uint32_t Bit32u;
typedef int32_t Bit32s;
typedef uint64_t Bit64u;
typedef int64_t Bit64s;
typedef bool bx_bool;

Bit16u fat_datetime(time_t time, int return_time);

typedef struct array_t {
  char *pointer;
  unsigned int size, next, item_size;
} array_t;

typedef struct {
  Bit8u head;
  Bit8u sector;
  Bit8u cylinder;
} mbr_chs_t;

#if defined(_MSC_VER) && (_MSC_VER<1300)
#pragma pack(push, 1)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=packed
#endif

typedef
#if defined(_MSC_VER) && (_MSC_VER>=1300)
  __declspec(align(1))
#endif
  struct direntry_t {
    Bit8u name[8];
    Bit8u extension[3];
    Bit8u attributes;
    Bit8u reserved[2];
    Bit16u ctime;
    Bit16u cdate;
    Bit16u adate;
    Bit16u begin_hi;
    Bit16u mtime;
    Bit16u mdate;
    Bit16u begin;
    Bit32u size;
}
#if !defined(_MSC_VER)
__attribute__ ((packed))
#endif
direntry_t;

#if defined(_MSC_VER) && (_MSC_VER<1300)
#pragma pack(pop)
#elif defined(__MWERKS__) && defined(macintosh)
#pragma options align=reset
#endif

// this structure are used to transparently access the files

enum {
  MODE_UNDEFINED = 0, MODE_NORMAL = 1, MODE_MODIFIED = 2,
  MODE_DIRECTORY = 4, MODE_FAKED = 8,
  MODE_DELETED = 16, MODE_RENAMED = 32
};

typedef struct mapping_t {
  // begin is the first cluster, end is the last+1
  Bit32u begin, end;
  // as this->directory is growable, no pointer may be used here
  unsigned int dir_index;
  // the clusters of a file may be in any order; this points to the first
  int first_mapping_index;
  union {
    /* offset is
     * - the offset in the file (in clusters) for a file, or
     * - the next cluster of the directory for a directory, and
     * - the address of the buffer for a faked entry
     */
    struct {
      Bit32u offset;
    } file;
    struct {
      int parent_mapping_index;
      int first_dir_index;
    } dir;
  } info;
  // path contains the full path, i.e. it always starts with vvfat_path
  char *path;

  Bit8u mode;

  int read_only;
} mapping_t;

#define STANDARD_HEADER_MAGIC     "Bochs Virtual HD Image"
#define STANDARD_HEADER_V1        (0x00010000)
#define STANDARD_HEADER_VERSION   (0x00020000)
#define STANDARD_HEADER_SIZE      (512)

 // WARNING : headers are kept in x86 (little) endianness
 typedef struct
 {
   Bit8u   magic[32];
   Bit8u   type[16];
   Bit8u   subtype[16];
   Bit32u  version;
   Bit32u  header;
 } standard_header_t;


 typedef struct
 {
   // the fields in the header are kept in little endian
   Bit32u  catalog;    // #entries in the catalog
   Bit32u  bitmap;     // bitmap size in bytes
   Bit32u  extent;     // extent size in bytes
   Bit32u  timestamp;  // modification time in FAT format (subtype 'undoable' only)
   Bit64u  disk;       // disk size in bytes
 } redolog_specific_header_t;

 typedef struct
 {
   // the fields in the header are kept in little endian
   Bit32u  catalog;    // #entries in the catalog
   Bit32u  bitmap;     // bitmap size in bytes
   Bit32u  extent;     // extent size in bytes
   Bit64u  disk;       // disk size in bytes
 } redolog_specific_header_v1_t;

 typedef struct
 {
   standard_header_t standard;
   redolog_specific_header_t specific;

   Bit8u padding[STANDARD_HEADER_SIZE - (sizeof (standard_header_t) + sizeof (redolog_specific_header_t))];
 } redolog_header_t;

 typedef struct
 {
   standard_header_t standard;
   redolog_specific_header_v1_t specific;

   Bit8u padding[STANDARD_HEADER_SIZE - (sizeof (standard_header_t) + sizeof (redolog_specific_header_v1_t))];
 } redolog_header_v1_t;

// REDOLOG class
class redolog_t
{
  public:
      redolog_t();
      int make_header(const char* type, Bit64u size);
      int create(const char* filename, const char* type, Bit64u size);
      int create(int filedes, const char* type, Bit64u size);
      int open(const char* filename, const char* type);
      int open(const char* filename, const char* type, int flags);
      void close();
      Bit64u get_size();
      Bit32u get_timestamp();
      bx_bool set_timestamp(Bit32u timestamp);

      Bit64s lseek(Bit64s offset, int whence);
      ssize_t read(void* buf, size_t count);
      ssize_t write(const void* buf, size_t count);

      static int check_format(int fd, const char *subtype);

#ifdef BXIMAGE
      int commit(device_image_t *base_image);
#else
      bx_bool save_state(const char *backup_fname);
#endif

  private:
      void             print_header();
      int              fd;
      redolog_header_t header;     // Header is kept in x86 (little) endianness
      Bit32u          *catalog;
      Bit8u           *bitmap;
      bx_bool          bitmap_update;
      Bit32u           extent_index;
      Bit32u           extent_offset;
      Bit32u           extent_next;

      Bit32u           bitmap_blocks;
      Bit32u           extent_blocks;

      Bit64s           imagepos;
};


class vvfat_image_t// : public device_image_t
{
  public:
    vvfat_image_t(Bit64u size, const char* redolog_name);
    virtual ~vvfat_image_t();

    int open(const char* pathname);
    void close();
    Bit64s lseek(Bit64s offset, int whence);
    ssize_t read(void* buf, size_t count);
    ssize_t write(const void* buf, size_t count);
    Bit32u get_capabilities();
    void commit_changes(void);

  private:
    bx_bool sector2CHS(Bit32u spos, mbr_chs_t *chs);
    void init_mbr();
    direntry_t* create_long_filename(const char* filename);
    void fat_set(unsigned int cluster, Bit32u value);
    void init_fat();
    direntry_t* create_short_and_long_name(unsigned int directory_start,
      const char* filename, int is_dot);
    int read_directory(int mapping_index);
    Bit32u sector2cluster(off_t sector_num);
    off_t cluster2sector(Bit32u cluster_num);
    int init_directories(const char* dirname);
    bx_bool read_sector_from_file(const char *path, Bit8u *buffer, Bit32u sector);
    void set_file_attributes(void);
    Bit32u fat_get_next(Bit32u current);
    bx_bool write_file(const char *path, direntry_t *entry, bx_bool create);
    direntry_t* read_direntry(Bit8u *buffer, char *filename);
    void parse_directory(const char *path, Bit32u start_cluster);
    void close_current_file(void);
    int open_file(mapping_t* mapping);
    int find_mapping_for_cluster_aux(int cluster_num, int index1, int index2);
    mapping_t* find_mapping_for_cluster(int cluster_num);
    mapping_t* find_mapping_for_path(const char* path);
    int read_cluster(int cluster_num);

    Bit8u  *first_sectors;
    Bit32u offset_to_bootsector;
    Bit32u offset_to_fat;
    Bit32u offset_to_root_dir;
    Bit32u offset_to_data;

    Bit16u cluster_size;
    Bit8u  sectors_per_cluster;
    Bit32u sectors_per_fat;
    Bit32u sector_count;
    Bit32u cluster_count; // total number of clusters of this partition
    Bit32u max_fat_value;
    Bit32u first_cluster_of_root_dir;
    Bit16u root_entries;
    Bit16u reserved_sectors;

    Bit8u  fat_type;
    array_t fat, directory, mapping;

    int current_fd;
    mapping_t* current_mapping;
    Bit8u  *cluster; // points to current cluster
    Bit8u  *cluster_buffer; // points to a buffer to hold temp data
    Bit16u current_cluster;

    const char *vvfat_path;
    Bit32u sector_num;

    bx_bool use_mbr_file;
    bx_bool use_boot_file;
    FILE    *vvfat_attr_fd;

    bx_bool   vvfat_modified;
    void      *fat2;
    redolog_t *redolog;       // Redolog instance
    char      *redolog_name;  // Redolog name
    char      *redolog_temp;  // Redolog temporary file name
    unsigned heads;
    unsigned cylinders;
    unsigned spt;
    Bit64u   hd_size;
};

#endif
