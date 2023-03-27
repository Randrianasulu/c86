/* align.h - memory alignment requirements for linker */

#ifndef S_ALIGNMENT
# define align(x)
#else
# define align(x) ((x) = (void *)((((long) (x)) + (S_ALIGNMENT-1)) & ~(S_ALIGNMENT-1)))
				/* assumes sizeof(long) == sizeof(char *) */
#endif
