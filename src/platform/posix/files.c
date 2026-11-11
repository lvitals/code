/* The POSIX files and folders: open, read, write (a new file renamed
 * over the old one), map a large one, list a folder; the settings folder;
 * the messages before the screen.  For the terminal and the window alike. */
#define _XOPEN_SOURCE 700
#if defined(__linux__)
#define _GNU_SOURCE		/* O_TMPFILE: a snapshot of a file mapped */
#endif
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#if defined(__linux__)
#include <sys/ioctl.h>
#ifndef FICLONE
#define FICLONE		_IOW(0x94, 9, int)
#endif
#endif
#include <unistd.h>
#include "../platform.h"

/* the nanoseconds of a file's time of change: POSIX.1-2008 has st_mtim
 * (Linux, FreeBSD, OpenBSD, NetBSD); macOS, in its POSIX mode, has
 * st_mtimensec */
#if defined(__APPLE__)
#define MTIME_NSEC(s)	((s).st_mtimensec)
#else
#define MTIME_NSEC(s)	((s).st_mtim.tv_nsec)
#endif

const int file_crlf = 0;
const char path_sep = '/';
const char plat_encoding[] = "UTF-8";

int
udec(char *s, pos_t n)
{
	return sprintf(s, "%lu", (unsigned long)n);
}

void
plat_print(const char *s)
{
	size_t n = strlen(s), i, o = 0;
	char b[1024];

	for(i = 0; i < n; i++)		/* the messages have CR LF: LF here */
		if(s[i] != '\r') {
			b[o++] = s[i];
			if(o == sizeof b) {
				(void)!write(1, b, o);
				o = 0;
			}
		}
	(void)!write(1, b, o);
}

#ifndef PATH_MAX
#define PATH_MAX	4096
#endif

/* the files mapped: a save does not write into one of them */
static struct map {
	char *p;
	pos_t size;
	dev_t dev;
	ino_t ino;
	/* its snapshot, being made: by pid, into snap, of the file orig
	   as it was then (its size, its time) */
	pid_t pid;
	int snap, orig;
	off_t osize;
	time_t osec;
	long onsec;
} *maps;
static int nmaps;

/* the file being written: a new one, renamed over dst when it is closed */
static struct {
	int fd, bad;
	char tmp[PATH_MAX + 16], dst[PATH_MAX];
} wr = { -1, 0, "", "" };

static volatile sig_atomic_t lost;
static long pagesize;
static int zerofd = -1;

/* a mapped page past the end of its file (cut by another program): zeros
 * in its place, and the access goes on */
static void
onbus(int sig, siginfo_t *si, void *ctx)
{
	char *a = si->si_addr, *page;
	int i;

	(void)ctx;
	for(i = 0; i < nmaps; i++)
		if(a >= maps[i].p && a < maps[i].p + maps[i].size) {
			page = (char *)((uintptr_t)a & ~(uintptr_t)(pagesize - 1));
			if(zerofd >= 0 && mmap(page, (size_t)pagesize, PROT_READ, MAP_PRIVATE | MAP_FIXED,
			    zerofd, 0) != MAP_FAILED) {
				lost = 1;
				return;
			}
		}
	signal(sig, SIG_DFL);	/* not ours: as it would be */
	raise(sig);
}

int
file_lost(void)
{
	int r = lost;

	lost = 0;
	return r;
}

pos_t
file_stamp(const char *name)
{
	struct stat s;
	unsigned long long h = 1469598103934665603ULL, v[5];
	int i;

	if(stat(name, &s) < 0)
		return 0;
	v[0] = (unsigned long long)s.st_dev;
	v[1] = (unsigned long long)s.st_ino;
	v[2] = (unsigned long long)s.st_size;
	v[3] = (unsigned long long)s.st_mtime;
	v[4] = (unsigned long long)MTIME_NSEC(s);
	for(i = 0; i < 5; i++)
		h = (h ^ v[i]) * 1099511628211ULL;
	return (pos_t)h | 1;
}

static int
is_mapped(const struct stat *s)
{
	int i;

	for(i = 0; i < nmaps; i++)
		if(maps[i].dev == s->st_dev && maps[i].ino == s->st_ino)
			return 1;
	return 0;
}

