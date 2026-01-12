#include <stdlib.h>   // abs, labs, llabs
#include <math.h>     // fabs, fabsf, fabsl if needed
#include <cstdint>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "SDL.h"
#include "MigCstring.h"

// Minimal Windows/MFC compatibility shims for Linux/gcc
#ifndef WIN32_COMPAT_H
#define WIN32_COMPAT_H

/* ------------------------------------------------------------------
   Windows-style max/min macros
   ------------------------------------------------------------------ */
#ifndef maxstub
#define maxstub(a,b) (( (a) > (b) ) ? (a) : (b))
#endif

#ifndef ZeroMemory
#include <string.h>  // for memset
#define ZeroMemory(Destination,Length) memset((Destination), 0, (Length))
#endif

#ifndef Sleep
#include <sched.h>    // for sched_yield
static inline void Sleep(unsigned int dwMilliseconds)
{
    if (dwMilliseconds == 0) {
        // Yield the processor, like Windows Sleep(0)
        sched_yield();
    } else {
        // usleep takes microseconds
        usleep(dwMilliseconds * 1000);
    }
}
#endif

typedef long HRESULT;

#define FAR
#define __FAR
#define PASCAL

#ifndef WINAPI
#  if defined(__i386__) || defined(_M_IX86)
#    define WINAPI __attribute__((stdcall))
#  else
#    define WINAPI
#  endif
#endif

// Basic character and string types
typedef char CHAR;
typedef char* LPSTR;
typedef const CHAR* LPCSTR;
typedef LPCSTR PCSTR;
typedef const CHAR* LPCTSTR;
typedef wchar_t* LPWSTR;
typedef const wchar_t* LPCWSTR;
typedef const void* LPCVOID;/* Long pointer to constant void */

typedef unsigned long ULONG;
typedef ULONG *PULONG;

typedef void VOID;
typedef void* LPVOID;
typedef const void* LPCVOID;

typedef uint32_t DWORD;
typedef DWORD* LPDWORD;

typedef uint16_t WORD;
typedef WORD* LPWORD;

typedef uint8_t BYTE;
typedef BYTE* LPBYTE;


#ifndef AFX_CDECL
#if defined(__GNUC__)
#define AFX_CDECL
#else
#define AFX_CDECL __cdecl
#endif
#endif

// TCHAR/LPCTSTR compatibility
#ifndef TCHAR
typedef char TCHAR;   // or wchar_t if you want to mimic UNICODE builds
#endif

#ifndef LPCTSTR
typedef const TCHAR* LPCTSTR;
#endif

// Some MSVC headers use the __cdecl keyword; on non-MSVC compilers make it a no-op
#ifndef __cdecl
#define __cdecl
#endif

// Calling convention macros
#ifndef __thiscall
#define __thiscall
#endif

// Inline assembly macros
#ifndef _asm
#define _asm asm
#endif

// Debug/int3
#ifndef INT3
#define INT3 ((void)0)
#endif

// Floating point control
#include "MATHASM.H"

// BOOL/TRUE/FALSE if not defined elsewhere
#ifndef BOOL
typedef int BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

// Simple stub for AfxTrace used in some headers — defined as inline to avoid link errors
inline void AfxTrace(const char*, ...) {}

#ifndef CALLBACK
#define CALLBACK
#endif

#ifndef PVOID
typedef void* PVOID;
#endif

#ifndef BOOLEAN
typedef unsigned char BOOLEAN;
#endif

#ifndef RECT_DEFINED
#define RECT_DEFINED
typedef long LONG;
typedef LONG* LPLONG;  // pointer to LONG

typedef struct tagRECT {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
} RECT;

typedef RECT* PRECT;
typedef const RECT* LPCRECT;
#endif

/// Windows GUI stuff
// SDL2 window replaces HWND
struct SDL_Window;
using HWND = SDL_Window*;
typedef void *HANDLE;

static inline BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase)
{
    // Linux stub: no-op
    (void)hWnd;
    (void)lpRect;
    (void)bErase;
    return TRUE;
}

