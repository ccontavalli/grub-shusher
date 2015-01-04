What is this?
=============

On most systems, GRUB shows a message like

    GRUB loading.
    Weclome to GRUB!

at boot, just before loading the boot menu. Current versions of GRUB
do not provide any mechanism to disable this message without patching
and recompiling GRUB itself.

This process is cumbersone, as it requires expertise and forces you to
recompile / repatch every time you update grub.

`grub-shusher` contains two tiny .c files that instead of patching the
grub source code, they patch the grub binaries or installed master boot
record to disable those messages.

The software is generally safe: it looks for a specific set of patterns,
and if not all are found, it stops processing. It has been tested on a
few machines, and works as expected.

However, I only have access to a handful of machines, and the code
will modify your master boot record.

**USE IT AT YOUR OWN RISK - YOU ARE RESPONSIBLE FOR BACKING UP AND RESTORING YOUR DATA**

At this point, it has been tested on systems with:

   * EFI
   * GPT
   * Standard partition table

If you have success / failure stories to tell, please email me
directly (ccontavalli AT gmail.com) or open issues on github
(https://github.com/ccontavalli/grub-shusher/issues).


How to use grub-shusher
=======================

**REPLACE /dev/sda with your GRUB PARTITION, used with grub-setup or grub-install**


On Debian Systems with normal partition table (no EFI, no GPT)
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


On Any Other System (TM) with normal partition table (no EFI, no GPT)
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


On Any System (TM) with EFI
------------------------

    $ make
    $ sudo -s
    # ./grub-kernel /boot/efi/EFI/***/grubx64.efi
    
... and done. Important: 

* make always sure that you have made a backup of `grubx64.efi`

* replace `***` with the distribution name, for example `/boot/efi/EFI/manjaro/grubx64.efi`.

... and go read configuring grub.


On Any System (TM) with GPT
-------------------------

    $ make
    $ sudo -s
    # ./mbr -g /dev/sda
    # ./mbr -g /dev/bios-boot-partition

Note:

   * You must provide the *-g* flag, this is important for GPT partitions.
   * You must run `mbr` twice:

      1. With the partition GRUB was told to use (/dev/sda, in this document).
      2. With the GPT partition, the one marked with the EF02 type. You can
         find this partition by running something like:
    
    # parted /dev/sda print


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


Contributors and THANKS!
------------------------

Thanks go to:

   * Carlo Contavalli, www.github.com/ccontavalli - main author.
   * cybnetsurfe3011, www.github.com/cybnetsurfe3011 - GPT support.
   * vincent-t, www.github.com/vincent-t - EFI testing and documentation.
