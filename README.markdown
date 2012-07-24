Introduction
============

ChmSee is an HTML Help viewer for Unix/Linux.

Homepage: [http://code.google.com/p/chmsee](http://code.google.com/p/chmsee)

Building
========

For ChmSee 2.0(1.99+), ChmSee has changed from GTK+ based application
to a xulrunner application.

Now in ChmSee, most of codes use cross-platform languages: JavaScript,
XUL, CSS. So building is unnecessary for them.

But you still need to compile a C/C++ component, it's a XPCOM
component which is used to retrieve HTML and image from packed chm
file. All source codes for this component are located in `chmsee/src`
directory.

Before building, you must confirm *xulrunner sdk* and *chmlib* already
installed in your system.

Then `cd chmsee/src` directory, choose one of Makefile.${OS} files and
rename it to Makefile. If there is no proper Makefile.${OS} file for
your OS in chmsee distribution, you can create one by copying from a
similar Makefile.${OS} or Makefile.sample and modify the INCLUDE and
LIB variables in it.

When the Makefile is ready, type `make` in the `chmsee/src` directory,
the compiled XPCOM component and its xpt defines will be output to
`chmsee/components` directory.

Install and running
===================

You can use xulrunner sdk tool *xulrunner-bin* to run chmsee directly
or install ChmSee to any directory you want.

For example, ChmSee source directory is `~/downloads/chmsee`, and
xulrunner sdk is installed to `/usr/lib/xulrunner-14.0.1`.

To launch ChmSee from chmsee directory:

    $ cd ~/downloads/chmsee
    $ /usr/lib/xulrunner-14.0.1/xulrunner-bin application.ini

If you want to install it to `~/tools/chmsee`, enter following command:

    $ /usr/lib/xulrunner-14.0.1/xulrunner-bin --install-app ~/downloads/chmsee ~/tools

The `~/tools/chmsee` directory will be created and contains a execute
file `chmsee`, running this file will launch ChmSee program.

If chmsee cannot launch and display a "Could not find the Mozilla
runtime." message. You need ensure that there is a *xulrunner runtime*
directory located in the installed chmsee folder, if not, add one by
yourself:

    $ cd ~/tools/chmsee
    $ ln -s /usr/lib/xulrunner-14.0.1 xulrunner

If you have *firefox* installed, this link can be changed to:

    $ ln -s /usr/lib/firefox xulrunner


Report bug
==========

If you encounter any ChmSee problem, please issue them to
[http://code.google.com/p/chmsee/issues/entry](http://code.google.com/p/chmsee/issues/entry),
thanks.

About ChmSee logo
=================

ChmSee logo comes from Open Clip Art Library.
[http://www.openclipart.org/detail/17922](http://www.openclipart.org/detail/17922)
The logo author is AJ Ashton.
