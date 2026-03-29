// dsound.h - Linux stub for DirectSound (DX7)
// Non-functional placeholder for compilation only.

#pragma once

typedef long HRESULT;
#define DS_OK            0
#define DSERR_GENERIC   -1

typedef struct IDirectSound       IDirectSound;
typedef struct IDirectSoundBuffer IDirectSoundBuffer;

struct IDirectSound       { void* unused; };
struct IDirectSoundBuffer { void* unused; };

// Cooperative levels
#define DSSCL_NORMAL 0

// Function stubs
static inline HRESULT DirectSoundCreate(const void* lpGuid,
                                        IDirectSound** ppDS,
                                        void* pUnkOuter)
{
    (void)lpGuid; (void)ppDS; (void)pUnkOuter;
    return DSERR_GENERIC;
}

