/*
 * $Id$
 *
 * Prints out following information in same format as %dump for builder:
 *
 *  	$2 ~ /^PACKAGE_/ {print}
 *  	$2 ~ /^SOURCEURL/ {print}
 *  	$2 ~ /^PATCHURL/  {print}
 *  	$2 ~ /^nosource/ {print}
 *  	$2 ~ /^PACKAGE_/ {print}
 *
 *  $ rpm-specdump qemu.spec
 *  h PACKAGE_NAME qemu
 *  h PACKAGE_VERSION 0.9.0
 *  h PACKAGE_RELEASE 60k
 *  source: /home/glen/rpm/pld/SOURCES
 *  patch: /home/glen/rpm/pld/SOURCES
 *  s PATCH13 /home/glen/rpm/pld/SOURCES/qemu-dosguest.patch
 *  s PATCH11 /home/glen/rpm/pld/SOURCES/qemu-0.7.2-gcc4-opts.patch
 *  s PATCH9 /home/glen/rpm/pld/SOURCES/qemu-0.8.0-gcc4-hacks.patch
 *  s PATCH8 /home/glen/rpm/pld/SOURCES/qemu-kde_virtual_workspaces_hack.patch
 *  s PATCH6 /home/glen/rpm/pld/SOURCES/qemu-nosdlgui.patch
 *  s PATCH5 /home/glen/rpm/pld/SOURCES/qemu-gcc4_ppc.patch
 *  s PATCH4 /home/glen/rpm/pld/SOURCES/qemu-gcc4_x86.patch
 *  s PATCH3 /home/glen/rpm/pld/SOURCES/qemu-dot.patch
 *  s PATCH1 /home/glen/rpm/pld/SOURCES/qemu-cc.patch
 *  s PATCH0 /home/glen/rpm/pld/SOURCES/qemu-nostatic.patch
 *  s SOURCE1 /home/glen/rpm/pld/SOURCES/kqemu-1.3.0pre11.tar.gz
 *  s SOURCEURL1 http://fabrice.bellard.free.fr/qemu/kqemu-1.3.0pre11.tar.gz
 *  s SOURCE0 /home/glen/rpm/pld/SOURCES/qemu-0.9.0.tar.gz
 *  s SOURCEURL0 http://fabrice.bellard.free.fr/qemu/qemu-0.9.0.tar.gz
 *
 * Version 0.1, 2008-01-23
 * - initial version, based on getdeps.c
 */


#define _GNU_SOURCE

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <grp.h>
#include <sys/time.h>
#include <sys/types.h>

#include <rpmbuild.h>
#include <rpmlib.h>
#include <header.h>
#include <rpmts.h>

#define ARG_WITH	1024
#define ARG_WITHOUT	1025
#define ARG_DEFINE	1026
#define ARG_TARGET	1027
#define ARG_RCFILE	1028
#define ARG_CHROOT	1029
#define ARG_UID		1030
#define ARG_GID		1031

static struct option const
CMDLINE_OPTIONS[] = {
  { "help",     no_argument,  0, 'h' },
  { "version",  no_argument,  0, 'v' },
  { "with",     required_argument, 0, ARG_WITH    },
  { "without",  required_argument, 0, ARG_WITHOUT },
  { "define",   required_argument, 0, ARG_DEFINE  },
  { "target",   required_argument, 0, ARG_TARGET  },
  { "rcfile",   required_argument, 0, ARG_RCFILE  },
  { "chroot",   required_argument, 0, ARG_CHROOT  },
  { "uid",	required_argument, 0, ARG_UID },
  { "gid",	required_argument, 0, ARG_GID },
  { 0,0,0,0 }
};

struct Arguments
{
    char const *	target;
    char const *	rcfile;
    char const *	chroot;
    uid_t		uid;
    gid_t		gid;
    
    struct {
	char const **	values;
	size_t		cnt;
	size_t		reserved;
    }			macros;

    char const *	specfile;
};

struct DepSet {
    int32_t const *	flags;
    char const **	name;
    char const **	version;
    ssize_t		cnt;
};

inline static void 
writeStr(int fd, char const *cmd)
{
  (void)write(fd, cmd, strlen(cmd));
}

#define WRITE_MSG(FD,X)		(void)(write(FD,X,sizeof(X)-1))
#define WRITE_STR(FD,X)		writeStr(FD,X)

static void
showHelp(int fd, char const *cmd, int res)
{
  char		tmp[strlen(cmd)+1];
  strcpy(tmp, cmd);
  
  WRITE_MSG(fd, "Usage:  ");
  WRITE_STR(fd, basename(tmp));
  WRITE_MSG(fd,
	    " [--define '<macro> <value>']* [--with[out] <key>]* [--chroot <dir>]\n"
	    "                [--target <target>] [--rcfile <rcfile>] [--] <specfile>\n");
  exit(res);
}

static void
addDefine(struct Arguments *args, char const *val)
{
  register size_t	c = args->macros.cnt;
  if (args->macros.reserved <= c) {
    args->macros.reserved *= 2;
    args->macros.reserved += 1;
  
    args->macros.values = realloc(args->macros.values,
				  args->macros.reserved * sizeof(char const *));
    if (args->macros.values==0) {
      perror("realloc()");
      exit(1);
    }
  }

  args->macros.values[c] = strdup(val);
  ++args->macros.cnt;
}