#ifndef IDOK
#define IDOK        1
#endif
#ifndef IDCANCEL
#define IDCANCEL    2
#endif
#ifndef IDABORT
#define IDABORT     3
#endif
#ifndef IDRETRY
#define IDRETRY     4
#endif
#ifndef IDIGNORE
#define IDIGNORE    5
#endif
#ifndef IDYES
#define IDYES       6
#endif
#ifndef IDNO
#define IDNO        7
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFFu
#endif


// guid stub
#ifndef GUID_DEFINED
#define GUID_DEFINED
// Match Windows GUID layout
typedef struct _GUID {
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
} GUID;

typedef GUID* LPGUID;
typedef const GUID* LPCGUID;

// Optional: define IID/CLSID aliases if your code uses them
typedef GUID IID;
typedef GUID CLSID;

/* Reference to a GUID (by const pointer) */
//typedef const GUID* REFGUID;
//typedef const GUID* REFIID;
//typedef const GUID* REFCLSID;
typedef const GUID& REFGUID;
typedef const GUID& REFIID;
typedef const GUID& REFCLSID;


/* Common aliases */
typedef GUID IID;
typedef GUID CLSID;

/* ------------------------------------------------------------------
   Comparison helpers (mimic Windows InlineIsEqualGUID)
   ------------------------------------------------------------------ */

#ifdef __cplusplus
#include <algorithm>  /* for std::equal */

inline bool operator==(const GUID& a, const GUID& b) {
    return a.Data1 == b.Data1 &&
           a.Data2 == b.Data2 &&
           a.Data3 == b.Data3 &&
           std::equal(std::begin(a.Data4), std::end(a.Data4), std::begin(b.Data4));
}

inline bool operator!=(const GUID& a, const GUID& b) {
    return !(a == b);
}
#endif /* __cplusplus */

/* C‑style helper macro, like Windows IsEqualGUID */
#define IsEqualGUID(g1,g2) \
    ((g1).Data1 == (g2).Data1 && \
     (g1).Data2 == (g2).Data2 && \
     (g1).Data3 == (g2).Data3 && \
     (g1).Data4[0] == (g2).Data4[0] && \
     (g1).Data4[1] == (g2).Data4[1] && \
     (g1).Data4[2] == (g2).Data4[2] && \
     (g1).Data4[3] == (g2).Data4[3] && \
     (g1).Data4[4] == (g2).Data4[4] && \
     (g1).Data4[5] == (g2).Data4[5] && \
     (g1).Data4[6] == (g2).Data4[6] && \
     (g1).Data4[7] == (g2).Data4[7])

#endif // GUID_DEFINED


// WIN32_COMPAT.H
#ifndef MB_OK
#define MB_OK 0x00000000L
#endif
#ifndef MB_OKCANCEL
#define MB_OKCANCEL 0x00000001L 
#endif

#if 0
inline int MessageBox(void* /*hWnd*/, const char* text, const char* caption, unsigned /*type*/) {
    // For now, just print to stderr or stdout
    fprintf(stderr, "[MessageBox: %s] %s\n", caption ? caption : "", text ? text : "");
    return 0; // IDOK
}
#else
inline int MessageBox(void* /*hWnd*/, const CString& text, const CString& caption, unsigned /*type*/)
{
    // For now, just print to stderr
    std::fprintf(stderr, "[MessageBox: %s] %s\n",
                 caption.c_str(),   // CString::c_str() returns const char*
                 text.c_str());

    return IDOK; // Always return OK for MB_OK
}
#endif

// Simple OutputDebugString stub
#define OutputDebugString(msg) fprintf(stderr, "%s", msg)

/* COM interface declaration stubs for GCC/Linux */

#ifndef DECLARE_INTERFACE_
# define DECLARE_INTERFACE_(iface, base) struct iface : public base
#endif

#ifndef STDMETHOD
# define STDMETHOD(method) virtual HRESULT method
#endif

#ifndef STDMETHOD_
# define STDMETHOD_(type,method) virtual type method
#endif

#ifndef PURE
# define PURE = 0
#endif

/* In C++ mode, THIS and THIS_ are empty */
#ifndef THIS
# define THIS
#endif

#ifndef THIS_
# define THIS_
#endif

/* ------------------------------------------------------------------
   Windows memory copy compatibility
   ------------------------------------------------------------------ */
