#if defined (EPOC) || defined(WIN32)
/*
**	Dummy <unistd.h> header file
*/

#define mode_t	int

#include <sys/types.h>

#define	SEEK_SET	0
#define	SEEK_CUR	1
#define	SEEK_END	2

#define	STDIN_FILENO	0
#define	STDOUT_FILENO	1
#define	STDERR_FILENO	2

int	close (int);
mode_t	umask (mode_t);
int	chmod (const char*, mode_t);
int	write (int, const void*, unsigned);
int	open (const char*, int, ...);
int	creat (const char*, mode_t);
off_t	lseek (int, off_t, int);
int	read (int, void*, unsigned);
#endif /* EPOC */
