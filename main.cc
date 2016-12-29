/*
 * busexmp - example memory-based block device using BUSE
 * Copyright (C) 2013 Adam Cozzette
 *
 * This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "buse.h"
#include "vvfat.h"

static void *data;
static int xmpl_debug = 1;

static int xmp_read(void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
    fprintf(stderr, "R - %lu, %u\n", offset, len);

    vvfat_image_t *image = (vvfat_image_t*)userdata;
    image->lseek(offset, SEEK_SET);
    int ret = image->read(buf, len);

    if (ret < 0) {
        return ret;
    }

    return 0;
}

static int xmp_write(const void *buf, u_int32_t len, u_int64_t offset, void *userdata)
{
    vvfat_image_t *image = (vvfat_image_t*)userdata;
    image->lseek(offset, SEEK_SET);
    int ret = image->write(buf, len);

    if (ret < 0) {
        return ret;
    }

    return 0;
}

static void xmp_disc(void *userdata)
{
  (void)(userdata);
  fprintf(stderr, "Received a disconnect request.\n");
}

static int xmp_flush(void *userdata)
{
    fprintf(stderr, "Received a flush request.\n");

    vvfat_image_t *image = (vvfat_image_t*)userdata;
    image->commit_changes();

    return 0;
}

static int xmp_trim(u_int64_t from, u_int32_t len, void *userdata)
{
  (void)(userdata);
  fprintf(stderr, "T - %lu, %u\n", from, len);
  return 0;
}


static struct buse_operations aop = {
  .read = xmp_read,
  .write = xmp_write,
  .disc = xmp_disc,
  .flush = xmp_flush,
  .trim = xmp_trim,
  .size = 128 * 1024 * 1024,
};

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    fprintf(stderr, 
        "Usage:\n"
        "  %s /dev/nbd0 /export/ums\n"
        "Don't forget to load nbd kernel module (`modprobe nbd`) and\n"
        "run example from root.\n", argv[0]);
    return 1;
  }
  vvfat_image_t image(aop.size, NULL);
  if (image.open(argv[2]) != 0) {
      fprintf(stderr, "Failed to open directory %s\n", argv[2]);
      return 1;
  }

  int ret = buse_main(argv[1], &aop, (void *)&image);
  image.close();
  return ret;
}