#ifndef CopyMemory
#define CopyMemory(Destination, Source, Length) \
    memcpy((Destination), (Source), (Length))
#endif

/* ------------------------------------------------------------------
   Win32 file I/O compatibility
   ------------------------------------------------------------------ */

/* Desired access */
#ifndef GENERIC_READ
#define GENERIC_READ    0x80000000
#endif
#ifndef GENERIC_WRITE
#define GENERIC_WRITE   0x40000000
#endif

/* Share modes */
#ifndef FILE_SHARE_READ
#define FILE_SHARE_READ 0x00000001
#endif
#ifndef FILE_SHARE_WRITE
#define FILE_SHARE_WRITE 0x00000002
#endif

/* Creation disposition */
#ifndef CREATE_NEW
#define CREATE_NEW      1
#endif
#ifndef CREATE_ALWAYS
#define CREATE_ALWAYS   2
#endif
#ifndef OPEN_EXISTING
#define OPEN_EXISTING   3
#endif
#ifndef OPEN_ALWAYS
#define OPEN_ALWAYS     4
#endif
#ifndef TRUNCATE_EXISTING
#define TRUNCATE_EXISTING 5
#endif

/* Invalid handle */
#ifndef INVALID_HANDLE_VALUE
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#endif

/* ------------------------------------------------------------------
   CreateFile replacement
   ------------------------------------------------------------------ */
static inline HANDLE CreateFileA(LPCSTR lpFileName,
                                 DWORD dwDesiredAccess,
                                 DWORD dwShareMode,
                                 void* lpSecurityAttributes,
                                 DWORD dwCreationDisposition,
                                 DWORD dwFlagsAndAttributes,
                                 HANDLE hTemplateFile)
{
    (void)dwShareMode; (void)lpSecurityAttributes;
    (void)dwFlagsAndAttributes; (void)hTemplateFile;

    int flags = 0;
    if ((dwDesiredAccess & GENERIC_READ) && (dwDesiredAccess & GENERIC_WRITE))
        flags = O_RDWR;
    else if (dwDesiredAccess & GENERIC_WRITE)
        flags = O_WRONLY;
    else
        flags = O_RDONLY;

    switch (dwCreationDisposition) {
        case CREATE_NEW:        flags |= O_CREAT | O_EXCL; break;
        case CREATE_ALWAYS:     flags |= O_CREAT | O_TRUNC; break;
        case OPEN_EXISTING:     /* no O_CREAT */ break;
        case OPEN_ALWAYS:       flags |= O_CREAT; break;
        case TRUNCATE_EXISTING: flags |= O_TRUNC; break;
    }

    int fd = open(lpFileName, flags, 0666);
    if (fd == -1) return INVALID_HANDLE_VALUE;
    return (HANDLE)(intptr_t)fd;
}
#define CreateFile CreateFileA

/* ------------------------------------------------------------------
   GetFileSize replacement
   ------------------------------------------------------------------ */
static inline DWORD GetFileSize(HANDLE hFile, DWORD* lpFileSizeHigh)
{
    off_t size = lseek((int)(intptr_t)hFile, 0, SEEK_END);
    lseek((int)(intptr_t)hFile, 0, SEEK_SET); // reset position
    if (size == (off_t)-1) return (DWORD)-1;

    if (lpFileSizeHigh) {
        uint64_t u = (uint64_t)size;
        *lpFileSizeHigh = (DWORD)(u >> 32);
    }
    return (DWORD)(size & 0xFFFFFFFF);
}

/* ------------------------------------------------------------------
   ReadFile replacement
   ------------------------------------------------------------------ */