int
file_open(const char *name, int w)
{
	struct stat s;
	mode_t mask;
	int fd, exists;

	if(w == FILE_SCAN) {	/* a regular file, or none: never waits */
		if((fd = open(name, O_RDONLY | O_NONBLOCK)) < 0)
			return -1;
		if(fstat(fd, &s) < 0 || !S_ISREG(s.st_mode)) {
			close(fd);
			return -1;
		}
		return fd;
	}
	if(!w)
		return open(name, O_RDONLY);
	if(wr.fd >= 0)
		return -1;
	/* the file behind the links: it is the one replaced */
	if(realpath(name, wr.dst) == 0) {
		if(strlen(name) >= sizeof wr.dst)
			return -1;
		strcpy(wr.dst, name);	/* a new file */
	}
	exists = stat(wr.dst, &s) == 0;
	if(exists && !S_ISREG(s.st_mode))
		return open(name, O_WRONLY | O_TRUNC);	/* a device, a pipe */
	snprintf(wr.tmp, sizeof wr.tmp, "%s.XXXXXX", wr.dst);
	fd = mkstemp(wr.tmp);
	if(fd < 0) {
		/* the folder takes no new file: in place, if not mapped */
		if(exists && is_mapped(&s))
			return -1;
		return open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	}
	if(exists) {
		fchmod(fd, s.st_mode & 07777);
		if(fchown(fd, s.st_uid, s.st_gid) < 0)
			fchmod(fd, s.st_mode & 0777);	/* not set-id for another */
	} else {
		mask = umask(0);
		umask(mask);
		fchmod(fd, 0666 & ~mask);
	}
	wr.fd = fd;
	wr.bad = 0;
	return fd;
}

int
file_read(int h, char *buf, int n)
{
	int r;

	while((r = (int)read(h, buf, n)) < 0 && errno == EINTR)
		;
	return r;
}

int
file_write(int h, const char *buf, int n)
{
	int o = 0, r;

	while(o < n) {
		r = (int)write(h, buf + o, n - o);
		if(r < 0 && errno == EINTR)
			continue;
		if(r <= 0) {
			if(h == wr.fd)
				wr.bad = 1;
			return -1;
		}
		o += r;
	}
	return n;
}

int
file_close(int h)
{
	int r;

	if(h != wr.fd)
		return close(h);
	wr.fd = -1;
	r = wr.bad || fsync(h) < 0 ? -1 : 0;
	if(close(h) < 0)
		r = -1;
	if(r == 0 && rename(wr.tmp, wr.dst) < 0)
		r = -1;
	if(r < 0)
		unlink(wr.tmp);
	return r;
}

pos_t
file_size(int h)
{
	struct stat s;

	if(fstat(h, &s) < 0 || !S_ISREG(s.st_mode) || s.st_size < 0 ||
	   (unsigned long long)s.st_size != (unsigned long long)(pos_t)s.st_size ||
	   (unsigned long long)s.st_size != (unsigned long long)(size_t)s.st_size)
		return POS_NONE;
	return (pos_t)s.st_size;
}

/* A snapshot of a file mapped, sharing its blocks (a reflink: btrfs, XFS,
 * bcachefs...), in a file with no name in its folder, is mapped in its
 * place: what another program then writes into the file does not reach
 * the text (the editor asks to read it again).  Making one waits for the
 * file's data to be on the disk (seconds, after another program wrote a
 * large file): a child makes it, while the editor goes on with the file
 * itself; snapshot_done puts it in when it is made (the same bytes, at the
 * same place).  Where no reflink can be made the file itself stays. */
static void
snapshot_start(struct map *m, int h)
{
	m->pid = 0;
	m->snap = m->orig = -1;
#if defined(__linux__) && defined(O_TMPFILE)
	{
		char link[64], path[4096], *slash;
		struct stat s;
		ssize_t n;
		pid_t pid;
		int t, o;

		sprintf(link, "/proc/self/fd/%d", h);
		if((n = readlink(link, path, sizeof path - 1)) <= 0)
			return;
		path[n] = 0;
		for(slash = 0; n > 0; n--)	/* the last / */
			if(path[n - 1] == '/') {
				slash = path + n - 1;
				break;
			}
		if(slash == 0)
			return;
		slash[slash == path] = 0;	/* its folder ("/" stays) */
		if((t = open(path, O_TMPFILE | O_RDWR, 0600)) < 0)
			return;
		if((o = dup(h)) < 0 || fstat(o, &s) < 0) {
			if(o >= 0)
				close(o);
			close(t);
			return;
		}
		if((pid = fork()) < 0) {
			close(o);
			close(t);
			return;
		}
		if(pid == 0)	/* the child: the reflink, and no more */
			_exit(ioctl(t, FICLONE, o) < 0);
		m->pid = pid;
		m->snap = t;
		m->orig = o;
		m->osize = s.st_size;
		m->osec = s.st_mtim.tv_sec;
		m->onsec = s.st_mtim.tv_nsec;
	}
#else
	(void)h;
#endif
}

