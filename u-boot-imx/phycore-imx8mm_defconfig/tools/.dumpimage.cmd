cmd_tools/dumpimage := gcc  -isystem/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/include -O2 -pipe -L/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib                         -L/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/lib                         -Wl,--enable-new-dtags                         -Wl,-rpath-link,/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib                         -Wl,-rpath-link,/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/lib                         -Wl,-rpath,/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib                         -Wl,-rpath,/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/lib                         -Wl,-O1 -Wl,--allow-shlib-undefined -Wl,--dynamic-linker=/home/jma/phytec_works/iMX8MM.test_image/build/tmp/sysroots-uninative/x86_64-linux/lib/ld-linux-x86-64.so.2   -o tools/dumpimage tools/aisimage.o tools/atmelimage.o tools/fit_common.o tools/fit_image.o tools/image-host.o tools/common/image-fit.o tools/common/fdt_region.o tools/common/bootm.o tools/lib/crc32.o tools/default_image.o tools/lib/fdtdec_common.o tools/lib/fdtdec.o tools/common/image.o tools/imagetool.o tools/imximage.o tools/imx8image.o tools/imx8mimage.o tools/kwbimage.o tools/lib/md5.o tools/lpc32xximage.o tools/mxsimage.o tools/omapimage.o tools/os_support.o tools/pblimage.o tools/pbl_crc32.o tools/vybridimage.o tools/stm32image.o tools/lib/rc4.o tools/rkcommon.o tools/rkimage.o tools/rksd.o tools/rkspi.o tools/socfpgaimage.o tools/sunxi_egon.o tools/lib/crc16.o tools/lib/sha1.o tools/lib/sha256.o tools/lib/sha512.o tools/common/hash.o tools/ublimage.o tools/zynqimage.o tools/zynqmpimage.o tools/zynqmpbif.o tools/libfdt/fdt.o tools/libfdt/fdt_ro.o tools/libfdt/fdt_wip.o tools/libfdt/fdt_sw.o tools/libfdt/fdt_rw.o tools/libfdt/fdt_strerror.o tools/libfdt/fdt_empty_tree.o tools/libfdt/fdt_addresses.o tools/libfdt/fdt_overlay.o tools/gpimage.o tools/gpimage-common.o tools/mtk_image.o tools/dumpimage.o   