static inline BOOL ReadFile(HANDLE hFile,
                            LPVOID lpBuffer,
                            DWORD nNumberOfBytesToRead,
                            DWORD* lpNumberOfBytesRead,
                            void* lpOverlapped)
{
    (void)lpOverlapped;
    ssize_t ret = read((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToRead);
    if (ret < 0) return 0; // failure
    if (lpNumberOfBytesRead) *lpNumberOfBytesRead = (DWORD)ret;
    return 1; // success
}

/* ------------------------------------------------------------------
   WriteFile replacement
   ------------------------------------------------------------------ */
static inline BOOL WriteFile(HANDLE hFile,
                             LPCVOID lpBuffer,
                             DWORD nNumberOfBytesToWrite,
                             DWORD* lpNumberOfBytesWritten,
                             void* lpOverlapped)
{
    (void)lpOverlapped;
    ssize_t ret = write((int)(intptr_t)hFile, lpBuffer, nNumberOfBytesToWrite);
    if (ret < 0) return 0; // failure
    if (lpNumberOfBytesWritten) *lpNumberOfBytesWritten = (DWORD)ret;
    return 1; // success
}

static inline BOOL SetEndOfFile(HANDLE hFile)
{
    // unwrap HANDLE back to int fd
    int fd = (int)(intptr_t)hFile;

    // get current file pointer
    off_t pos = lseek(fd, 0, SEEK_CUR);
    if (pos == (off_t)-1)
        return 0;  // failure

    // truncate/extend file to current position
    return (ftruncate(fd, pos) == 0);
}

#ifndef FILE_BEGIN
#define FILE_BEGIN   SEEK_SET
#endif
#ifndef FILE_CURRENT
#define FILE_CURRENT SEEK_CUR
#endif
#ifndef FILE_END
#define FILE_END     SEEK_END
#endif

#ifndef INVALID_SET_FILE_POINTER
#define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif

static inline DWORD SetFilePointer(HANDLE hFile,
                                   LONG lDistanceToMove,
                                   LONG* lpDistanceToMoveHigh,
                                   DWORD dwMoveMethod)
{
    int fd = (int)(intptr_t)hFile;

    // Always do math in 64 bits
    int64_t offset = (int64_t)lDistanceToMove;
    if (lpDistanceToMoveHigh) {
        offset |= ((int64_t)(*lpDistanceToMoveHigh) << 32);
    }

    off_t pos = lseek(fd, (off_t)offset, (int)dwMoveMethod);
    if (pos == (off_t)-1)
        return INVALID_SET_FILE_POINTER;

    if (lpDistanceToMoveHigh) {
        int64_t pos64 = (int64_t)pos;
        *lpDistanceToMoveHigh = (LONG)(pos64 >> 32);
    }

    return (DWORD)(pos & 0xFFFFFFFF);
}


#ifndef FILE_ATTRIBUTE_TEMPORARY
#define FILE_ATTRIBUTE_TEMPORARY 0x00000100
#endif

#ifndef GetLastError
static inline DWORD GetLastError(void)
{
    return (DWORD)errno;
}
#endif

#ifndef FlushFileBuffers
static inline BOOL FlushFileBuffers(HANDLE hFile)
{
    int fd = (int)(intptr_t)hFile;
    return (fsync(fd) == 0);
}
#endif

#ifndef FILE_ATTRIBUTE_NORMAL
#define FILE_ATTRIBUTE_NORMAL 0x00000080
#endif

#ifndef FILE_FLAG_RANDOM_ACCESS
#define FILE_FLAG_RANDOM_ACCESS 0x10000000
#endif

#ifndef DeleteFile
static inline BOOL DeleteFile(const char* lpFileName)
{
    return (unlink(lpFileName) == 0);
}
#endif

#include <sys/sysinfo.h>

typedef struct {
    unsigned long dwLength;
    unsigned long dwMemoryLoad;
    unsigned long dwTotalPhys;
    unsigned long dwAvailPhys;
    unsigned long dwTotalPageFile;
    unsigned long dwAvailPageFile;
    unsigned long dwTotalVirtual;
    unsigned long dwAvailVirtual;
} MEMORYSTATUS;

static inline void GlobalMemoryStatus(MEMORYSTATUS* lpBuffer)
{
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        lpBuffer->dwTotalPhys = 100000000; //RERUN, not yet -> info.totalram;
        lpBuffer->dwAvailPhys = 100000000; //RERUN, not yet -> info.freeram;
        lpBuffer->dwTotalPageFile = info.totalswap;
        lpBuffer->dwAvailPageFile = info.freeswap;
        //RERUN lpBuffer->dwTotalVirtual = info.totalram + info.totalswap;
        //RERUN lpBuffer->dwAvailVirtual = info.freeram + info.freeswap;
        lpBuffer->dwTotalVirtual = 100000000; //RERUN
        lpBuffer->dwAvailVirtual = 100000000; //RERUN
        lpBuffer->dwMemoryLoad = (unsigned long)
            (100 - (lpBuffer->dwAvailPhys * 100 / lpBuffer->dwTotalPhys));
    } else {
        // fallback: zero everything
        memset(lpBuffer, 0, sizeof(*lpBuffer));
    }
}

