
Debian
====================
This directory contains files used to package hashd/hash-qt
for Debian-based Linux systems. If you compile hashd/hash-qt yourself, there are some useful files here.

## hash: URI support ##


hash-qt.desktop  (Gnome / Open Desktop)
To install:

	sudo desktop-file-install hash-qt.desktop
	sudo update-desktop-database

If you build yourself, you will either need to modify the paths in
the .desktop file or copy or symlink your hashqt binary to `/usr/bin`
and the `../../share/pixmaps/hash128.png` to `/usr/share/pixmaps`

hash-qt.protocol (KDE)

