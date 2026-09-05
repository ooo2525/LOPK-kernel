# Reset OVMF files
cp /usr/share/OVMF/OVMF_CODE_4M.fd ./OVMF_CODE_4M.fd
cp OVMF_VARS_clean.fd OVMF_VARS_test.fd

# Rebuild
./build.sh

# Run QEMU
qemu-system-x86_64 \
    -drive if=pflash,format=raw,readonly=on,file=OVMF_CODE_4M.fd \
    -drive if=pflash,format=raw,file=OVMF_VARS_test.fd \
    -drive format=raw,file=test.img
