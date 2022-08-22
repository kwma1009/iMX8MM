cmd_arch/arm/dts/imx8mn-beacon-kit.dtb := mkdir -p arch/arm/dts/ ; (cat ../arch/arm/dts/imx8mn-beacon-kit.dts; echo '$(pound)include "imx8mn-beacon-kit-u-boot.dtsi"') > arch/arm/dts/.imx8mn-beacon-kit.dtb.pre.tmp; aarch64-phytec-linux-gcc  --sysroot=/home/jma/phytec_works/iMX8MM.test_image/build/tmp/work/phyboard_polis_imx8mm_4-phytec-linux/u-boot-imx/2021.04-r0/recipe-sysroot -E -Wp,-MD,arch/arm/dts/.imx8mn-beacon-kit.dtb.d.pre.tmp -nostdinc -I../arch/arm/dts -I../arch/arm/dts/include -Iinclude -I../include -I../arch/arm/include -include ../include/linux/kconfig.h -D__ASSEMBLY__ -undef -D__DTS__ -x assembler-with-cpp -o arch/arm/dts/.imx8mn-beacon-kit.dtb.dts.tmp arch/arm/dts/.imx8mn-beacon-kit.dtb.pre.tmp ; ./scripts/dtc/dtc -O dtb -o arch/arm/dts/imx8mn-beacon-kit.dtb -b 0 -i ../arch/arm/dts/  -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-graph_port -Wno-unique_unit_address -Wno-simple_bus_reg -Wno-pci_device_reg -Wno-pci_bridge -Wno-pci_device_bus_num  -@  -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-graph_port -Wno-unique_unit_address -Wno-simple_bus_reg -Wno-pci_device_reg -Wno-pci_bridge -Wno-pci_device_bus_num  -@ -d arch/arm/dts/.imx8mn-beacon-kit.dtb.d.dtc.tmp arch/arm/dts/.imx8mn-beacon-kit.dtb.dts.tmp || (echo "Check /home/jma/phytec_works/iMX8MM.test_image/build/workspace/sources/u-boot-imx/phycore-imx8mm_defconfig/arch/arm/dts/.imx8mn-beacon-kit.dtb.pre.tmp for errors" && false) ; sed "s:arch/arm/dts/.imx8mn-beacon-kit.dtb.pre.tmp:../arch/arm/dts/imx8mn-beacon-kit.dts:" arch/arm/dts/.imx8mn-beacon-kit.dtb.d.pre.tmp arch/arm/dts/.imx8mn-beacon-kit.dtb.d.dtc.tmp > arch/arm/dts/.imx8mn-beacon-kit.dtb.d

source_arch/arm/dts/imx8mn-beacon-kit.dtb := ../arch/arm/dts/imx8mn-beacon-kit.dts

deps_arch/arm/dts/imx8mn-beacon-kit.dtb := \
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
  ../arch/arm/dts/imx8mn.dtsi \
  ../arch/arm/dts/include/dt-bindings/clock/imx8mn-clock.h \
  ../arch/arm/dts/include/dt-bindings/gpio/gpio.h \
  ../arch/arm/dts/include/dt-bindings/input/input.h \
  ../arch/arm/dts/include/dt-bindings/input/linux-event-codes.h \
  ../arch/arm/dts/include/dt-bindings/reset/imx8mn-dispmix.h \
  ../arch/arm/dts/include/dt-bindings/interrupt-controller/arm-gic.h \
  ../arch/arm/dts/include/dt-bindings/interrupt-controller/irq.h \
  ../arch/arm/dts/include/dt-bindings/thermal/thermal.h \
  ../arch/arm/dts/imx8mn-pinfunc.h \
  ../arch/arm/dts/imx8mn-beacon-som.dtsi \
  ../arch/arm/dts/imx8mn-beacon-baseboard.dtsi \
  ../arch/arm/dts/imx8mn-beacon-kit-u-boot.dtsi \

arch/arm/dts/imx8mn-beacon-kit.dtb: $(deps_arch/arm/dts/imx8mn-beacon-kit.dtb)

$(deps_arch/arm/dts/imx8mn-beacon-kit.dtb):