#include <time.h>

typedef union {
    struct { uint32_t LowPart; int32_t HighPart; };
    int64_t QuadPart;
} LARGE_INTEGER;

static inline int QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0)
        return 0;
    int64_t ns = (int64_t)ts.tv_sec * 1000000000LL + ts.tv_nsec;
    lpPerformanceCount->QuadPart = ns;
    lpPerformanceCount->LowPart  = (uint32_t)(ns & 0xFFFFFFFF);
    lpPerformanceCount->HighPart = (int32_t)(ns >> 32);
    return 1; // TRUE
}

static inline int QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency)
{
    // CLOCK_MONOTONIC is defined in nanoseconds
    lpFrequency->QuadPart = 1000000000LL;
    lpFrequency->LowPart  = (uint32_t)(lpFrequency->QuadPart & 0xFFFFFFFF);
    lpFrequency->HighPart = (int32_t)(lpFrequency->QuadPart >> 32);
    return 1; // TRUE
}


typedef int32_t   LRESULT;
typedef DWORD*    PDWORD;
typedef WORD*     PWORD;
typedef GUID INTERFACEID;
typedef unsigned int UINT;   // 32-bit unsigned
typedef long         LPARAM; // 32-bit signed (with -m32)

typedef void* HMIDIOUT; // a handle to an open MIDI output device. 
// It’s used with functions like midiOutOpen, midiOutShortMsg, and 
// midiOutClose to send MIDI messages to hardware or software synthesizers.
typedef HMIDIOUT* LPHMIDIOUT;
typedef struct MIDIHDR {
    char*     lpData;
    uint32_t  dwBufferLength;
    uint32_t  dwBytesRecorded;
    uintptr_t dwUser;
    uint32_t  dwFlags;
    struct MIDIHDR* lpNext;
    uintptr_t reserved;
    uint32_t  dwOffset;
    uintptr_t dwReserved[8];
} MIDIHDR, *PMIDIHDR, *NPMIDIHDR, *LPMIDIHDR;

// Flag defines
#define MHDR_DONE     0x00000001
#define MHDR_PREPARED 0x00000002
#define MHDR_INQUEUE  0x00000004
#define MHDR_ISSTRM   0x00000008

typedef uint32_t MMRESULT;

#define MMSYSERR_NOERROR     0
#define MMSYSERR_INVALHANDLE 5
#define MMSYSERR_INVALPARAM  11
#define MIDIERR_UNPREPARED   64
#define MIDIERR_NOTREADY     67

// Stub functions
inline MMRESULT midiOutPrepareHeader(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh) {
    if (!pmh || cbmh < sizeof(MIDIHDR)) return MMSYSERR_INVALPARAM;
    pmh->dwFlags |= MHDR_PREPARED;
    return MMSYSERR_NOERROR;
}

inline MMRESULT midiOutLongMsg(HMIDIOUT hmo, LPMIDIHDR pmh, UINT cbmh) {
    if (!pmh || !(pmh->dwFlags & MHDR_PREPARED)) return MIDIERR_UNPREPARED;
    // In a real implementation, you’d send pmh->lpData of length pmh->dwBufferLength
    pmh->dwFlags |= MHDR_DONE;
    return MMSYSERR_NOERROR;
}

#ifndef IN
#define IN
#endif

#ifndef OUT
#define OUT
#endif

#ifndef OPTIONAL
#define OPTIONAL
#endif

#ifndef INOUT
#define INOUT
#endif

#define FAILED(hr) (((HRESULT)(hr)) < 0)
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)

// HDC: A handle (opaque pointer) to a device context.
#ifndef HDC
typedef struct HDC__ { int unused; } *HDC;
#endif

