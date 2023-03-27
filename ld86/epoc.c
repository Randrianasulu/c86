#ifdef EPOC_TARGET
#include "type.h"
#include "obj.h"
#ifdef EPOC
#include <plib.h>

void
checksum(char *name)
{
    VOID *pfcb;
    INT len, len1;
    UWORD crc;
    UINT size;
    LONG pos;
    LONG write_offset, read_offset;
    INT  stacksize;
    ImgHeader header;
    UBYTE buffer[256];

    p_open(&pfcb, name, P_FOPEN | P_FRANDOM | P_FUPDATE);
    p_read(pfcb, &header, sizeof header);
    for (crc = 0, size = header.CodeParas*16; size; size -=len) {
	len = sizeof buffer < size ? sizeof buffer : size;
	if ((len = p_read(pfcb, &buffer, len)) < 0)
	    break;
        for (len1 = len; len1--;) {
	    crc += buffer[len1];
	}
    }
    header.CodeCheckSum = crc;

    /*
     *  Save the current file position as we are going
     *  to need this to move the data up.  Also read
     *  the size of data (i.e. stack + magic) that we
     *  are going to move out - this is stored in the
     *  first two words.
     */
    write_offset = 0L;
    p_seek (pfcb,P_FCUR,&write_offset);
    p_read(pfcb, &stacksize, sizeof(stacksize));
    read_offset=write_offset+stacksize;

    /*
     *  Checksum the data area, and at the same
     *  time move it up to remove stack+magic
     */
    for (crc = 0, size = header.InitializedData - stacksize; size; size -=len) {
	len = sizeof buffer <size ? sizeof buffer : size;
	p_seek (pfcb, P_FABS, &read_offset);
	if ((len = p_read(pfcb, &buffer, len)) < 0)
	    break;
        for (len1 = len; len1--;) {
	    crc += buffer[len1];
	}
	p_seek (pfcb, P_FABS, &write_offset);
	p_write (pfcb, &buffer, len);
	write_offset += len;
	read_offset += len;
    }
    header.DataCheckSum = crc;
    header.StackParas = stacksize / 16;
    header.DataParas -= header.StackParas;
    header.InitializedData -= stacksize;
    pos = 0L;
    p_seek(pfcb, P_FABS, &pos);
    p_write(pfcb, &header, sizeof header);
    /*
     *  Do not forget to adjust length of file now that
     *  the magic statics and stack have been removed
     */
    p_iow (pfcb, P_FSETEOF, &write_offset);
    p_close(pfcb);
}
#else /* EPOC */
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif /* WIN32 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>

#ifndef O_BINARY
#define	O_BINARY	0
#endif /* O_BINARY */

#define	SignatureSize	16
#define	MaxAddFiles	4

typedef struct
    {
    unsigned short int offset;
    unsigned short int length;
    } ADDFILE;

typedef struct
    {
    unsigned char Signature[SignatureSize];
    unsigned short int ImageVersion;
    unsigned short int HeaderSizeBytes;
    unsigned short int CodeParas;
    unsigned short int InitialIP;
    unsigned short int StackParas;
    unsigned short int DataParas;
    unsigned short int HeapParas;
    unsigned short int InitializedData;
    unsigned short int CodeCheckSum;
    unsigned short int DataCheckSum;
    unsigned short int CodeVersion;
    unsigned short int Priority;
    ADDFILE Add[MaxAddFiles];
    unsigned short int DylCount;
    unsigned char DylTableOffset[4];
    unsigned short int Spare;
    } ImgHeader;

void
checksum(char *name)
{
    int fd;
    int len, len1;
    unsigned short crc;
    size_t size;
    off_t pos;
    off_t write_offset, read_offset;
    int  stacksize;
    ImgHeader header;
    unsigned char buffer[256];

    fd = open(name, O_RDWR | O_BINARY);
    read(fd, &header, sizeof header);
    for (crc = 0, size = cntooffset((char *)&header.CodeParas,sizeof(header.CodeParas))*16; size; size -=len) {
	len = sizeof buffer < size ? sizeof buffer : size;
	if ((len = read(fd, &buffer, len)) <= 0)
	    break;
        for (len1 = len; len1--;) {
	    crc += buffer[len1];
	}
    }
   offtocn((char *)&header.CodeCheckSum,crc,sizeof(header.CodeCheckSum));
    /*
     *  Save the current file position as we are going
     *  to need this to move the data up.  Also read
     *  the size of data (i.e. stack + magic) that we
     *  are going to move out - this is stored in the
     *  first two words.
     */
#if 0
    write_offset = 0L;
    write_offset = lseek (fd,write_offset,SEEK_CUR);
    read(fd, &stacksize, sizeof(stacksize));
    read_offset=write_offset+stacksize;
#else
    write_offset = cntooffset((char *)&header.CodeParas,sizeof(header.CodeParas))*16 + sizeof header;
    write_offset = lseek (fd,write_offset,SEEK_SET);
    read(fd, &stacksize, sizeof(stacksize));
    stacksize = cntooffset((char *)&stacksize,sizeof(stacksize));
    read_offset=write_offset+stacksize;
#endif

    /*
     *  Checksum the data area, and at the same
     *  time move it up to remove stack+magic
     */
    for (crc = 0, size = cntooffset((char *)&header.InitializedData,sizeof(header.InitializedData)) - stacksize; size; size -=len) {
	len = sizeof buffer <size ? sizeof buffer : size;
	lseek (fd, read_offset, SEEK_SET);
	if ((len = read(fd, &buffer, len)) <= 0)
	    break;
        for (len1 = len; len1--;) {
	    crc += buffer[len1];
	}
	lseek (fd, write_offset, SEEK_SET);
	write (fd, &buffer, len);
	write_offset += len;
	read_offset += len;
    }
    offtocn((char *)&header.DataCheckSum,crc, sizeof(header.DataCheckSum));
    offtocn((char *)&header.StackParas,stacksize / 16, sizeof (header.StackParas));
    offtocn((char *)&header.DataParas, \
            cntooffset((char *)&header.DataParas,sizeof(header.DataParas)) - stacksize / 16, \
            sizeof (header.DataParas));
    offtocn((char *)&header.InitializedData, \
            cntooffset((char *)&header.InitializedData,sizeof(header.InitializedData)) - stacksize, \
            sizeof (header.InitializedData));
    pos = 0L;
    lseek(fd, pos, SEEK_SET);
    write(fd, &header, sizeof header);
    /*
     *  Do not forget to adjust length of file now that
     *  the magic statics and stack have been removed
     */


#ifdef WIN32
	chsize(fd, write_offset);
#else
    ftruncate(fd, write_offset);
#endif
    close(fd);
}
#endif /* EPOC */
#endif /* TARGET_EPOC */
