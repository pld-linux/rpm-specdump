CC := gcc
CFLAGS :=
LDFLAGS :=
CPPFLAGS :=
INCLUDES := -I/usr/include/rpm
LIBS := -lrpm -lrpmdb -lrpmio -lrpmbuild

rpm-specdump: rpm-specdump.o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

rpm-specdump.o: rpm-specdump.c
	$(CC) $(CFLAGS) $(INCLUDES) -W -Wall -c $< -o $@
