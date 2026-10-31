A8Manager for Linux - test build
================================

Thanks for testing! This is the first Linux build of A8Manager, so please report
anything that does not work, looks wrong, or behaves differently from Windows/macOS.

Requirements
------------
- 64 bit x86 (x86_64) Linux. ARM machines such as a Raspberry Pi are not supported.
- Ubuntu 22.04, Debian 12, Fedora 36, Linux Mint 21 or newer, or any current
  rolling release such as Arch. Older distros will refuse to start it with an
  error mentioning GLIBC.
- A desktop session. Wayland desktops run it through XWayland, which is
  installed by default on GNOME and KDE.

Running it
----------
Run it straight from this folder:

    ./A8Manager

Or install it for your user, which adds it to your applications menu:

    ./install.sh

and remove it again with:

    ./install.sh --uninstall

Installing needs no root access - everything goes under ~/.local.

Things worth knowing
--------------------
- Deleting files sends them to the Trash. On an SD card the desktop keeps that
  Trash on the card itself, in a hidden .Trash-<number> folder (usually
  .Trash-1000), so the space is not
  freed until you empty the Trash. Please tell me if this causes problems on the
  Assimil8or.
- Audio playback has not been tested yet - please try previewing samples and
  let me know whether it works and which audio device you selected.

Reporting problems
------------------
Please include:
- your distro and version (the output of: cat /etc/os-release)
- whether you are on Wayland or X11 (the output of: echo $XDG_SESSION_TYPE)
- any messages printed when you start ./A8Manager from a terminal
