#ifdef EPOC
#include <plib.h>
#include "unistd.h"
#include "lib.h"

off_t
lseek (int fd, off_t offset, int whence)
{
    VOID * pfcb = _filetable[fd];
    LONG wanted = 0;
    LONG pos = offset;
    INT sense;
    switch (whence) {
      case SEEK_SET:	sense = P_FABS; break;
      case SEEK_CUR:	sense = P_FCUR; break;
      case SEEK_END:	sense = P_FEND; break;
		
    }
    /*
     *  Seek using correct mode, but with offset
     *  set to zero.
     */
    if (p_seek(pfcb, sense, &wanted))
	return ((off_t)-1);
    if (offset == 0L)
        return wanted;

    wanted += offset;
    /*
     *  Now try again, with desired offset
     */
    if (p_seek(pfcb, sense, &pos))
	return ((off_t)-1);
    /*
     *  If we failed, then need to extend file
     *  This could fail if not enough room.
     */
    if (pos < wanted) {
	if (p_iow(pfcb, P_FSETEOF, &wanted) < 0)
            return ((off_t)-1);
	if (p_seek(pfcb, sense, &wanted))
	    return ((off_t)-1);
    }
    
    return wanted;
}
#endif
