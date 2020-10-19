RPM_FORMAT_VERSION := `pkg-config --modversion rpm | cut -d . -f 1`
RPM_MAJOR_VERSION := `pkg-config --modversion rpm | cut -d . -f 2`
RPM_MINOR_VERSION := `pkg-config --modversion rpm | cut -d . -f 3`

CC := gcc
CFLAGS := -Wall -Wno-unused-result -DRPM_FORMAT_VERSION=$(RPM_FORMAT_VERSION) -DRPM_MAJOR_VERSION=$(RPM_MAJOR_VERSION) -DRPM_MINOR_VERSION=$(RPM_MINOR_VERSION) $(RPMCFLAGS)
LDFLAGS := $(RPMLDFLAGS)
INCLUDES := `pkg-config --cflags rpm`
LIBS := `pkg-config --libs rpm`
rpm-specdump: rpm-specdump.o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

rpm-specdump.o: rpm-specdump.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@
