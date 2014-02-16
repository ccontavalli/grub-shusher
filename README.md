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

The software is as safe as I could make it: it looks for a specific set of patterns,
and if not all are found, it stops processing. I have tested it on a few machines,
and it is working.

Consider though that they will read your master boot record and modify it. This
probably does not work on EFI systems.


**USE THEM AT YOUR OWN RISK**

**ONLY TESTED on AMD64 using BIOS BOOT - NO EFI**


How to use grub-shusher
=======================

**REPLACE /dev/sda with your GRUB PARTITION, used with grub-setup or grub-install**


On Debian Systems
-----------------

    # ./setup-debian.sh /dev/sda

... and go read [configuring grub](#configuring-grub).

If you want to setup grub-shusher automatically after each upgrade, you
can edit `/etc/apt/apt.conf`, and add a section like:

    DPkg
    {
        Post-Invoke {"/opt/projects/grub-shusher/setup-debian.sh /dev/sda >/dev/null 2>/dev/null || true;";};
    }

where `/opt/projects/grub-shuser` is the directory where you downloaded grub-shusher, and `/dev/sda` is
the partition or disk where grub is installed.


On Any Other System (TM)
------------------------

    $ make
    $ sudo -s
    # ./grub-kernel /boot/grub/kernel.img
    # ./grub-kernel /usr/lib/grub/i386-pc/kernel.img
    # grub-install /dev/sda
    # ./mbr /dev/sda


... and done. Note that the order is important:

  1. `make` will compile the code, you need to have GCC installed.
  2. `grub-kernel ...` will remove the 'Welcome to GRUB!' message from the `kernel.img` file.
  3. `grub-install /dev/sda` will create a new compressed image
     (by merging several other files, including kernel.img) and install it on your disk.
  4. `mbr /dev/sda` will remove a few other messages from the installed mbr.

... and go read [configuring grub](#configuring-grub).


Configuring GRUB
----------------

To make GRUB entirely quiet, my `/etc/defaults/grub` has:

    GRUB_DEFAULT=0
    GRUB_TIMEOUT=0
     
    GRUB_HIDDEN_TIMEOUT=5
    GRUB_HIDDEN_TIMEOUT_QUIET=true

Note that once you have those lines, you need to run `update-grub`, and on next reboot,
you will have to press 'ESC' or keep 'shift' pressed to get into the grub menu. I suggest
you try this before you `shush` grub.

If something goes wrong, you can:

    # apt-get install --reinstall grub2
    # grub-install /dev/sda

to clean up after yourself.

You can read more about [grub-shusher on this blog post](http://rabexc.org/posts/grub-shush).

Getting back into GRUB
----------------------

Once you change `/etc/defaults/grub` and run `update-grub`, grub will no longer show up at
boot. To get into the menu, you can try two things:

   * Keep `shift` pressed during boot. This is the documented mechanism, did not work for me.
   * Press `ESC` at boot. This is a bit tricky, as if you press it too early, most BIOSes
     will bring you in the BIOS menu. If you press it too late, well, the operating system
     will have booted already. The rule of thumb is to press it at the time the GRUB menu
     would have showed up, had it been enabled. If you use the settings above, you have *5*
     seconds of window to press the button. On my laptopt, this is right after the screen
     is cleared.
