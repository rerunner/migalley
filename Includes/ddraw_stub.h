// ddraw.h - Linux stub for DirectDraw (DX7)
// Non-functional placeholder for compilation only.

#pragma once

#include "WIN32_COMPAT.H"

typedef long HRESULT;
#define DD_OK            0
#define DDERR_GENERIC   -1

// DirectDraw color key flags
#define DDCKEY_SRCBLT       0x00000001
#define DDCKEY_DESTBLT      0x00000002
#define DDCKEY_SRCOVERLAY   0x00000004
#define DDCKEY_DESTOVERLAY  0x00000008
#define DDGBS_ISBLTDONE 0x00000001
#define DDGBS_ISBLTING  0x00000002
#define DDBLT_WAIT 0x00000010

#define DD_ROP_SPACE 8

#define DDCAPS_BLT           0x00000001
#define DDCAPS_BLTQUEUE      0x00000002
#define DDCAPS_BLTFOURCC     0x00000004
#define DDCAPS_BANKSWITCHED  0x00000080
#define DDCAPS_COLORKEY      0x00000400
#define DDCAPS_OVERLAY       0x00000800
#define DDCAPS_ZBLTS         0x00002000

typedef struct DDCAPS {
    DWORD dwSize;        // size of this struct
    DWORD dwCaps;        // general capabilities flags
    DWORD dwCaps2;       // more capability flags
    DWORD dwCKeyCaps;    // color key capabilities
    DWORD dwFXCaps;      // effects capabilities
    DWORD dwFXAlphaCaps; // alpha blending caps
    DWORD dwPalCaps;     // palette capabilities
    DWORD dwAlphaBltConstBitDepths;
    DWORD dwAlphaBltPixelBitDepths;
    DWORD dwAlphaBltSurfaceBitDepths;
    DWORD dwAlphaOverlayConstBitDepths;
    DWORD dwAlphaOverlayPixelBitDepths;
    DWORD dwAlphaOverlaySurfaceBitDepths;
    DWORD dwZBufferBitDepths;
    DWORD dwVidMemTotal;
    DWORD dwVidMemFree;
    DWORD dwMaxVisibleOverlays;
    DWORD dwCurrVisibleOverlays;
    DWORD dwNumFourCCCodes;
    DWORD dwAlignBoundarySrc;
    DWORD dwAlignSizeSrc;
    DWORD dwAlignBoundaryDest;
    DWORD dwAlignSizeDest;
    DWORD dwAlignStrideAlign;
    DWORD dwRops[DD_ROP_SPACE]; // raster ops supported
    // … add more fields if the code references them
} DDCAPS, *LPDDCAPS;



typedef struct IDirectDraw       IDirectDraw;

struct IDirectDraw        { void* unused; };
// IDirectDrawSurface
struct IDirectDrawSurface {
    virtual ULONG   Release() = 0;
    virtual HRESULT SetColorKey(DWORD dwFlags, void* lpDDColorKey) = 0;
    // add other methods as needed
};
typedef IDirectDrawSurface* LPDIRECTDRAWSURFACE;

// IDirectDrawSurface2
struct IDirectDrawSurface2 {
    virtual ULONG   Release() = 0;
    virtual HRESULT SetColorKey(DWORD dwFlags, void* lpDDColorKey) = 0;
    virtual HRESULT Blt(void* lpDestRect,
                        IDirectDrawSurface2* lpSrcSurface,
                        void* lpSrcRect,
                        DWORD dwFlags,
                        void* lpDDBltFx) = 0;
    virtual HRESULT GetBltStatus(DWORD dwFlags) = 0;
    // add other methods as needed
};
typedef IDirectDrawSurface2* LPDIRECTDRAWSURFACE2;


// Cooperative levels
#define DDSCL_NORMAL 0

// Function stubs
static inline HRESULT DirectDrawCreate(const void* lpGuid,
                                       IDirectDraw** ppDD,
                                       void* pUnkOuter)
{
    (void)lpGuid; (void)ppDD; (void)pUnkOuter;
    return DDERR_GENERIC;
}

// Structure for Direct Draw Parameters							//ARM 25Jun96
//JIM 12/1/98
#ifdef	DECLARE_HANDLE
#undef	DECLARE_HANDLE
#endif
#ifdef STRICT
typedef void *HANDLE;
#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name
#else
#define DECLARE_HANDLE(name) typedef HANDLE name
#endif
typedef HANDLE *PHANDLE;

//RERUN DECLARE_HANDLE            (HWND);
																//ARM 25Jun96
struct _DDSURFACEDESC;											//ARM 25Jun96
typedef struct IDirectDraw			*LPDIRECTDRAW;				//ARM 25Jun96
typedef struct IDirectDraw2			*LPDIRECTDRAW2;				//ARM 25Nov96
typedef struct IDirectDrawSurface   *LPDIRECTDRAWSURFACE;		//ARM 25Jun96
typedef struct IDirectDrawPalette   *LPDIRECTDRAWPALETTE;		//ARM 25Jun96
typedef struct IDirectDrawClipper   *LPDIRECTDRAWCLIPPER;		//ARM 25Jun96
																//ARM 25Jun96
