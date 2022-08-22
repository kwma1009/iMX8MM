cmd_drivers/usb/gadget/f_fastboot.o := aarch64-phytec-linux-gcc  --sysroot=/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot -Wp,-MD,drivers/usb/gadget/.f_fastboot.o.d  -nostdinc -isystem /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/bin/aarch64-phytec-linux/../../lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include -Iinclude  -I../include  -I../arch/arm/include -include ../include/linux/kconfig.h  -I../drivers/usb/gadget -Idrivers/usb/gadget -D__KERNEL__ -D__UBOOT__ -Wall -Wstrict-prototypes -Wno-format-security -fno-builtin -ffreestanding -std=gnu11 -fshort-wchar -fno-strict-aliasing -fno-PIE -Os -fno-stack-protector -fno-delete-null-pointer-checks -Wno-stringop-truncation -Wno-maybe-uninitialized -fmacro-prefix-map=../= -g -fstack-usage -Wno-format-nonliteral -Wno-address-of-packed-member -Wno-unused-but-set-variable -Werror=date-time -D__ARM__ -fno-pic -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8 -mgeneral-regs-only    -DKBUILD_BASENAME='"f_fastboot"'  -DKBUILD_MODNAME='"f_fastboot"' -c -o drivers/usb/gadget/f_fastboot.o ../drivers/usb/gadget/f_fastboot.c

source_drivers/usb/gadget/f_fastboot.o := ../drivers/usb/gadget/f_fastboot.c

