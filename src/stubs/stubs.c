#include <sys/types.h>
#include <sys/unistd.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>

extern _READ_WRITE_RETURN_TYPE  _read(int __fd, const void *__buf, size_t __nbyte) __attribute__((weak));
extern _READ_WRITE_RETURN_TYPE  _write (int __fd, const void *__buf, size_t __nbyte) __attribute__((weak));
extern int                      _close(int file) __attribute__((weak));
extern int                      _fstat(int file, struct stat *st) __attribute__((weak));
extern int                      _isatty(int file) __attribute__((weak));
extern off_t                    _lseek (int __fildes, off_t __offset, int __whence) __attribute__((weak));
extern void                     _exit (int __status)  _ATTRIBUTE ((__noreturn__)) __attribute__((weak));
extern int                      _kill (pid_t, int) __attribute__((weak));
extern pid_t                    _getpid (void) __attribute__((weak));

_READ_WRITE_RETURN_TYPE
_read (int __fd, const void *__buf, size_t __nbyte)
{
    return -1;
}

_READ_WRITE_RETURN_TYPE
_write (int __fd, const void *__buf, size_t __nbyte)
{
    return -1;
}

int
_close(int file)
{
    return -1;
}

int
_fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int
_isatty(int file)
{
    return 1;
}

off_t
_lseek (int __fildes, off_t __offset, int __whence)
{
    return 0;
}

void
_exit (int __status)
{
    while (1)
    {
    }
}

int
_kill (pid_t p, int s)
{
    return -1;
}

pid_t
_getpid (void)
{
    return (pid_t) -1;
}

/*-------------------------------------------------------------------------------------------------------------------------------------------
 * malloc-Familie als Stubs (N1, Flash-Analyse S3.1)
 *
 * Die Firmware ruft weder malloc/free/realloc/calloc noch asprintf/open_memstream/fopen. Gelinkt wurde
 * newlibs Allokator trotzdem, nur weil ihn drei Bibliotheksmodule REFERENZIEREN (Map F103):
 *   findfp.o  -> _malloc_r   (__sfmoreglue: weitere FILE-Objekte; hereingezogen ueber exit.o)
 *   fflush.o  -> _free_r
 *   nano-svfprintf.o -> _realloc_r  (__ssputs_r, nur auf dem __SMBF-Pfad von asprintf/open_memstream)
 * sprintf/snprintf arbeiten mit einem FILE auf dem Stack und erreichen keinen dieser Pfade.
 *
 * Verhaltensneutral: __HEAP_SIZE ist 0 (CMakeLists.txt), _sbrk() in sbrk.c liefert fuer jedes
 * Inkrement > 0 schon heute -1, malloc und realloc gaben also schon immer NULL zurueck, und free bekam
 * nie einen gueltigen Zeiger. errno setzen diese Stubs nicht; die Firmware liest errno nirgends.
 * Gespart: mallocr.o, freer.o, reallocr.o, msizer.o, sbrkr.o, mlock.o und _sbrk (rund 680 Byte).
 * Wer je dynamischen Speicher braucht, muss diese drei Funktionen UND __HEAP_SIZE anfassen.
 *
 * __attribute__((used, externally_visible)) ist noetig, nicht Zierde. Beim LTO-Link meldet der
 * Linker-Plugin nur die Referenzen, die er VOR der Optimierung kennt. findfp.o und fflush.o kommen
 * frueh herein (ueber exit.o), _malloc_r und _free_r gelten daher als gebraucht. nano-svfprintf.o
 * wird aber erst NACH LTO ueber snprintf aus dem LTO-Ergebnis gezogen -- _realloc_r galt bis dahin
 * als unbenutzt, LTO verwarf es, und der Link scheiterte mit "undefined reference to _realloc_r"
 * (Testbau N1, 11.10.2026). used haelt die Funktion fest, externally_visible verhindert, dass LTO sie
 * lokal macht. Die Stubs oben (_write usw.) brauchen das nicht: stdio.o/writer.o referenzieren sie
 * schon vor LTO; ihr weak an der Deklaration haelt nichts fest. Alle drei hier gleich behandelt.
 *-------------------------------------------------------------------------------------------------------------------------------------------
 */
#include <reent.h>

__attribute__((used, externally_visible)) void *
_malloc_r (struct _reent * r, size_t n)
{
    (void) r;
    (void) n;
    return (void *) 0;
}

__attribute__((used, externally_visible)) void
_free_r (struct _reent * r, void * p)
{
    (void) r;
    (void) p;
}

__attribute__((used, externally_visible)) void *
_realloc_r (struct _reent * r, void * p, size_t n)
{
    (void) r;
    (void) p;
    (void) n;
    return (void *) 0;
}
