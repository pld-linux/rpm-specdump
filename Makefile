CC := gcc
CFLAGS := -Wall -Wno-unused-result $(RPMCFLAGS)
LDFLAGS := $(RPMLDFLAGS)
INCLUDES := `pkg-config --cflags rpm`
LIBS := `pkg-config --libs rpm`

rpm-specdump: rpm-specdump.o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

rpm-specdump.o: rpm-specdump.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
