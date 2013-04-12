AChat
=====

A simple multi-user chat server/client. The client has a UI based on curses.

Usage
-----

First run the server to start listening for connections:

    $ ./AChat_server 
    server: waiting for connections...

Then connect to it using the client:

    $ ./AChat_client <host> <username>
    

Build
-----

AChat is built using the GCC toolchain under Linux and Window. Run

    make
    
to build the server and the client (to folder `bin`). The client depends on libpthread and libcurses so make sure they're available (they're usually available by default under Linux. MinGW usually includes libpthread and for libcurses see [PDCurses](http://sourceforge.net/apps/mediawiki/cobcurses/index.php?title=Install-MinGW-pdcurses)).

Screenshots
-----------

![AChat_client Linux](http://i.imgur.com/boLeg37.png)

![AChat_client Windows](http://i.imgur.com/FEd5Qq4.png)

Licensing
---------

AChat is distributed under the BSD-2 License. See [<code>LICENSE.md</code>](LICENSE.md) for details.