static void
snapshot_drop(struct map *m)
{
	int st;

	if(m->pid > 0) {
		kill(m->pid, SIGKILL);
		while(waitpid(m->pid, &st, 0) < 0 && errno == EINTR)
			;
	}
	if(m->snap >= 0)
		close(m->snap);
	if(m->orig >= 0)
		close(m->orig);
	m->pid = 0;
	m->snap = m->orig = -1;
}

/* a snapshot made: mapped where the file was, if the file is still as it
 * was (else the file stays: the editor tells of the change) */
static void
snapshot_done(struct map *m)
{
	struct stat s;
	int st, ok;
	void *q;

	if(m->pid <= 0 || waitpid(m->pid, &st, WNOHANG) != m->pid)
		return;
	m->pid = 0;
	ok = WIFEXITED(st) && WEXITSTATUS(st) == 0 && fstat(m->orig, &s) == 0 &&
	    s.st_size == m->osize && s.st_mtim.tv_sec == m->osec &&
	    s.st_mtim.tv_nsec == m->onsec;
	if(ok) {
		q = mmap(m->p, (size_t)m->size, PROT_READ, MAP_PRIVATE | MAP_FIXED, m->snap, 0);
		if(q == MAP_FAILED)	/* the file again, where it was */
			mmap(m->p, (size_t)m->size, PROT_READ, MAP_PRIVATE | MAP_FIXED, m->orig, 0);
	}
	snapshot_drop(m);
}

/* now and then: the snapshots made put in */
void
file_idle(void)
{
	int i;

	for(i = 0; i < nmaps; i++)
		snapshot_done(&maps[i]);
}

char *
file_map(int h, pos_t size, int snapshot)
{
	struct stat s;
	struct map *m;
	void *p;

	struct sigaction sa;

	if(size == 0 || fstat(h, &s) < 0)
		return 0;
	if(zerofd < 0) {	/* the first: the pages lost read as zeros */
		pagesize = sysconf(_SC_PAGESIZE);
		zerofd = open("/dev/zero", O_RDONLY);
		memset(&sa, 0, sizeof sa);
		sa.sa_sigaction = onbus;
		sa.sa_flags = SA_SIGINFO | SA_NODEFER;
		sigemptyset(&sa.sa_mask);
		sigaction(SIGBUS, &sa, 0);
	}
	m = realloc(maps, (nmaps + 1) * sizeof *maps);
	if(m == 0)
		return 0;
	maps = m;
	/* the file itself, at once; its snapshot in its place when made */
	p = mmap(0, (size_t)size, PROT_READ, MAP_PRIVATE, h, 0);
	if(p == MAP_FAILED)
		return 0;
	m[nmaps].p = p;
	m[nmaps].size = size;
	m[nmaps].dev = s.st_dev;
	m[nmaps].ino = s.st_ino;
	m[nmaps].pid = 0;
	m[nmaps].snap = m[nmaps].orig = -1;
	if(snapshot)
		snapshot_start(&m[nmaps], h);
	nmaps++;
	return p;
}

void
file_unmap(char *p, pos_t size)
{
	int i;

	munmap(p, (size_t)size);
	for(i = 0; i < nmaps; i++)
		if(maps[i].p == p) {
			snapshot_drop(&maps[i]);
			maps[i] = maps[--nmaps];
			break;
		}
}

int
file_kind(const char *name)
{
	struct stat s;

	if(stat(name, &s) < 0)
		return 0;
	return S_ISDIR(s.st_mode) ? 2 : 1;
}

static DIR *dir;
static char dirpath[4096];

int
dir_open(const char *path)
{
	strncpy(dirpath, path, sizeof dirpath - 1);
	dir = opendir(path);
	return dir != 0;
}

