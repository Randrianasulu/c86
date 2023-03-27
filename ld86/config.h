/* config.h - configuration for linker */

/* one of these target processors must be defined */

#define  I8086			/* Intel 8086 */
#undef I80386			/* Intel 80386 */
#undef  MC6809			/* Motorola 6809 */

/* one of these target operating systems must be defined */

#undef  EDOS			/* generate EDOS executable */
#undef MINIX			/* generate Minix executable */
/* #define EPOC			/* generate EPOC executable */

/* these may need to be defined to suit the source processor */

#define  S_ALIGNMENT 1		/* source memory alignment, power of 2 */
				/* don't use for 8 bit processors */
				/* don't use even for 80386 - overhead for */
				/* alignment cancels improved access */

/* these should be defined if they are supported by the source compiler */

#define  PROTO			/* compiler handles prototypes */

/* these must be defined to suit the source libraries */

#define CREAT_PERMS 0666	/* permissions for creat() */
#define EXEC_PERMS  0111	/* extra permissions to set for executable */
