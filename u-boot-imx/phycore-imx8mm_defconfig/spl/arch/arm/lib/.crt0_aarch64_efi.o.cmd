cmd_spl/arch/arm/lib/crt0_aarch64_efi.o := aarch64-phytec-linux-gcc  --sysroot=/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot -Wp,-MD,spl/arch/arm/lib/.crt0_aarch64_efi.o.d  -nostdinc -isystem /home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot-native/usr/bin/aarch64-phytec-linux/../../lib/aarch64-phytec-linux/gcc/aarch64-phytec-linux/10.2.0/include -Ispl/include -Iinclude  -I../include  -I../arch/arm/include -include ../include/linux/kconfig.h -D__KERNEL__ -D__UBOOT__ -DCONFIG_SPL_BUILD -D__ASSEMBLY__ -fno-PIE -g -D__ARM__ -mstrict-align -ffunction-sections -fdata-sections -fno-common -ffixed-r9 -fno-common -ffixed-x18 -pipe -march=armv8-a -D__LINUX_ARM_ARCH__=8 -mgeneral-regs-only   -c -o spl/arch/arm/lib/crt0_aarch64_efi.o ../arch/arm/lib/crt0_aarch64_efi.S

source_spl/arch/arm/lib/crt0_aarch64_efi.o := ../arch/arm/lib/crt0_aarch64_efi.S

deps_spl/arch/arm/lib/crt0_aarch64_efi.o := \
  ../include/linux/kconfig.h \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
    $(wildcard include/config/spl/.h) \
    $(wildcard include/config/tpl/build.h) \
    $(wildcard include/config/spl/build.h) \
    $(wildcard include/config/spl/foo.h) \
    $(wildcard include/config/tpl/foo.h) \
    $(wildcard include/config/spl/option.h) \
    $(wildcard include/config/spl/acme.h) \
    $(wildcard include/config/acme.h) \
    $(wildcard include/config/tpl/acme.h) \
  ../include/asm-generic/pe.h \

spl/arch/arm/lib/crt0_aarch64_efi.o: $(deps_spl/arch/arm/lib/crt0_aarch64_efi.o)

$(deps_spl/arch/arm/lib/crt0_aarch64_efi.o):
