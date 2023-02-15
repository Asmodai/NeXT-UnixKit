# Sudo 1.6.7, patchlevel 5 for NEXTSTEP 3.3.
----

This has been hacked (quickly, and probably poorly) to run on NEXTSTEP 3.3.

It can be compiled with the GNU C version 2.5.8.

Simply run `make`.  Do _not_ run `configure` unless you want to edit `Makefile` to add `-posix` and any relevant `-arch` flags.

# Rebuilding

`LDFLAGS="-posix" CFLAGS="-O3 -posix" ./configure --prefix=/usr/local`

If you wish to build a package, then use a prefix of, say, `/tmp/sudo` to isolate the package contents from the live system.
