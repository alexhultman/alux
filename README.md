# alux OS - let's build a minimal linux OS

This repo holds, for educational purposes, step-by-step procedures for creating a minimal distribution of the Linux kernel.

The immediate goal is to build from scratch a bootable USB-stick which can boot up a (custom made) graphical shell with mouse and keyboard support and eventually support basic Wayland apps. We are not going to use any existing user space apps such as Xorg, bash or systemd. This is a bare minimum Linux kernel distribution with graphics and devices working.

By only relying on the very Linux kernel and nothing else, one can focus ones attention to this piece of software without the fluffy stuff around.

| The software stack | The vector |
|---|---|
| ![](misc/stack.png) | ![](https://img1-327a.kxcdn.com/DataImage.ashx/9687377/350/350)   |

## Make a bootable USB stick (wip)
We want to support nothing but UEFI motherboards, so grab a USB stick and partition it with a GPT, not MBR.

* Put a Fat32 parition of roughly 100mb in the top, mark it as ESP (EFI System Parition) - call it boot
* Add an ext4 parition for remaining space, call it root
* Run bootctl install --path /dev/sdXY on the mounted EFI partition to install systemd-boot (a bootloader, not systemd)

```
[   boot   |                  root                  ]
```

* Boot parition will hold the linux kernel image and its parameters,
* Root partition will hold everything user-space and its needed images, ELF binaries, etc.

## Build the shell and populate the root partition

* shell is a dynamically linked ELF binary depending on a few shared objects (.so). Depending on what system you build on it may look different. Use LDD to track dependencies of shell ELF and ship them in /lib64, /usr/lib64 depending on need.
* wallpaper.png lies in root /wallpaper.png

## Take note of the PARTGUID of root partition and add systemd-boot entries
* yes

## Copy the Linux kernel or build it from source
* lies in /boot/EFI/whatever_you_configured_systemd
