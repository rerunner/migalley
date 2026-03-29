// ddraw.h - Linux stub for DirectDraw (DX7)
// Non-functional placeholder for compilation only.

#pragma once
#include <vector>
#include <SDL.h>

#include "WIN32_COMPAT.H"

#pragma pack(push, 1)

typedef long HRESULT;
#define DD_OK            0
#define DDERR_GENERIC   -1

// DirectDraw color key flags
#define DDCKEY_SRCBLT       0x00000001
#define DDCKEY_DESTBLT      0x00000002
#define DDSD_CAPS               0x00000001
#define DDSD_HEIGHT             0x00000002
#define DDSD_WIDTH              0x00000004
#define DDSD_PITCH              0x00000008
#define DDSD_BACKBUFFERCOUNT    0x00000020
#define DDSD_ZBUFFERBITDEPTH    0x00000040
#define DDSD_ALPHABITDEPTH      0x00000080
#define DDSD_LPSURFACE          0x00000800
#define DDSD_PIXELFORMAT        0x00001000
#define DDSD_CKDESTOVERLAY      0x00002000
#define DDSD_CKDESTBLT          0x00004000
#define DDSD_CKSRCOVERLAY       0x00008000
#define DDSD_CKSRCBLT           0x00010000
#define DDSD_MIPMAPCOUNT        0x00020000
#define DDSD_REFRESHRATE        0x00040000
#define DDSD_LINEARSIZE         0x00080000
#define DDSD_DEPTH              0x00800000
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

// EnumDisplayModes flags
#define DDEDM_REFRESHRATES      0x00000001

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


// DirectDraw Enumeration Callback
typedef BOOL (PASCAL * LPDDENUMCALLBACKA)(GUID FAR *, LPSTR, LPSTR, LPVOID);
#define LPDDENUMCALLBACK LPDDENUMCALLBACKA

// DirectDraw Mode Enumeration Callback
typedef HRESULT (PASCAL * LPDDENUMMODESCALLBACK)(LPDDSURFACEDESC, LPVOID);

// IDirectDraw Interface Stub
struct IDirectDraw {
    // --- IUnknown Methods (Indices 0-2) ---
    virtual HRESULT QueryInterface(const IID& iid, void** ppv) {
        if (!ppv) return DDERR_GENERIC;
        *ppv = this; 
        AddRef();
        return DD_OK;
    }
    virtual ULONG AddRef() { return ++m_refCount; }
    virtual ULONG Release() { 
        if (--m_refCount == 0) {
            delete this; 
            return 0; 
        }
        return m_refCount;
    }

    // --- IDirectDraw Methods (Indices 3-23) ---
    virtual HRESULT Compact() { return DD_OK; }                                     // 3
    virtual HRESULT CreateClipper(DWORD, void**, void*) { return DD_OK; }           // 4
    virtual HRESULT CreatePalette(DWORD, void*, void**, void*) { return DD_OK; }    // 5
    
    virtual HRESULT CreateSurface(LPDDSURFACEDESC lpDDSurfaceDesc, LPDIRECTDRAWSURFACE* lplpDDSurface, IUnknown* pUnkOuter) {
        if (lplpDDSurface) *lplpDDSurface = nullptr; 
        return DD_OK; 
    }                                                                               // 6

    virtual HRESULT DuplicateSurface(void*, void**) { return DD_OK; }               // 7

