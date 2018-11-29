### Toolchain
You can either build or download a ready-to-use toolchain. An example of such toolchains are from Linaro website.

When writing this document the following toolchain was used – http://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/gcc-linaro-7.3.1-2018.05-i686_aarch64-linux-gnu.tar.xz

Linaro updates it’s toolchain quite often and more frequent one can be downloaded from here – http://releases.linaro.org/components/toolchain/binaries/

Download and extract the toolchain into some place; and as instructed below the CROSS_COMPILE environment variables needs to be set to the path of the toolchain prefex.

For instance if the toolchain was extracted under /opt/imx8m/toolchain/gcc-linaro-7.3.1-2018.05-i686_aarch64-linux-gnu/ then the CROSS_COMPILE needs to be set as follows –
```
export ARCH=arm64
export CROSS_COMPILE=/opt/imx8m/toolchain/gcc-linaro-7.3.1-2018.05-i686_aarch64-linux-gnu/bin/aarch64-linux-gnu
```

### Download Source and Firmware
```
git clone https://github.com/SolidRun/arm-trusted-firmware.git -b imx-atf-v1.6
git clone https://github.com/SolidRun/u-boot.git -b v2018.11-solidrun
wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-7.9.bin
```

### ATF
Building ATF is as follows – *make sure you have set your ARCH and CROSS_COMPILE environment variables as noted above*
```
cd arm-trusted-firmware
make PLAT=imx8mq bl31
cp build/imx8mq/release/bl31.bin ../u-boot/
cd ..
```

### Extract and copy firmware
Extract the NXP firmware archive and accept the end user agreement
```
chmod +x firmware-imx-7.9.bin
./firmware-imx-7.9.bin
cp firmware-imx-7.9/firmware/hdmi/cadence/signed_hdmi_imx8m.bin u-boot/
cp firmware-imx-7.9/firmware-imx-7.9/firmware/ddr/synopsys/lpddr4*.bin u-boot/
```

### U-Boot
Build U-Boot and generate the image - *make sure you have set your ARCH and CROSS_COMPILE environment variables as noted above*
```
make imx8mq_hb_defconfig
make flash.bin
```

### Writing to SD-Card
```
sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33
```
