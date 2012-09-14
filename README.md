Introduction
============

ChmSee is an HTML Help viewer for Unix/Linux.

Homepage: [http://code.google.com/p/chmsee](http://code.google.com/p/chmsee)

Building
========

For ChmSee 2.0(1.99+), ChmSee has been changed from GTK+ based
application to a xulrunner application.

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
similar Makefile.${OS} or Makefile.sample and modify the INCLUDE, LIB
or CFLAGS variables in it.

When the Makefile is ready, type GNU `make` in the `chmsee/src`
directory. If building succeed, the compiled XPCOM component and its
xpt defines will be output to `chmsee/components` directory.

Running
=======

You can launch ChmSee immediately in the chmsee folder with xulrunner
platform command from xulrunner sdk or firefox.

In the following examples, ChmSee source directory is located in
`~/downloads/chmsee`, xulrunner sdk is installed to
`/usr/lib/xulrunner-sdk` and  firefox is installed to
`/usr/lib/firefox`.

Launch ChmSee with xulrunner sdk tool:

    $ cd ~/downloads/chmsee
    $ /usr/lib/xulrunner-sdk/xulrunner application.ini

Launch ChmSee with firefox:

    $ cd ~/downloads/chmsee
    $ /usr/lib/firefox/firefox --app application.ini

Installing
==========

You can also use xulrunner sdk tool *xulrunner* to install ChmSee
to any directory you want.

If you want to install it to `~/tools/chmsee`, enter following command:

    $ /usr/lib/xulrunner-sdk/xulrunner --install-app ~/downloads/chmsee ~/tools

The `~/tools/chmsee` directory will be created and contains an execute
file `chmsee`, running this file will launch ChmSee program.

If chmsee can not launch and display a "Could not find the Mozilla
runtime." message. You need ensure that there is a *xulrunner runtime*
directory located in the installed chmsee folder, if not, add one by
yourself:

    $ cd ~/tools/chmsee
    $ ln -s /usr/lib/xulrunner-sdk xulrunner

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