#ifndef _MBS_FUNCS_STUB
#define _MBS_FUNCS_STUB
// Return the next character (as unsigned int) from a multibyte string
inline unsigned int _mbsnextc(const unsigned char* p) {
    return static_cast<unsigned int>(*p);
}

// Advance pointer to the next character
inline const unsigned char* _mbsinc(const unsigned char* p) {
    return p + 1;
}
#endif

inline BOOL KillTimer(void* /*hWnd*/, unsigned int /*uIDEvent*/) {
    return TRUE; // pretend success
}

inline BOOL SetCursorPos(int X, int Y) {
    // SDL_WarpMouseGlobal moves the mouse cursor in global screen coordinates
    if (SDL_WarpMouseGlobal(X, Y) == 0) {
        return 1; // TRUE on success
    } else {
        return 0; // FALSE on failure
    }
}

//// Window Event replacement by Posix

#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT  1
#define WAIT_FAILED   -1   /* or 0xFFFFFFFF */

/// Semaphore and Mutex stubs
#include <pthread.h>
#include <semaphore.h>

// Internal wrapper types
struct LinuxSemaphore {
    sem_t sem;
    int   maxCount;   // optional: track maximum count
};

struct PosixEventObject {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    bool            signaled;
    bool            manualReset;
};
// Drop-in replacement for CreateEvent
inline HANDLE CreateEvent(void* lpEventAttributes, bool bManualReset, bool bInitialState, const char* lpName)
{
    PosixEventObject* ev = new PosixEventObject;
    pthread_mutex_init(&ev->mutex, nullptr);
    pthread_cond_init(&ev->cond, nullptr);
    ev->signaled = bInitialState;
    ev->manualReset = bManualReset;
    return (HANDLE)ev;
}

// Drop-in replacement for SetEvent
inline bool SetEvent(HANDLE hEvent) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = true;
    if (ev->manualReset)
        pthread_cond_broadcast(&ev->cond);
    else
        pthread_cond_signal(&ev->cond);
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

// Drop-in replacement for ResetEvent
inline bool ResetEvent(HANDLE hEvent) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_lock(&ev->mutex);
    ev->signaled = false;
    pthread_mutex_unlock(&ev->mutex);
    return true;
}

static inline bool CloseHandle(HANDLE hEvent) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;
    pthread_mutex_destroy(&ev->mutex);
    pthread_cond_destroy(&ev->cond);
    delete ev;
    return true;
}
//static inline BOOL CloseHandle(HANDLE hObject)
//{
//    return (close((int)(intptr_t)hObject) == 0);
//}

// Drop-in replacement for WaitForSingleObject
inline int WaitForSingleObject(HANDLE hEvent, int timeout) {
    PosixEventObject* ev = (PosixEventObject*)hEvent;

    if (timeout == INFINITE) {
        int ret = pthread_mutex_lock(&ev->mutex);
        return (ret == 0) ? WAIT_OBJECT_0 : WAIT_FAILED;  // Map success/failure
    } else {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) != 0)
            return WAIT_FAILED;

        /* add the millisecond interval */
        const long long NSEC_PER_MSEC = 1000000LL;
        const long long NSEC_PER_SEC  = 1000000000LL;

        long long nsec = (long long)timeout * NSEC_PER_MSEC;
        ts.tv_sec  += nsec / NSEC_PER_SEC;
        ts.tv_nsec += nsec % NSEC_PER_SEC;

        /* carry over if nanoseconds overflow */
        if (ts.tv_nsec >= NSEC_PER_SEC) {
            ts.tv_nsec -= NSEC_PER_SEC;
            ++ts.tv_sec;
        }

        int rc = pthread_mutex_timedlock(&ev->mutex, &ts);
        if (rc == 0)          return WAIT_OBJECT_0;
        if (rc == ETIMEDOUT)  return WAIT_TIMEOUT;
        return WAIT_FAILED;   // any other error (e.g. EINVAL, EDEADLK, …)
    }
}

