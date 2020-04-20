#! /bin/bash
MK_BINARY_PATH=
cd ../../build/arm-softmmu
./qemu-system-arm -monitor stdio -M stm32f107 -cpu cortex-m3   -kernel ${MK_BINARY_PATH}
