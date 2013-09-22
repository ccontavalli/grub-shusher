What is this?
=============

At boot time, GRUB on Debian and most other distros will show something like:

    GRUB loading.
    Weclome to GRUB!

and then load the boot menu.

By editing `/etc/default/grub`, you can easily disable the boot menu. However,
to disable those two messages, you have to patch `grub` and recompile, which is
cumbersone, as it forces you to maintain your own `.deb` files up-to-date with
the needed patches.

`grub-shusher` contains two tiny .c files that will patch your master boot record
and grub files to disable those two messages.   

They are relatively risky to use, as they will read your disk and try to patch
it on the fly, but have used them on a few systems without issues.

**USE THEM AT YOUR OWN RISK**


How to use them
===============

**REPLACE /dev/sda with your GRUB PARTITION, used with grub-setup**

    $ make
    $ sudo -s
    # ./mbr /dev/sda
    # ./grub-kernel /boot/grub/kernel.img

... and done.

To make GRUB entirely quiet, my `/etc/defaults/grub` has:

    GRUB_DEFAULT=0
    GRUB_TIMEOUT=0
     
    GRUB_HIDDEN_TIMEOUT=5
    GRUB_HIDDEN_TIMEOUT_QUIET=true

Note that once you have those lines, you need to run `update-grub`, and on next reboot,
you will have to press 'ESC' or keep 'shift' pressed to get into the grub menu. I suggest
you try this before you `shush` grub.

If something goes wrong, you can:

    # grub-setup /dev/sda
    # apt-get install --reinstall grub2

to clean up after yourself.