inline int WaitForSingleObjectSemaphore(HANDLE hEventSem, int timeout) {
    LinuxSemaphore* lsem = (LinuxSemaphore*)hEventSem;
    if (timeout == INFINITE) {
        return sem_wait(&lsem->sem) == 0 ? WAIT_OBJECT_0 : WAIT_FAILED;
    } else {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout / 1000;
        ts.tv_nsec += (timeout % 1000) * 1000000;
        if (sem_timedwait(&lsem->sem, &ts) == 0)
            return WAIT_OBJECT_0;
        else if (errno == ETIMEDOUT)
            return WAIT_TIMEOUT;
        else
            return WAIT_FAILED;
    }
}


// Drop-in replacement for CreateSemaphore
inline HANDLE CreateSemaphore(void* /*lpSemaphoreAttributes*/,
                              int lInitialCount,
                              int lMaximumCount,
                              const char* /*lpName*/) {
    LinuxSemaphore* s = new LinuxSemaphore;
    if (sem_init(&s->sem, 0, lInitialCount) != 0) {
        delete s;
        return nullptr;
    }
    // Note: POSIX semaphores don’t enforce max count, you’ll need to track it manually if required
    return s;
}

// Drop-in replacement for CreateMutex
inline HANDLE CreateMutex(void* /*lpMutexAttributes*/,
                          bool /*bInitialOwner*/,
                          const char* /*lpName*/) {
    PosixEventObject* m = new PosixEventObject;
    if (pthread_mutex_init(&m->mutex, nullptr) != 0) {
        delete m;
        return nullptr;
    }
    return m;
}

// ReleaseSemaphore: increments the semaphore count
inline bool ReleaseSemaphore(HANDLE hSemaphore, int lReleaseCount, int* lpPreviousCount) {
    LinuxSemaphore* s = (LinuxSemaphore*)hSemaphore;
    if (!s) return false;

    // Optionally track previous count
    if (lpPreviousCount) {
        int val;
        sem_getvalue(&s->sem, &val);
        *lpPreviousCount = val;
    }

    // Increment by lReleaseCount
    for (int i = 0; i < lReleaseCount; i++) {
        if (sem_post(&s->sem) != 0) {
            printf("ReleaseSemaphore returns false!\n");
            return false;
        }
    }
    return true;
}

// ReleaseMutex: unlocks the mutex
inline bool ReleaseMutex(HANDLE hMutex) {
    PosixEventObject* m = (PosixEventObject*)hMutex;
    if (!m) return false;
    return pthread_mutex_unlock(&m->mutex) == 0;
}

////////////////////////////
// Windows thread code stubs
typedef DWORD (*LPTHREAD_START_ROUTINE)(LPVOID);

// Internal wrapper
struct LinuxThread {
    pthread_t thread;
};

// Drop-in replacement for CreateThread
inline HANDLE CreateThread(void* /*lpThreadAttributes*/,
                           size_t /*dwStackSize*/,
                           LPTHREAD_START_ROUTINE lpStartAddress,
                           LPVOID lpParameter,
                           DWORD /*dwCreationFlags*/,
                           DWORD* /*lpThreadId*/) {
    LinuxThread* t = new LinuxThread;

    // Wrap the Windows-style function pointer into a pthread entry
    auto trampoline = [](void* arg) -> void* {
        auto pair = static_cast<std::pair<LPTHREAD_START_ROUTINE, LPVOID>*>(arg);
        LPTHREAD_START_ROUTINE func = pair->first;
        LPVOID param = pair->second;
        func(param); // Windows expects DWORD return, we ignore it here
        delete pair;
        return nullptr;
    };

    auto* pair = new std::pair<LPTHREAD_START_ROUTINE, LPVOID>(lpStartAddress, lpParameter);
    if (pthread_create(&t->thread, nullptr, trampoline, pair) != 0) {
        delete t;
        return nullptr;
    }
    return t;
}

// Drop-in replacement for SetThreadPriority
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_BELOW_NORMAL -1

inline int SetThreadPriority(HANDLE hThread, int /*nPriority*/) {
    // On Linux without root, changing priority has no effect.
    // We stub it out as success.
    return 1; // nonzero = success
}

// Optional cleanup
#if 0 
inline void CloseHandle(HANDLE h) {
    if (!h) return;
    LinuxThread* t = (LinuxThread*)h;
    pthread_detach(t->thread); // or pthread_join if you want to wait
    delete t;
}
#endif

