/* A command of the shell, for Filter Through Command: sh -c cmd, with its
 * input from a buffer and its output and errors to others.  The three
 * pipes are served together (poll), so that a command that writes much
 * before it reads all its input cannot hold the editor; one that runs too
 * long is stopped. */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "../platform.h"

#define RUN_SECONDS	30			/* the longest a command runs */
#define RUN_MAX		((size_t)256 << 20)	/* the most it may write */

struct obuf {
	char *p;
	size_t n, cap;
};

/* what fd has, added to b: 0 at its end, -1 on an error (or too much) */
static int
take(int fd, struct obuf *b)
{
	char *q;
	ssize_t r;

	if(b->cap - b->n < 65536) {
		if(b->cap >= RUN_MAX)
			return -1;
		b->cap = b->cap ? b->cap * 2 : 65536;
		if((q = realloc(b->p, b->cap + 1)) == 0)
			return -1;
		b->p = q;
	}
	r = read(fd, b->p + b->n, b->cap - b->n);
	if(r < 0)
		return errno == EINTR || errno == EAGAIN ? 1 : -1;
	b->n += (size_t)r;
	return r > 0;
}

int
plat_run(const char *cmd, const char *dir, const char *in, size_t n,
    char **out, size_t *outn, char **err, size_t *errn)
{
	int pi[2] = { -1, -1 }, po[2] = { -1, -1 }, pe[2] = { -1, -1 };
	int status = -1, nf, k, r, failed = 0;
	struct obuf ob = { 0, 0, 0 }, eb = { 0, 0, 0 };
	struct pollfd pf[3];
	struct sigaction ign, old;
	size_t done = 0;
	time_t until;
	ssize_t w;
	pid_t pid;

	*out = *err = 0;
	*outn = *errn = 0;
	if(pipe(pi) < 0 || pipe(po) < 0 || pipe(pe) < 0)
		goto fail;
	if((pid = fork()) < 0)
		goto fail;
	if(pid == 0) {	/* the command: the pipes as its 0, 1, 2 */
		dup2(pi[0], 0);
		dup2(po[1], 1);
		dup2(pe[1], 2);
		close(pi[0]); close(pi[1]);
		close(po[0]); close(po[1]);
		close(pe[0]); close(pe[1]);
		if(dir && *dir && chdir(dir) < 0)
			_exit(126);
		execl("/bin/sh", "sh", "-c", cmd, (char *)0);
		_exit(127);
	}
	close(pi[0]);
	close(po[1]);
	close(pe[1]);
	pi[0] = po[1] = pe[1] = -1;
	/* a command that leaves its input unread: no SIGPIPE for the editor */
	memset(&ign, 0, sizeof ign);
	ign.sa_handler = SIG_IGN;
	sigemptyset(&ign.sa_mask);
	sigaction(SIGPIPE, &ign, &old);
	fcntl(pi[1], F_SETFL, fcntl(pi[1], F_GETFL) | O_NONBLOCK);
	if(n == 0) {
		close(pi[1]);
		pi[1] = -1;
	}
	until = time(0) + RUN_SECONDS;
	while(po[0] >= 0 || pe[0] >= 0) {
		nf = 0;
		if(po[0] >= 0) {
			pf[nf].fd = po[0];
			pf[nf++].events = POLLIN;
		}
		if(pe[0] >= 0) {
			pf[nf].fd = pe[0];
			pf[nf++].events = POLLIN;
		}
		if(pi[1] >= 0) {
			pf[nf].fd = pi[1];
			pf[nf++].events = POLLOUT;
		}
		if(time(0) > until) {
			kill(pid, SIGKILL);
			failed = 2;
			break;
		}
		if((r = poll(pf, (nfds_t)nf, 250)) < 0) {
			if(errno == EINTR)
				continue;
			kill(pid, SIGKILL);
			failed = 1;
			break;
		}
		for(k = 0; k < nf && r > 0; k++) {
			if(pf[k].revents == 0)
				continue;
			if(pf[k].fd == pi[1]) {
				w = write(pi[1], in + done, n - done > 65536 ? 65536 : n - done);
				if(w > 0)
					done += (size_t)w;
				if((w < 0 && errno != EAGAIN && errno != EINTR) || done == n) {
					close(pi[1]);	/* all of it, or it reads no more */
					pi[1] = -1;
				}
			} else {
				int *fd = pf[k].fd == po[0] ? &po[0] : &pe[0];
				int t = take(*fd, fd == &po[0] ? &ob : &eb);

				if(t <= 0) {
					if(t < 0) {
						kill(pid, SIGKILL);
						failed = 1;
					}
					close(*fd);
					*fd = -1;
				}
			}
		}
	}
	while(waitpid(pid, &status, 0) < 0 && errno == EINTR)
		;
	sigaction(SIGPIPE, &old, 0);
	if(pi[1] >= 0)
		close(pi[1]);
	if(po[0] >= 0)
		close(po[0]);
	if(pe[0] >= 0)
		close(pe[0]);
	if(failed) {
		free(ob.p);
		free(eb.p);
		return failed == 2 ? -2 : -1;
	}
	*out = ob.p;
	*outn = ob.n;
	*err = eb.p;
	*errn = eb.n;
	if(WIFEXITED(status))
		return WEXITSTATUS(status);
	return 128 + (WIFSIGNALED(status) ? WTERMSIG(status) : 0);
fail:
	for(k = 0; k < 2; k++) {
		if(pi[k] >= 0)
			close(pi[k]);
		if(po[k] >= 0)
			close(po[k]);
		if(pe[k] >= 0)
			close(pe[k]);
	}
	return -1;
}
