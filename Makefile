CC := gcc
CFLAGS := -Wall -Wno-unused-result $(RPMCFLAGS)
LDFLAGS := $(RPMLDFLAGS)
INCLUDES := `pkg-config --cflags rpm`
LIBS := `pkg-config --libs rpm`
RPM_FORMAT_VERSION := `pkg-config --modversion rpm | cut -d . -f 1`
RPM_MAJOR_VERSION := `pkg-config --modversion rpm | cut -d . -f 1`
RPM_MINOR_VERSION := `pkg-config --modversion rpm | cut -d . -f 1`

rpm-specdump: rpm-specdump.o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

rpm-specdump.o: rpm-specdump.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
