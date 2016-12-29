Quick and ugly hack to expose a directory as a block device with a vfat file
system, based on BUSE and the vvfat code from bochs née qemu.

This should make it possible to e. g. expose a directory as a USB mass storage
device.

An alternative is tojblockd, which seems to be experimental and without
finished write support.