int
dir_next(char *name, int max, int *isdir)
{
	struct dirent *e;
	char full[4096 + 256];

	if(dir == 0 || (e = readdir(dir)) == 0)
		return 0;
	strncpy(name, e->d_name, max - 2);
	name[max - 2] = 0;
	snprintf(full, sizeof full, "%s/%s", dirpath, e->d_name);
	*isdir = file_kind(full) == 2;
	return 1;
}

int
dir_entry(char *name, int max)
{
	struct dirent *e;
	struct stat sb;
	char full[4096 + 256];
	int kind = -1;

	if(dir == 0 || (e = readdir(dir)) == 0)
		return DK_END;
	strncpy(name, e->d_name, max - 1);
	name[max - 1] = 0;
#ifdef DT_DIR	/* the kind from the folder itself: no stat */
	if(e->d_type == DT_REG)
		kind = DK_FILE;
	else if(e->d_type == DT_DIR)
		kind = DK_DIR;
	else if(e->d_type != DT_UNKNOWN && e->d_type != DT_LNK)
		kind = DK_OTHER;
#endif
	if(kind < 0) {
		snprintf(full, sizeof full, "%s/%s", dirpath, e->d_name);
		if(lstat(full, &sb) != 0)
			kind = DK_OTHER;
		else if(S_ISLNK(sb.st_mode))	/* a file through it; a folder not */
			kind = stat(full, &sb) == 0 && S_ISREG(sb.st_mode) ? DK_FILE : DK_OTHER;
		else
			kind = S_ISDIR(sb.st_mode) ? DK_DIR : S_ISREG(sb.st_mode) ? DK_FILE : DK_OTHER;
	}
	return kind;
}

void
dir_close(void)
{
	if(dir)
		closedir(dir);
	dir = 0;
}

int
plat_chdir(const char *path)
{
	return chdir(path) == 0;
}

const char *
dir_make(const char *path)
{
	return mkdir(path, 0777) == 0 ? 0 : strerror(errno);
}

/* --- the Trash ------------------------------------------------------------
 *
 * As the freedesktop.org Trash specification has it, so that the file
 * managers of GNOME, KDE, Xfce, ... show what was moved and restore it:
 *
 *   TRASH/files/NAME             what was moved (renamed: never copied)
 *   TRASH/info/NAME.trashinfo    where it was, and when:
 *
 *       [Trash Info]
 *       Path=/home/me/project/old%20notes
 *       DeletionDate=2026-11-11T15:30:00
 *
 * TRASH is the home's ($XDG_DATA_HOME/Trash, ~/.local/share/Trash) for
 * what is on the same file system; else the one at the top of the file
 * system of what is moved: $top/.Trash/$uid (when $top/.Trash is a
 * folder with the sticky bit, not a link), else $top/.Trash-$uid, and
 * Path is then relative to $top.  NAME is the name, or NAME.2, NAME.3,
 * ... when it is taken: its .trashinfo is made first (O_EXCL), so two
 * programs cannot take the same one.
 *
 * macOS has a Trash of its own, the Finder's: ~/.Trash for the home's
 * disk, $top/.Trashes/$uid for another; what is moved there keeps its name
 * ("name 2", "name 3", ... when it is taken), and has no .trashinfo. */

/* dir made, with its parents (0700): 0 if it cannot be */
static int
make_dirs(char *dir)
{
	struct stat s;
	char *p;

	if(stat(dir, &s) == 0)
		return S_ISDIR(s.st_mode);
	for(p = dir + 1; *p; p++)
		if(*p == '/') {
			*p = 0;
			if(mkdir(dir, 0700) < 0 && errno != EEXIST) {
				*p = '/';
				return 0;
			}
			*p = '/';
		}
	return mkdir(dir, 0700) == 0 || errno == EEXIST;
}

/* the top of the file system of path (on dev): the last folder going up
 * from it that is still on dev ("/" when the root is) */
static void
fs_top(const char *path, dev_t dev, char *top, size_t max)
{
	struct stat s;
	char *p;
	size_t n = strlen(path);

	if(n >= max) {
		strcpy(top, "/");
		return;
	}
	memcpy(top, path, n + 1);
	while((p = (strrchr)(top, '/')) != 0) {	/* the function: not glibc's C23 macro */
		if(p == top) {	/* its parent is the root */
			if(stat("/", &s) == 0 && s.st_dev == dev)
				strcpy(top, "/");
			return;
		}
		*p = 0;		/* the parent */
		if(stat(top, &s) < 0 || s.st_dev != dev) {
			*p = '/';	/* another file system: the one below it */
			return;
		}
	}
}

