hfs2tar
========

*Convert a HFS-plus volume image to a tar archive in user space*

*hfs2tar* is a small little tool that will convert a raw image of a HFS-plus volume (including the Apple Partition Map) and convert it to a tar archive. *WARNING*: this is work in progress - there are still many limitiations. See Issues.

Build Instructions
------------------
hfs2tar requires a modern compiler supporting C++0x/C++11 and requires the boost libraries. You can compile the code by executing: 'make'

Usage
-----
hfs2tar raw-image-filename.img > volume.tar

The input image must be in raw format, i.e. *DMG images are not supported* and must be converted with dmg2img (http://vu1tur.eu.org/tools/). Furthermore, the image *must* contain an Apple Partition Map. See issues.

Issues
------
This software is *experimental* and work in progress. The following limitations are known:
1. hfs2tar only understands raw image files. DMG files must be converted with dmg2img (http://vu1tur.eu.org/tools/) before using them with hfs2tar.
2. The images must also contain an apple partition map - this is the default with most Mac OS X disk images.
3. The first HFS-plus partition in the image is converted to a tar archive. All other partitions are ignored.
4. Currently, hfs2tar does not support HFS files with require the extents overflow feature. This feature is typically used only on highly fragmented disk images.
5. Modification times of the tar archive will be incorrect.

Contact
-------
Fabian Renn, fabian.renn@gmail.com
http://github.com/hogliux/hfs2tar