static void
setWithMacro(struct Arguments *args,
	     char const *name, char const *prefix, size_t prefix_len)
{
  size_t	len = strlen(name);
  char		tmp[2*len + 2*prefix_len + sizeof("__ ---")];
  char *	ptr = tmp;

  // set '_<prefix>_<name>'
  *ptr++ = '_';
  memcpy(ptr, prefix, prefix_len); ptr += prefix_len;
  *ptr++ = '_';
  memcpy(ptr, name,   len);        ptr += len;
  *ptr++ = ' ';

  // append ' --<prefix>-<name>'
  *ptr++ = '-';
  *ptr++ = '-';
  memcpy(ptr, prefix, prefix_len); ptr += prefix_len;
  *ptr++ = '-';
  memcpy(ptr, name,   len);        ptr += len;
  *ptr   = '\0';

  addDefine(args, tmp);
}


static void
parseArgs(struct Arguments *args, int argc, char *argv[])
{
  while (1) {
    int		c = getopt_long(argc, argv, "", CMDLINE_OPTIONS, 0);
    if (c==-1) break;
    switch (c) {
      case 'h'		:  showHelp(1, argv[0], 0);
      case ARG_TARGET	:  args->target = optarg; break;
      case ARG_RCFILE	:  args->rcfile = optarg; break;
      case ARG_CHROOT	:  args->chroot = optarg; break;
      case ARG_UID	:  args->uid    = atoi(optarg); break;
      case ARG_GID	:  args->gid    = atoi(optarg); break;
      case ARG_DEFINE	:  addDefine(args, optarg); break;
      case ARG_WITH	:  setWithMacro(args, optarg, "with",    4); break;
      case ARG_WITHOUT	:  setWithMacro(args, optarg, "without", 7); break;
      default:
	WRITE_MSG(2, "Try '");
	WRITE_STR(2, argv[0]);
	WRITE_MSG(2, " --help\" for more information.\n");
	exit(1);
    }
  }

  if (optind+1!=argc) {
    write(2, "No/too much specfile(s) given; aborting\n", 40);
    exit(1);
  }

  if (args->gid==(gid_t)(-1))
    args->gid = args->uid;

  args->specfile = argv[optind];
}

static void
setMacros(char const * const *macros, size_t cnt)
{
  size_t	i;
  for (i=0; i<cnt; ++i)
    rpmDefineMacro(rpmGlobalMacroContext, macros[i], 0);
}

int main(int argc, char *argv[])
{
struct Arguments args = { 0,0,0,-1,-1, {0,0,0}, 0 };
Spec s;

	parseArgs(&args, argc, argv);

	if ((args.chroot && chroot(args.chroot)==-1) ||
		(args.uid!=(uid_t)(-1) && (setgroups(0,0)  ==-1 || getgroups(0,0)!=0))  ||
		(args.gid!=(gid_t)(-1) && (setgid(args.gid)==-1 || getgid()!=args.gid)) ||
		(args.uid!=(uid_t)(-1) && (setuid(args.uid)==-1 || getuid()!=args.uid))) {

		perror("chroot/setuid/setgid()");
		return EXIT_FAILURE;
	}
  
	rpmReadConfigFiles(args.rcfile, args.target);
	setMacros(args.macros.values, args.macros.cnt);

	rpmts ts = rpmtsCreate();
	if (parseSpec(ts, args.specfile, NULL, 0, NULL, NULL, 0, 1, 1) != 0) {
		return EXIT_FAILURE;
	}
  
	s = rpmtsSpec(ts);
	Header h = s->sourceHeader;
	const char *name, *version, *release;

	initSourceHeader(s, NULL);

	if (
		headerGetEntryMinMemory(h, RPMTAG_NAME, NULL, (void *)&name, NULL) == 0 ||
		headerGetEntryMinMemory(h, RPMTAG_VERSION, NULL, (void *)&version, NULL) == 0 ||
		headerGetEntryMinMemory(h, RPMTAG_RELEASE, NULL, (void *)&release, NULL) == 0
		) {
		printf(stderr, "NVR query failed\n");
		return EXIT_FAILURE;
	}

	printf("h PACKAGE_NAME %s\n", name);
	printf("h PACKAGE_VERSION %s\n", version);
	printf("h PACKAGE_RELEASE %s\n", release);
	// XXX kill the ! hack
	const char *sourcedir = rpmExpand("%{_sourcedir}!"); *(strchr(sourcedir, '!')) = '\0';
	const char *patchdir = rpmExpand("%{_patchdir}!"); *(strchr(patchdir, '!')) = '\0';

	struct Source *ps = s->sources;
	while (ps) {
		const char *type = (ps->flags & RPMFILE_SOURCE) ? "SOURCE" : "PATCH";
		const char *sdir = (ps->flags & RPMFILE_SOURCE) ? sourcedir : patchdir;
		printf("s %s%d %s/%s\n", type, ps->num, sdir, ps->source);
		if (ps->source != ps->fullSource) {
			printf("s %sURL%d %s\n", type, ps->num, ps->fullSource);
		}
		if (ps->flags & RPMFILE_GHOST) {
			printf("s nosource %d\n", ps->num);
		}
		ps = ps->next;
	}

	return(0);
}
