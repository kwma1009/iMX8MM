cmd_u-boot-nodtb.bin := aarch64-phytec-linux-objcopy --gap-fill=0xff  -j .text -j .secure_text -j .secure_data -j .rodata -j .data -j .u_boot_list -j .rela.dyn -j .got -j .got.plt -j .binman_sym_table -j .text_rest -j .dtb.init.rodata -j .efi_runtime -j .efi_runtime_rel -O binary   u-boot u-boot-nodtb.bin && {  echo '  start=$$(aarch64-phytec-linux-nm  | grep __rel_dyn_start | cut -f 1 -d '\'' '\''); end=$$(aarch64-phytec-linux-nm  | grep __rel_dyn_end | cut -f 1 -d '\'' '\''); tools/relocate-rela   $$start $$end'; start=$$(aarch64-phytec-linux-nm u-boot | grep __rel_dyn_start | cut -f 1 -d ' '); end=$$(aarch64-phytec-linux-nm u-boot | grep __rel_dyn_end | cut -f 1 -d ' '); tools/relocate-rela u-boot-nodtb.bin 0x40200000 $$start $$end; } || { rm -f u-boot-nodtb.bin; false; }
