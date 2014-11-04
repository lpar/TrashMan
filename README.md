
# TrashMan

This is a small POSIX ANSI C utility I wrote to clean up my download folder
and temporary work-in-progress folder automatically.

Basically, I operate the folder equivalent of Inbox Zero. If something I
download or create is important, I'll file it away in its proper place.
If I haven't done so within a week or two, it's obviously crap and can be
deleted. So I have a program do so automatically, without even asking me. So
before I go any further...

## Warning

*This program deletes files.* If you use the `-d` flag then it may perform the
equivalent of `rm -rf` automatically, with no "Are you sure?" prompt. This is a
feature, not a bug.

You are responsible for making sure you keep backups, and for making sure you
only run the program on your downloads folder, temporary work folders, and
other folders full of junk you don't want to keep forever.

If you are not fully comfortable with the Unix command line, run away from this 
software as quickly as you can. 

If you are one of those people who aliases `rm` to `rm -i` because otherwise
you sometimes delete the wrong file, then this is not the program for you.

This program may have bugs which result in loss of data. I run it every day,
but I can't make any guarantees that it won't misbehave for you. The risk is
yours. At least the code is short enough that you can probably read it through
to check that it looks OK, and it's not in a scripting language so there
shouldn't be any problems caused by globbing differences, filenames with spaces
and other punctuation in, or other stuff that frequently causes shell scripts
to malfunction.

## How I got here

I originally tried to use standard `tmpwatch`-like utilities to keep my
Downloads folder from getting out of control; then I tried `find`. In both
cases I discovered problems with using the standard tools.

Firstly, downloading files via Safari on my Mac preserves their original
creation and modification dates. Right now, my download folder has files in it
dated late 2004. So using creation or modification dates, files would get
deleted immediately, even if I had only just downloaded them.

Next, downloaded and unpacked folders will often contain files which have
different creation or modification dates from the folder. So the `tmpwatch`
approach would often leave me with a half-deleted folder. This was particularly
problematic on the Mac, where applications are a special kind of folder
— I'd launch an app I'd downloaded the day before and it would crash because
some vital part of its innards had been removed.

Both OS X and Linux support access times. Unfortunately, OS X Spotlight reads
files to index them, which counts as accessing them; and Linux file browsers
read files to create preview icons, which also resets the last access time. So
using access time rather than creation or modification time didn't work well.

Next, I discovered that OS X had a feature whereby it recorded when a file was
first added to the Downloads folder. I tried writing a utility to pull that
information out using `mdfind`. That worked moderately well on the Mac, but
relied on Spotlight having indexed the directory, and hence didn't seem work at
all on any type of file Spotlight didn't recognize. Also, it was no use on
Linux.

My next solution was a Ruby program which basically worked like TrashMan, but
stored the timestamps in a hidden dotfile in the directory. Like TrashMan, it
only timestamped the top level of my Downloads directory, and it always removed
apps (and other subdirectories) all at once. I ran that for a couple of months,
and it basically worked, except that getting it to run reliably from crontab
was a pain because of dependencies and having multiple Ruby versions installed.

Still, I had proved to myself that the concept was sound. So I sat down and
wrote a C version, and made it use extended attributes to store the timestamps
instead of a clunky dotfile so that files would keep their timestamps even if I
renamed them.

## Portability notes

I care about Mac OS X and Linux, and it works on both of those. It should work
on BSD too, but I don't have a BSD system to test it on.

It won't work on Windows, and I don't care because I don't use Windows.

You need extended attribute support enabled in the kernel and filesystem. This
is true by default on OS X 10.10, Debian, and Fedora 20.

## License

GNU General Public License version 3 or later. No warranty express or implied.
Use only as directed.