    // Index 8
    virtual HRESULT EnumDisplayModes(DWORD dwFlags, LPDDSURFACEDESC lpSurfDesc, LPVOID lpContext, LPDDENUMMODESCALLBACK lpCallback) {
        if (!lpCallback) return DDERR_GENERIC;

        // Use Primary Display (index 0)
        const int displayIndex = 0;
        const int numModes = SDL_GetNumDisplayModes(displayIndex);
        if (numModes <= 0) return DD_OK; 

        // Legacy setup screens usually expect a clean list of unique resolutions per bit depth.
        // SDL returns a mode for every supported refresh rate, so we filter duplicates.
        struct ModeKey { int w, h, bpp; };
        std::vector<ModeKey> seen;

        for (int i = 0; i < numModes; ++i) {
            SDL_DisplayMode mode;
            if (SDL_GetDisplayMode(displayIndex, i, &mode) == 0) {
                int bpp = SDL_BITSPERPIXEL(mode.format);

                // Honor pixel format filter if provided in lpSurfDesc (e.g. searching for 16-bit modes)
                if (lpSurfDesc && (lpSurfDesc->dwFlags & DDSD_PIXELFORMAT)) {
                    if (lpSurfDesc->ddpfPixelFormat.dwRGBBitCount != 0 && 
                        lpSurfDesc->ddpfPixelFormat.dwRGBBitCount != (DWORD)bpp) {
                        continue;
                    }
                }

                // Filter out duplicate resolutions unless refresh rates are explicitly requested
                bool duplicate = false;
                for (const auto& mk : seen) {
                    if (mk.w == mode.w && mk.h == mode.h && mk.bpp == bpp) {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate) continue;
                seen.push_back({mode.w, mode.h, bpp});

                DDSURFACEDESC desc = {};
                desc.dwSize = sizeof(DDSURFACEDESC);
                desc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT | DDSD_CAPS;
                desc.dwWidth = mode.w;
                desc.dwHeight = mode.h;
                desc.ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
                
                desc.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
                desc.ddpfPixelFormat.dwFlags = DDPF_RGB;
                desc.ddpfPixelFormat.dwRGBBitCount = bpp;

                Uint32 Rmask, Gmask, Bmask, Amask;
                int bpp_masks;
                if (SDL_PixelFormatEnumToMasks(mode.format, &bpp_masks, &Rmask, &Gmask, &Bmask, &Amask)) {
                    desc.ddpfPixelFormat.dwRBitMask = Rmask;
                    desc.ddpfPixelFormat.dwGBitMask = Gmask;
                    desc.ddpfPixelFormat.dwBBitMask = Bmask;
                    desc.ddpfPixelFormat.dwRGBAlphaBitMask = Amask;
                }

                if (lpCallback(&desc, lpContext) == DDENUMRET_CANCEL) {
                    break;
                }
            }
        }
        return DD_OK;
    }

    virtual HRESULT EnumSurfaces(DWORD, void*, void*, void*) { return DD_OK; }      // 9
    virtual HRESULT FlipToGDISurface() { return DD_OK; }                            // 10
    virtual HRESULT GetCaps(void*, void*) { return DD_OK; }                         // 11
    virtual HRESULT GetDisplayMode(LPDDSURFACEDESC lpddsd) {                        // 12
        if (!lpddsd) return DDERR_GENERIC;
        SDL_DisplayMode mode;
        if (SDL_GetDesktopDisplayMode(0, &mode) != 0) return DDERR_GENERIC;

        lpddsd->dwWidth = mode.w;
        lpddsd->dwHeight = mode.h;
        lpddsd->ddpfPixelFormat.dwRGBBitCount = SDL_BITSPERPIXEL(mode.format);

        Uint32 Rmask, Gmask, Bmask, Amask;
        int bpp;
        if (SDL_PixelFormatEnumToMasks(mode.format, &bpp, &Rmask, &Gmask, &Bmask, &Amask)) {
            lpddsd->ddpfPixelFormat.dwRBitMask = Rmask;
            lpddsd->ddpfPixelFormat.dwGBitMask = Gmask;
            lpddsd->ddpfPixelFormat.dwBBitMask = Bmask;
            lpddsd->ddpfPixelFormat.dwRGBAlphaBitMask = Amask;
        }
        return DD_OK;
    }
    virtual HRESULT GetFourCCCodes(DWORD*, DWORD*) { return DD_OK; }                // 13
    virtual HRESULT GetGDISurface(void**) { return DD_OK; }                         // 14
    virtual HRESULT GetMonitorFrequency(void*) { return DD_OK; }                    // 15
    virtual HRESULT GetScanLine(void*) { return DD_OK; }                            // 16
    virtual HRESULT GetVerticalBlankStatus(void*) { return DD_OK; }                 // 17
    virtual HRESULT Initialize(void*) { return DD_OK; }                             // 18
    
    virtual HRESULT RestoreDisplayMode() {                                          // 19
        SDL_Window* win = SDL_GetWindowFromID(1);
        if (win) {
            SDL_SetWindowSize(win, 1024, 768);
            SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
        return DD_OK;
    }

    virtual HRESULT SetCooperativeLevel(HWND hWnd, DWORD dwFlags) {                 // 20
        return DD_OK;
    }

    virtual HRESULT SetDisplayMode(DWORD dwWidth, DWORD dwHeight, DWORD dwBPP, DWORD dwRefresh, DWORD dwFlags) { // 21 (v2 signature)
        SDL_Window* win = SDL_GetGrabbedWindow();
        if (!win) win = SDL_GL_GetCurrentWindow();
        if (!win) win = SDL_GetWindowFromID(1); 
        if (win) {
            SDL_SetWindowSize(win, (int)dwWidth, (int)dwHeight);
            SDL_SetWindowPosition(win, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }
        return DD_OK;
    }

    virtual HRESULT WaitForVerticalBlank(DWORD, void*) { return DD_OK; }            // 22
    virtual HRESULT GetAvailableVidMem(void*, DWORD*, DWORD*) { return DD_OK; }     // 23

    // Data members must come AFTER virtual functions to keep VTable at offset 0
    ULONG m_refCount;
    IDirectDraw() : m_refCount(1) {}
};

#pragma pack(pop)

typedef struct IDirectDraw          *LPDIRECTDRAW;
typedef struct IDirectDraw          IDirectDraw2; // Alias for stub simplicity
typedef IDirectDraw2                *LPDIRECTDRAW2;

// GUIDs
static const GUID IID_IDirectDraw2 = { 0xB3A6F3E0, 0x2B43, 0x11CF, { 0xA2, 0xDE, 0x00, 0xAA, 0x00, 0xB9, 0x33, 0x56 } };

// Function stubs
static inline HRESULT DirectDrawCreate(const void* lpGuid, IDirectDraw** ppDD, void* pUnkOuter) {
    if (ppDD) *ppDD = new IDirectDraw();
    return DD_OK;
}

static inline HRESULT DirectDrawEnumerate(LPDDENUMCALLBACKA lpCallback, LPVOID lpContext) {
    if (!lpCallback) return DDERR_GENERIC;

    // Enumerate Primary Display Driver (GUID = NULL)
    // We use SDL to get a meaningful name if possible
    char desc[256] = "Primary Display Driver";
    char name[256] = "display";

    if (SDL_WasInit(SDL_INIT_VIDEO)) {
        const char* sdlName = SDL_GetDisplayName(0);
        if (sdlName) snprintf(desc, sizeof(desc), "SDL Primary: %s", sdlName);
        const char* drvName = SDL_GetCurrentVideoDriver();
        if (drvName) snprintf(name, sizeof(name), "%s", drvName);
    }

    // Invoke callback for the primary device
    lpCallback(NULL, desc, name, lpContext);

    return DD_OK;
}

typedef struct {
    DWORD dwSize;
    DWORD dwDDFX;
    DWORD dwROP;
    DWORD dwFillColor;
    DDCOLORKEY ddckDestColorkey;
    DDCOLORKEY ddckSrcColorkey;
} DDBLTFX, *LPDDBLTFX;
