/*
 * Linux File System Operation.
 *
 * Jerry 2011.12.21
 */
#ifndef FS_H
#define FS_H

#define FNAME_SIZE	128

typedef void		*_FILE_t;
typedef _FILE_t		FILE_t;

typedef struct f_date_time {
	unsigned short second;        /* seconds */
	unsigned short minute;        /* Minutes */
	unsigned short hour;          /* Hours */
	unsigned short day;           /* Days */
	unsigned short month;         /* Months */
	unsigned short year;          /* Year */
} TIME_t;

typedef struct f_info {
	char    spec[13];
	char    is_dir;         /* directory:1, regular file:0 */
	TIME_t  ftime;
	loff_t	size;
} FILE_INFO_t;


#define FSopen		Linux_FSopen
#define FSclose		Linux_FSclose
#define FSread		Linux_FSread
#define FSgetc		Linux_FSgetc
#define FSstat		Linux_FSstat

#endif