typedef struct _DDSURFACEDESC       *LPDDSURFACEDESC;			//ARM 25Jun96
typedef struct IDirect3D			*LPDIRECT3D;				//PD 03Feb97

// Forward declarations for legacy DirectDraw/Direct3D interfaces
typedef struct IDirectDrawSurface2 IDirectDrawSurface2;
typedef struct IDirect3DTexture    IDirect3DTexture;

// Stub pointer typedefs
typedef IDirectDrawSurface2* LPDIRECTDRAWSURFACE2;
typedef IDirect3DTexture*    LPDIRECT3DTEXTURE;

// Texture handle was just a DWORD
typedef DWORD D3DTEXTUREHANDLE;


// Dummy color key and pixel format structs
typedef struct {
    DWORD dwColorSpaceLowValue;
    DWORD dwColorSpaceHighValue;
} DDCOLORKEY;
typedef struct { DWORD dwSize; DWORD dwFlags; DWORD dwFourCC; DWORD dwRGBBitCount;
                 DWORD dwRBitMask; DWORD dwGBitMask; DWORD dwBBitMask; DWORD dwRGBAlphaBitMask; } DDPIXELFORMAT;
typedef struct _DDSCAPS {
    DWORD dwCaps;
} DDSCAPS;

// Capability flags
#define DDSCAPS_ALPHA            0x00000002
#define DDSCAPS_BACKBUFFER       0x00000004
#define DDSCAPS_COMPLEX          0x00000008
#define DDSCAPS_FLIP             0x00000010
#define DDSCAPS_FRONTBUFFER      0x00000020
#define DDSCAPS_OFFSCREENPLAIN   0x00000040
#define DDSCAPS_OVERLAY          0x00000080
#define DDSCAPS_PALETTE          0x00000100
#define DDSCAPS_PRIMARYSURFACE   0x00000200
#define DDSCAPS_SYSTEMMEMORY     0x00000400
#define DDSCAPS_TEXTURE          0x00000800
#define DDSCAPS_3DDEVICE         0x00001000
#define DDSCAPS_VIDEOMEMORY      0x00002000
#define DDSCAPS_VISIBLE          0x00004000
#define DDSCAPS_WRITEONLY        0x00008000
#define DDSCAPS_ZBUFFER          0x00010000
#define DDSCAPS_OWNDC            0x00020000
#define DDSCAPS_LIVEVIDEO        0x00080000
#define DDSCAPS_HWCODEC          0x00100000
#define DDSCAPS_MODEX            0x00200000
#define DDSCAPS_MIPMAP           0x00400000
#define DDSCAPS_ALLOCONLOAD      0x04000000
#define DDSCAPS_VIDEOPORT        0x08000000
#define DDSCAPS_LOCALVIDMEM      0x10000000
#define DDSCAPS_NONLOCALVIDMEM   0x20000000
#define DDSCAPS_STANDARDVGAMODE  0x40000000
#define DDSCAPS_OPTIMIZED        0x80000000


// The DDSURFACEDESC struct (simplified)
typedef struct _DDSURFACEDESC {
    DWORD         dwSize;
    DWORD         dwFlags;
    DWORD         dwHeight;
    DWORD         dwWidth;
    union {
        long  lPitch;
        DWORD dwLinearSize;
    };
    DWORD         dwBackBufferCount;
    DWORD         dwAlphaBitDepth;
    DWORD         dwReserved;
    LPVOID        lpSurface;
    DDCOLORKEY    ddckCKDestOverlay;
    DDCOLORKEY    ddckCKDestBlt;
    DDCOLORKEY    ddckCKSrcOverlay;
    DDCOLORKEY    ddckCKSrcBlt;
    DDPIXELFORMAT ddpfPixelFormat;
    DDSCAPS       ddsCaps;
} DDSURFACEDESC, *LPDDSURFACEDESC;


typedef struct {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwFillColor;
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX, *LPDDBLTFX;

// Windows-style typedefs and
// Palette entry flags for WinGDI compatibility
#define PC_RESERVED   0x01
#define PC_EXPLICIT   0x02
#define PC_NOCOLLAPSE 0x04
typedef struct {
    Uint8 peRed;
    Uint8 peGreen;
    Uint8 peBlue;
    Uint8 peFlags;   // not used by SDL, but kept for compatibility
} PALETTEENTRY, *LPPALETTEENTRY;

// Pixel format flags
#define DDPF_ALPHAPIXELS     0x00000001
#define DDPF_PALETTEINDEXED8 0x00000020
#define DDPF_RGB             0x00000040

// Enumeration return values
#define DDENUMRET_CANCEL 0
#define DDENUMRET_OK     1
