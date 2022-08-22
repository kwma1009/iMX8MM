cmd_u-boot.lds := aarch64-phytec-linux-gcc  --sysroot=/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot -E -Wp,-MD,./.u-boot.lds.d -D__KERNEL__ -D__UBOOT__   -D__ARM__           -fno-pic  -mstrict-align  -ffunction-sections -fdata-sections -fno-common -ffixed-r9     -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8 -mgeneral-regs-only   -Iinclude  -I../include  -I../arch/arm/include -include ../include/linux/kconfig.h  -nostdinc -isystem /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/bin/aarch64-phytec-linux/../../lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include -ansi -include ../include/u-boot/u-boot.lds.h -DCPUDIR=arch/arm/cpu/armv8  -D__ASSEMBLY__ -x assembler-with-cpp -std=c99 -P -o u-boot.lds ../arch/arm/cpu/armv8/u-boot.lds

source_u-boot.lds := ../arch/arm/cpu/armv8/u-boot.lds

deps_u-boot.lds := \
    $(wildcard include/config/armv8/secure/base.h) \
    $(wildcard include/config/armv8/psci.h) \
    $(wildcard include/config/armv8/psci/nr/cpus.h) \
    $(wildcard include/config/linux/kernel/image/header.h) \
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
  ../include/u-boot/u-boot.lds.h \
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
    $(wildcard include/config/sys/maxargs.h) \
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
  ../arch/arm/include/asm/psci.h \
    $(wildcard include/config/armv7/psci/nr/cpus.h) \

u-boot.lds: $(deps_u-boot.lds)

$(deps_u-boot.lds):
