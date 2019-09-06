Simple test of xgrep
====================

To build and run
----------------

first edit env.sh, pointing XGREP_DIR to the right place.

```
. ./env.sh
make clean; make
./test1
```

The bug
-----------
Ubuntu16 ships (sudo apt install -y libre2-dev) with a rather old re2.
This re2 engine will not accept `.*final.*`.   The Makefile has some
commented out compiler flag that will points to a more recent re2 from
vitessedata toolchain.  The output of newer re2 is 8 lines, which is 
the right result
```
grep final ./regex_testdata/nation.tbl | wc
```

test1 will run xre 10 times, in a loop.  From the result, first run 
will produce 1 line, later runs will produce 0 lines.