#if !defined(__APPLE__)
/* the trash of the top of a file system: its folder, or 0 */
static int
top_trash(const char *top, char *trash, size_t max)
{
	struct stat s;
	char t[4096];
	const char *sep = strcmp(top, "/") == 0 ? "" : "/";
	unsigned long uid = (unsigned long)getuid();

	if(snprintf(t, sizeof t, "%s%s.Trash", top, sep) >= (int)sizeof t)
		return 0;
	if(lstat(t, &s) == 0 && S_ISDIR(s.st_mode) && (s.st_mode & S_ISVTX) &&
	   snprintf(trash, max, "%s/%lu", t, uid) < (int)max && make_dirs(trash))
		return 1;
	return snprintf(trash, max, "%s%s.Trash-%lu", top, sep, uid) < (int)max &&
	    make_dirs(trash);
}

/* s, %-encoded as a URL's path (the bytes not plain, and not /) */
static int
url_path(const char *s, char *out, size_t max)
{
	static const char hex[] = "0123456789ABCDEF";
	size_t n = 0;
	unsigned char c;

	for(; (c = (unsigned char)*s) != 0; s++) {
		if(n + 4 > max)
			return 0;
		if((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
		   (strchr)("-._~/!$&'()*+,;=:@", c))
			out[n++] = (char)c;
		else {
			out[n++] = '%';
			out[n++] = hex[c >> 4];
			out[n++] = hex[c & 15];
		}
	}
	out[n] = 0;
	return 1;
}
#endif

#if defined(__APPLE__)
const char *
plat_trash(const char *path)
{
	char abs[4096], trash[4096], top[4096], dest[8300];
	const char *home = getenv("HOME"), *base;
	struct stat s, hs;
	int i;

	if(realpath(path, abs) == 0)
		return strerror(errno);
	if(strcmp(abs, "/") == 0)
		return "The root cannot be moved to the Trash";
	if(lstat(abs, &s) < 0)
		return strerror(errno);
	base = strrchr(abs, '/') + 1;
	trash[0] = 0;
	if(home && *home)
		snprintf(trash, sizeof trash, "%s/.Trash", home);
	if(!(trash[0] && make_dirs(trash) && stat(trash, &hs) == 0 && hs.st_dev == s.st_dev)) {
		fs_top(abs, s.st_dev, top, sizeof top);
		if(snprintf(trash, sizeof trash, "%s%s.Trashes/%lu", top,
		    strcmp(top, "/") == 0 ? "" : "/", (unsigned long)getuid()) >= (int)sizeof trash ||
		   !make_dirs(trash))
			return "There is no Trash on its disk";
	}
	if(strncmp(abs, trash, strlen(trash)) == 0 &&
	   (abs[strlen(trash)] == '/' || abs[strlen(trash)] == 0))
		return "It is in the Trash";
	for(i = 1; i < 10000; i++) {
		if(i == 1)
			snprintf(dest, sizeof dest, "%s/%s", trash, base);
		else
			snprintf(dest, sizeof dest, "%s/%s %d", trash, base, i);
		if(lstat(dest, &hs) < 0 && errno == ENOENT)
			break;
	}
	if(i == 10000)
		return "No name is free in the Trash";
	if(rename(abs, dest) < 0)
		return strerror(errno);
	return 0;
}
#else
const char *
plat_trash(const char *path)
{
	char abs[4096], trash[4096], dir[4200], info[5200], dest[5200], url[12288];
	char name[1024], when[32], text[12400];
	const char *home = getenv("HOME"), *data = getenv("XDG_DATA_HOME"), *base, *rel;
	struct stat s, hs;
	struct tm tm;
	time_t now;
	size_t n;
	int fd = -1, i, len;

	/* the path whole, without a / at its end */
	if(realpath(path, abs) == 0)
		return strerror(errno);
	if(strcmp(abs, "/") == 0)
		return "The root cannot be moved to the Trash";
	if(lstat(abs, &s) < 0)
		return strerror(errno);
	base = (strrchr)(abs, '/') + 1;
	if(strlen(base) + 8 >= sizeof name)
		return "The name is too long";

	/* the home's trash, on the same file system; else the top's */
	trash[0] = 0;
	if(data && *data == '/')
		snprintf(trash, sizeof trash, "%s/Trash", data);
	else if(home && *home)
		snprintf(trash, sizeof trash, "%s/.local/share/Trash", home);
	rel = abs;	/* Path=: whole, in the home's */
	if(trash[0] && make_dirs(trash) && stat(trash, &hs) == 0 && hs.st_dev == s.st_dev) {
		;
	} else {
		fs_top(abs, s.st_dev, dir, sizeof dir);
		if(!top_trash(dir, trash, sizeof trash))
			return "There is no Trash on its disk";
		n = strlen(dir);
		rel = abs + (strcmp(dir, "/") == 0 ? 1 : n + (abs[n] == '/'));
	}
	snprintf(dir, sizeof dir, "%s/files", trash);
	snprintf(info, sizeof info, "%s/info", trash);
	if(!make_dirs(dir) || !make_dirs(info))
		return "The Trash cannot be made";
	if(strncmp(abs, trash, strlen(trash)) == 0 &&
	   (abs[strlen(trash)] == '/' || abs[strlen(trash)] == 0))
		return "It is in the Trash";

	/* a name of its own in the Trash: its .trashinfo made first */
	for(i = 1; i < 10000; i++) {
		if(i == 1)
			snprintf(name, sizeof name, "%s", base);
		else
			snprintf(name, sizeof name, "%s.%d", base, i);
		if(snprintf(info, sizeof info, "%s/info/%s.trashinfo", trash, name) >= (int)sizeof info ||
		   snprintf(dest, sizeof dest, "%s/files/%s", trash, name) >= (int)sizeof dest)
			return "The name is too long";
		if(lstat(dest, &hs) == 0)
			continue;
		if((fd = open(info, O_WRONLY | O_CREAT | O_EXCL, 0600)) >= 0)
			break;
		if(errno != EEXIST)
			return strerror(errno);
	}
	if(fd < 0)
		return "No name is free in the Trash";
	now = time(0);
	localtime_r(&now, &tm);
	strftime(when, sizeof when, "%Y-%m-%dT%H:%M:%S", &tm);
	if(!url_path(rel, url, sizeof url)) {
		close(fd);
		unlink(info);
		return "The name is too long";
	}
	len = snprintf(text, sizeof text, "[Trash Info]\nPath=%s\nDeletionDate=%s\n", url, when);
	if(len <= 0 || len >= (int)sizeof text || write(fd, text, (size_t)len) != len ||
	   close(fd) < 0) {
		unlink(info);
		return "The Trash cannot be written";
	}
	if(rename(abs, dest) < 0) {
		const char *why = strerror(errno);

		unlink(info);
		return why;
	}
	return 0;
}
#endif

void
plat_cwd(char *buf, int max)
{
	size_t n;

	if(getcwd(buf, max - 1) == 0)
		strcpy(buf, "/");
	n = strlen(buf);
	if(n == 0 || buf[n - 1] != '/') {
		buf[n] = '/';
		buf[n + 1] = 0;
	}
}

/* $XDG_CONFIG_HOME/cedit/settings.json, or ~/.config/cedit/settings.json
 * (the same for cedit and xcedit); the file of an older name (edit) is
 * taken the first time */
int
plat_config(char *buf, int max)
{
	const char *x = getenv("XDG_CONFIG_HOME"), *home = getenv("HOME");
	char old[4096];
	int n, f, t, k;
	char b[4096];
	struct stat s;

	if(x && *x)
		n = snprintf(buf, max, "%s/cedit", x);
	else if(home && *home)
		n = snprintf(buf, max, "%s/.config/cedit", home);
	else
		return 0;
	if(n <= 0 || n + 15 >= max)
		return 0;
	buf[n - 6] = 0;		/* its parent first (~/.config) */
	mkdir(buf, 0700);
	snprintf(old, sizeof old, "%s/edit/settings.json", buf);
	buf[n - 6] = '/';
	if(mkdir(buf, 0700) < 0 && errno != EEXIST)
		return 0;
	strcat(buf, "/settings.json");
	if(stat(buf, &s) < 0 && (f = open(old, O_RDONLY)) >= 0) {
		if((t = open(buf, O_WRONLY | O_CREAT | O_EXCL, 0600)) >= 0) {
			while((k = (int)read(f, b, sizeof b)) > 0)
				if(write(t, b, (size_t)k) != k)
					break;
			close(t);
		}
		close(f);
	}
	return 1;
}

unsigned
plat_drives(void)
{
	return 0;
}