// Stub of the MIDIOUTCAPS structure
// Windows defines MAXPNAMELEN as 32
#ifndef MAXPNAMELEN
#define MAXPNAMELEN 32
#endif

// MIDI technology types (from WinMM)
#define MOD_MIDIPORT   1   // Hardware port
#define MOD_SYNTH      2   // Internal synthesizer
#define MOD_SQSYNTH    3   // Square wave synthesizer
#define MOD_FMSYNTH    4   // FM synthesizer
#define MOD_MAPPER     5   // MIDI mapper

typedef struct {
    uint16_t wMid;                     // Manufacturer ID
    uint16_t wPid;                     // Product ID
    uint32_t vDriverVersion;           // Driver version
    char     szPname[MAXPNAMELEN];     // Product name
    uint16_t wTechnology;              // Device type
    uint16_t wVoices;                  // Number of voices
    uint16_t wNotes;                   // Max simultaneous notes
    uint16_t wChannelMask;             // Channel mask
    uint32_t dwSupport;                // Supported features
} MIDIOUTCAPS;

// Stub function that would normally fill this struct
inline int midiOutGetDevCaps(int deviceID, MIDIOUTCAPS* caps, unsigned int size) {
    if (!caps || size < sizeof(MIDIOUTCAPS)) return -1;
    caps->wMid = 0;
    caps->wPid = 0;
    caps->vDriverVersion = 0;
    strncpy(caps->szPname, "SDL2 Software Synth", sizeof(caps->szPname));
    caps->wTechnology = MOD_SYNTH;
    caps->wVoices = 64;
    caps->wNotes = 64;
    caps->wChannelMask = 0xFFFF;
    caps->dwSupport = 0;
    return 0; // success
}

typedef struct {
    uint16_t wFormatTag;        // PCM = 1
    uint16_t nChannels;         // 1 = mono, 2 = stereo
    uint32_t nSamplesPerSec;    // sample rate (e.g. 44100)
    uint32_t nAvgBytesPerSec;   // sample rate * block align
    uint16_t nBlockAlign;       // channels * bits/8
} WAVEFORMAT, *LPWAVEFORMAT;

typedef struct {
    WAVEFORMAT wf;              // nested WAVEFORMAT
    uint16_t   wBitsPerSample;  // bits per sample (8, 16, etc.)
} PCMWAVEFORMAT;

#define WAVE_MAPPER   (-1)

// Posix version of the CEvent class.
class CEvent {
public:
    // Constructor: matches MFC style
    CEvent(bool manualReset = false, bool initialState = false,
           const char* lpName = nullptr, void* lpEventAttributes = nullptr) {
        hEvent = CreateEvent(lpEventAttributes, manualReset, initialState, lpName);
    }

    ~CEvent() {
        // Clean up underlying object
        PosixEventObject* ev = (PosixEventObject*)hEvent;
        pthread_mutex_destroy(&ev->mutex);
        pthread_cond_destroy(&ev->cond);
        delete ev;
    }

    // Delegate to global SetEvent
    void SetEvent() {
        ::SetEvent(hEvent);
    }

    // Delegate to global ResetEvent
    void ResetEvent() {
        ::ResetEvent(hEvent);
    }

    // Delegate to global WaitForSingleObject
    int WaitForSingleObject(int timeout = INFINITE) {
        return ::WaitForSingleObject(hEvent, timeout);
    }

    // MFC-style Lock() wrapper
    bool Lock(int timeout = INFINITE) {
        return (::WaitForSingleObject(hEvent, timeout) == WAIT_OBJECT_0);
    }

    // Allow implicit conversion to HANDLE (like MFC)
    operator HANDLE() const { return hEvent; }

private:
    HANDLE hEvent;
};

// WinMM MIDI constants
#ifndef MIDI_MAPPER
#define MIDI_MAPPER ((unsigned int)-1)
#endif

// Stub for midiOutGetNumDevs
inline unsigned int midiOutGetNumDevs(void) {
    // Pretend one MIDI device exists (SDL2_mixer backend)
    return 1;
}

#endif // WIN32_COMPAT_H