deps_drivers/usb/gadget/f_fastboot.o := \
    $(wildcard include/config/fastboot/uuu/support.h) \
    $(wildcard include/config/sys/stdio/deregister.h) \
    $(wildcard include/config/sys/cacheline/size.h) \
    $(wildcard include/config/psci/board/reboot.h) \
    $(wildcard include/config/android/recovery.h) \
  ../include/linux/kconfig.h \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/spl/.h) \
    $(wildcard include/config/tpl/build.h) \
    $(wildcard include/config/spl/build.h) \
    $(wildcard include/config/spl/foo.h) \
    $(wildcard include/config/tpl/foo.h) \
    $(wildcard include/config/option.h) \
    $(wildcard include/config/acme.h) \
    $(wildcard include/config/spl/acme.h) \
    $(wildcard include/config/tpl/acme.h) \
  ../include/command.h \
    $(wildcard include/config/sys/help/cmd/width.h) \
    $(wildcard include/config/sys/longhelp.h) \
    $(wildcard include/config/auto/complete.h) \
    $(wildcard include/config/cmd/run.h) \
    $(wildcard include/config/cmd/memory.h) \
    $(wildcard include/config/cmd/i2c.h) \
    $(wildcard include/config/cmd/itest.h) \
    $(wildcard include/config/cmd/pci.h) \
    $(wildcard include/config/cmd/setexpr.h) \
    $(wildcard include/config/cmd/bootd.h) \
    $(wildcard include/config/cmd/bootm.h) \
    $(wildcard include/config/cmd/nvedit/efi.h) \
    $(wildcard include/config/cmd/read.h) \
    $(wildcard include/config/cmdline.h) \
    $(wildcard include/config/needs/manual/reloc.h) \
    $(wildcard include/config/sys/maxargs.h) \
  ../include/env.h \
  ../include/compiler.h \
  /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include/stddef.h \
  /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include/stdbool.h \
  ../include/linux/string.h \
    $(wildcard include/config/sandbox.h) \
  ../include/linux/types.h \
    $(wildcard include/config/uid16.h) \
  ../include/linux/posix_types.h \
  ../include/linux/stddef.h \
  ../arch/arm/include/asm/posix_types.h \
  ../arch/arm/include/asm/types.h \
    $(wildcard include/config/arm64.h) \
    $(wildcard include/config/phys/64bit.h) \
    $(wildcard include/config/dma/addr/t/64bit.h) \
  ../include/asm-generic/int-ll64.h \
  ../arch/arm/include/asm/string.h \
    $(wildcard include/config/use/arch/memcpy.h) \
    $(wildcard include/config/use/arch/memset.h) \
  include/config.h \
    $(wildcard include/config/imx/config.h) \
    $(wildcard include/config/boarddir.h) \
  ../include/config_uncmd_spl.h \
    $(wildcard include/config/spl/dm.h) \
    $(wildcard include/config/dm/serial.h) \
    $(wildcard include/config/dm/i2c.h) \
    $(wildcard include/config/dm/spi.h) \
    $(wildcard include/config/dm/spi/flash.h) \
    $(wildcard include/config/dm/usb.h) \
    $(wildcard include/config/dm/stdio.h) \
  ../include/configs/phycore_imx8mm.h \
    $(wildcard include/config/sys/bootm/len.h) \
    $(wildcard include/config/spl/max/size.h) \
    $(wildcard include/config/sys/monitor/len.h) \
    $(wildcard include/config/sys/mmcsd/raw/mode/u/boot/use/sector.h) \
    $(wildcard include/config/sys/mmcsd/raw/mode/u/boot/sector.h) \
    $(wildcard include/config/sys/uboot/base.h) \
    $(wildcard include/config/spl/stack.h) \
    $(wildcard include/config/spl/bss/start/addr.h) \
    $(wildcard include/config/spl/bss/max/size.h) \
    $(wildcard include/config/sys/spl/malloc/start.h) \
    $(wildcard include/config/sys/spl/malloc/size.h) \
    $(wildcard include/config/malloc/f/addr.h) \
    $(wildcard include/config/spl/abort/on/raw/image.h) \
    $(wildcard include/config/sys/i2c.h) \
    $(wildcard include/config/extra/env/settings.h) \
    $(wildcard include/config/default/fdt/file.h) \
    $(wildcard include/config/sys/mmc/env/dev.h) \
    $(wildcard include/config/sys/mmc/img/load/part.h) \
    $(wildcard include/config/imx/hab.h) \
    $(wildcard include/config/bootcommand.h) \
    $(wildcard include/config/env/writeable/list.h) \
    $(wildcard include/config/env/flags/list/static.h) \
    $(wildcard include/config/loadaddr.h) \
    $(wildcard include/config/sys/load/addr.h) \
    $(wildcard include/config/sys/init/ram/addr.h) \
    $(wildcard include/config/sys/init/ram/size.h) \
    $(wildcard include/config/sys/init/sp/offset.h) \
    $(wildcard include/config/sys/init/sp/addr.h) \
    $(wildcard include/config/mmcroot.h) \
    $(wildcard include/config/sys/malloc/len.h) \
    $(wildcard include/config/sys/sdram/base.h) \
    $(wildcard include/config/mxc/uart/base.h) \
    $(wildcard include/config/sys/cbsize.h) \
    $(wildcard include/config/sys/bargsize.h) \
    $(wildcard include/config/fsl/usdhc.h) \
    $(wildcard include/config/sys/fsl/usdhc/num.h) \
    $(wildcard include/config/sys/fsl/esdhc/addr.h) \
    $(wildcard include/config/sys/i2c/speed.h) \
    $(wildcard include/config/mxc/usb/portsc.h) \
    $(wildcard include/config/usb/max/controller/count.h) \
    $(wildcard include/config/serial/tag.h) \
  ../include/linux/sizes.h \
  ../include/linux/const.h \
  ../include/linux/stringify.h \
  include/asm/arch/imx-regs.h \
    $(wildcard include/config/imx8mq.h) \
    $(wildcard include/config/imx8mp.h) \
    $(wildcard include/config/sys/fsl/sec/offset.h) \
    $(wildcard include/config/sys/fsl/sec/addr.h) \
    $(wildcard include/config/sys/fsl/jr0/offset.h) \
    $(wildcard include/config/sys/fsl/jr1/offset.h) \
    $(wildcard include/config/sys/fsl/jr0/addr.h) \
    $(wildcard include/config/sys/fsl/max/num/of/sec.h) \
  ../arch/arm/include/asm/mach-imx/regs-lcdif.h \
    $(wildcard include/config/mx28.h) \
    $(wildcard include/config/mx6sx.h) \
    $(wildcard include/config/mx6sl.h) \
    $(wildcard include/config/mx6sll.h) \
    $(wildcard include/config/mx6ul.h) \
    $(wildcard include/config/mx6ull.h) \
    $(wildcard include/config/mx7.h) \
    $(wildcard include/config/mx7ulp.h) \
    $(wildcard include/config/imx8m.h) \
    $(wildcard include/config/imx8.h) \
    $(wildcard include/config/imxrt.h) \
    $(wildcard include/config/mx23.h) \
  ../arch/arm/include/asm/mach-imx/regs-common.h \
  ../include/linux/bitops.h \
    $(wildcard include/config/sandbox/bits/per/long.h) \
  ../include/asm-generic/bitsperlong.h \
  ../include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/stack/validation.h) \
    $(wildcard include/config/kasan.h) \
  ../include/linux/compiler_types.h \
    $(wildcard include/config/have/arch/compiler/h.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/optimize/inlining.h) \
    $(wildcard include/config/cc/has/asm/inline.h) \
  ../include/linux/compiler_attributes.h \
  ../include/linux/compiler-gcc.h \
    $(wildcard include/config/retpoline.h) \
    $(wildcard include/config/arch/use/builtin/bswap.h) \
  ../include/linux/kernel.h \
  ../include/linux/printk.h \
    $(wildcard include/config/loglevel.h) \
    $(wildcard include/config/log.h) \
  ../include/log.h \
    $(wildcard include/config/log/max/level.h) \
    $(wildcard include/config/panic/hang.h) \
    $(wildcard include/config/log/error/return.h) \
    $(wildcard include/config/logf/file.h) \
    $(wildcard include/config/logf/line.h) \
    $(wildcard include/config/logf/func.h) \
  ../include/stdio.h \
    $(wildcard include/config/tpl/serial/support.h) \
    $(wildcard include/config/spl/serial/support.h) \
  /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include/stdarg.h \
  ../include/linker_lists.h \
    $(wildcard include/config/linker/list/align.h) \
  ../include/dm/uclass-id.h \
  ../include/linux/list.h \
  ../include/linux/poison.h \
  ../arch/arm/include/asm/bitops.h \
  ../include/asm-generic/bitops/__ffs.h \
  ../include/asm-generic/bitops/__fls.h \
  ../include/asm-generic/bitops/fls.h \
  ../include/asm-generic/bitops/fls64.h \
  ../arch/arm/include/asm/proc-armv/system.h \
    $(wildcard include/config/cpu/sa1100.h) \
    $(wildcard include/config/cpu/sa110.h) \
  ../include/configs/phycore_rauc_env.h \
  ../include/configs/phycore_fitimage_env.h \
  ../arch/arm/include/asm/config.h \
    $(wildcard include/config/lmb.h) \
    $(wildcard include/config/sys/boot/ramdisk/high.h) \
    $(wildcard include/config/arch/ls1021a.h) \
    $(wildcard include/config/cpu/pxa27x.h) \
    $(wildcard include/config/cpu/monahans.h) \
    $(wildcard include/config/cpu/pxa25x.h) \
    $(wildcard include/config/fsl/layerscape.h) \
  ../include/linux/kconfig.h \
  ../include/config_fallbacks.h \
    $(wildcard include/config/spl.h) \
    $(wildcard include/config/spl/pad/to.h) \
    $(wildcard include/config/sys/baudrate/table.h) \
    $(wildcard include/config/cmd/kgdb.h) \
    $(wildcard include/config/sys/pbsize.h) \
    $(wildcard include/config/sys/prompt.h) \
  ../include/linux/linux_string.h \
  ../arch/arm/include/asm/byteorder.h \
  ../include/linux/byteorder/little_endian.h \
  ../include/linux/byteorder/swab.h \
  ../include/linux/byteorder/generic.h \
  ../include/common.h \
  ../include/errno.h \
    $(wildcard include/config/errno/str.h) \
  ../include/linux/errno.h \
  ../include/time.h \
  ../include/linux/typecheck.h \
  ../arch/arm/include/asm/u-boot.h \
  ../include/asm-generic/u-boot.h \
    $(wildcard include/config/arm.h) \
    $(wildcard include/config/mpc8xx.h) \
    $(wildcard include/config/e500.h) \
    $(wildcard include/config/mpc86xx.h) \
    $(wildcard include/config/m68k.h) \
    $(wildcard include/config/mpc83xx.h) \
    $(wildcard include/config/cpm2.h) \
    $(wildcard include/config/extra/clock.h) \
    $(wildcard include/config/nr/dram/banks.h) \
  ../arch/arm/include/asm/u-boot-arm.h \
  ../include/display_options.h \
  ../include/vsprintf.h \
  ../include/fastboot.h \
    $(wildcard include/config/fastboot/flash.h) \
    $(wildcard include/config/fastboot/cmd/oem/format.h) \
    $(wildcard include/config/fastboot/cmd/oem/partconf.h) \
    $(wildcard include/config/fastboot/cmd/oem/bootbus.h) \
    $(wildcard include/config/fsl/fastboot.h) \
    $(wildcard include/config/fastboot/lock.h) \
    $(wildcard include/config/avb/support.h) \
    $(wildcard include/config/avb/atx.h) \
    $(wildcard include/config/virtual/ab/support.h) \
  ../include/malloc.h \
    $(wildcard include/config/sys/malloc/simple.h) \
  ../include/linux/usb/ch9.h \
  ../arch/arm/include/asm/unaligned.h \
  ../include/linux/unaligned/le_byteshift.h \
  ../include/linux/unaligned/be_byteshift.h \
  ../include/linux/unaligned/generic.h \
  ../include/linux/usb/gadget.h \
    $(wildcard include/config/usb/gadget/dualspeed.h) \
    $(wildcard include/config/usb/otg.h) \
    $(wildcard include/config/dm/usb/gadget.h) \
  ../include/usb.h \
    $(wildcard include/config/usb/musb/host.h) \
    $(wildcard include/config/usb/ehci/hcd.h) \
    $(wildcard include/config/usb/storage.h) \
    $(wildcard include/config/usb/host/ether.h) \
    $(wildcard include/config/usb/keyboard.h) \
  ../include/fdtdec.h \
    $(wildcard include/config/of/prior/stage.h) \
    $(wildcard include/config/multi/dtb/fit.h) \
    $(wildcard include/config/of/board.h) \
    $(wildcard include/config/of/separate.h) \
  ../include/linux/libfdt.h \
  ../include/linux/libfdt_env.h \
  ../include/linux/../../scripts/dtc/libfdt/libfdt.h \
  ../include/linux/../../scripts/dtc/libfdt/libfdt_env.h \
  ../include/linux/../../scripts/dtc/libfdt/fdt.h \
  ../include/pci.h \
    $(wildcard include/config/sys/pci/64bit.h) \
    $(wildcard include/config/dm/pci.h) \
    $(wildcard include/config/pci/indirect/bridge.h) \
    $(wildcard include/config/dm/pci/compat.h) \
    $(wildcard include/config/pci/fixup/dev.h) \
    $(wildcard include/config/mpc85xx.h) \
    $(wildcard include/config/pcie/imx.h) \
    $(wildcard include/config/pci/sriov.h) \
  ../include/pci_ids.h \
  ../include/dm/pci.h \
  ../include/usb_defs.h \
  ../arch/arm/include/asm/cache.h \
    $(wildcard include/config/sys/thumb/build.h) \
  ../arch/arm/include/asm/system.h \
    $(wildcard include/config/armv8/psci.h) \
    $(wildcard include/config/armv7/lpae.h) \
    $(wildcard include/config/cpu/v7a.h) \
    $(wildcard include/config/imx/trusty/os.h) \
    $(wildcard include/config/sys/arm/cache/writethrough.h) \
    $(wildcard include/config/sys/arm/cache/writealloc.h) \
    $(wildcard include/config/sys/arm/cache/writeback.h) \
    $(wildcard include/config/armv7/psci.h) \
    $(wildcard include/config/sys/noncached/memory.h) \
  ../arch/arm/include/asm/barriers.h \
  ../include/part.h \
    $(wildcard include/config/partition/uuids.h) \
    $(wildcard include/config/partition/type/guid.h) \
    $(wildcard include/config/dos/partition.h) \
    $(wildcard include/config/partitions.h) \
    $(wildcard include/config/spl/fs/ext4.h) \
    $(wildcard include/config/spl/fs/fat.h) \
    $(wildcard include/config/sys/mmcsd/raw/mode/u/boot/partition.h) \
    $(wildcard include/config/dual/bootloader.h) \
    $(wildcard include/config/efi/partition.h) \
  ../include/blk.h \
    $(wildcard include/config/sys/64bit/lba.h) \
    $(wildcard include/config/blk.h) \
    $(wildcard include/config/lba48.h) \
    $(wildcard include/config/block/cache.h) \
  ../include/efi.h \
    $(wildcard include/config/efi/stub/64bit.h) \
    $(wildcard include/config/x86/64.h) \
  ../include/linux/linkage.h \
  ../arch/arm/include/asm/linkage.h \
  ../include/ide.h \
    $(wildcard include/config/sys/ide/maxdevice.h) \
    $(wildcard include/config/sys/ide/maxbus.h) \
    $(wildcard include/config/sys/ata/base/addr.h) \
    $(wildcard include/config/ide/preinit.h) \
    $(wildcard include/config/of/ide/fixup.h) \
    $(wildcard include/config/ide/ahb.h) \
  ../include/uuid.h \
  ../include/part_efi.h \
    $(wildcard include/config/efi/partition/entries/numbers.h) \
  ../include/linux/compat.h \
    $(wildcard include/config/xen.h) \
    $(wildcard include/config/lbdaf.h) \
  ../include/console.h \
    $(wildcard include/config/console/record.h) \
    $(wildcard include/config/console/mux.h) \
  ../include/stdio_dev.h \
  ../include/iomux.h \
  ../arch/arm/include/asm/processor.h \
  ../arch/arm/include/asm/atomic.h \
    $(wildcard include/config/smp.h) \
  ../include/asm-generic/atomic.h \
  ../arch/arm/include/asm/ptrace.h \
  ../arch/arm/include/asm/proc-armv/ptrace.h \
  ../arch/arm/include/asm/proc-armv/processor.h \
  ../arch/arm/include/asm/proc-armv/domain.h \
  ../include/linux/err.h \
    $(wildcard include/config/err/ptr/offset.h) \
  ../include/linux/usb/composite.h \
  ../include/linux/bitmap.h \
  ../include/g_dnl.h \
  ../include/serial.h \
    $(wildcard include/config/post.h) \
    $(wildcard include/config/sys/post/uart.h) \
    $(wildcard include/config/arch/tegra.h) \
    $(wildcard include/config/sys/coreboot.h) \
    $(wildcard include/config/microblaze.h) \
    $(wildcard include/config/usb/tty.h) \
  ../include/post.h \
    $(wildcard include/config/post/external/word/funcs.h) \
    $(wildcard include/config/sys/post/word/addr.h) \
    $(wildcard include/config/arch/mpc8360.h) \
    $(wildcard include/config/sys/immr.h) \
    $(wildcard include/config/sys/mpc85xx/pic/offset.h) \
    $(wildcard include/config/sys/mpc86xx/pic/offset.h) \
    $(wildcard include/config/sys/post/rtc.h) \
    $(wildcard include/config/sys/post/watchdog.h) \
    $(wildcard include/config/sys/post/memory.h) \
    $(wildcard include/config/sys/post/cpu.h) \
    $(wildcard include/config/sys/post/i2c.h) \
    $(wildcard include/config/sys/post/cache.h) \
    $(wildcard include/config/sys/post/ether.h) \
    $(wildcard include/config/sys/post/usb.h) \
    $(wildcard include/config/sys/post/spr.h) \
    $(wildcard include/config/sys/post/sysmon.h) \
    $(wildcard include/config/sys/post/dsp.h) \
    $(wildcard include/config/sys/post/ocm.h) \
    $(wildcard include/config/sys/post/fpu.h) \
    $(wildcard include/config/sys/post/ecc.h) \
    $(wildcard include/config/sys/post/bspec1.h) \
    $(wildcard include/config/sys/post/bspec2.h) \
    $(wildcard include/config/sys/post/bspec3.h) \
    $(wildcard include/config/sys/post/bspec4.h) \
    $(wildcard include/config/sys/post/bspec5.h) \
    $(wildcard include/config/sys/post/codec.h) \
    $(wildcard include/config/sys/post/coproc.h) \
    $(wildcard include/config/sys/post/flash.h) \
    $(wildcard include/config/sys/post/mem/regions.h) \
  ../arch/arm/include/asm/io.h \
  ../arch/arm/include/asm/memory.h \
    $(wildcard include/config/discontigmem.h) \
  ../include/asm-generic/io.h \
  ../include/iotrace.h \
    $(wildcard include/config/io/trace.h) \

drivers/usb/gadget/f_fastboot.o: $(deps_drivers/usb/gadget/f_fastboot.o)

$(deps_drivers/usb/gadget/f_fastboot.o):
