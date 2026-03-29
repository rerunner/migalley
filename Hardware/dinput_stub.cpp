#include "WIN32_COMPAT.H"

#include "dinput_stub.h"

static const unsigned long SDL_JOY_GUID_MAGIC = 0x53444C4A; // 'SDLJ'

static bool IsJoystickGUID(const GUID& guid) {
    return guid.Data2 == (unsigned short)(SDL_JOY_GUID_MAGIC >> 16) &&
           guid.Data3 == (unsigned short)(SDL_JOY_GUID_MAGIC & 0xFFFF);
}

HRESULT IDirectInputA::QueryInterface(const IID& iid, void** ppv) {
    if (!ppv) return E_POINTER;
    *ppv = nullptr;

    if (memcmp(&iid, &IID_IDirectInputA, sizeof(IID)) == 0) {
        *ppv = this;
        AddRef();
        return DI_OK;
    }
    return E_NOINTERFACE;
}

ULONG IDirectInputA::AddRef() {
    return ++refCount;
}

ULONG IDirectInputA::Release() {
    ULONG r = --refCount;
    if (r == 0) delete this;
    return r;
}

HRESULT IDirectInputA::CreateDevice(const GUID& rguid,
                    LPDIRECTINPUTDEVICEA* lplpDevice,
                    LPUNKNOWN pUnkOuter)
{
    if (!lplpDevice) return DIERR_GENERIC;

    // Create the device object
    // Note: We ignore pUnkOuter for aggregation in this stub
    auto* dev = new IDirectInputDevice2A();
    *lplpDevice = dev;

    // Check for System Mouse
    if (memcmp(&rguid, &GUID_SysMouse, sizeof(GUID)) == 0) {
        dev->isMouse = true;
        return DI_OK;
    }
    // Check for System Keyboard
    else if (memcmp(&rguid, &GUID_SysKeyboard, sizeof(GUID)) == 0) {
        dev->isKeyboard = true;
        return DI_OK;
    }
    // Check for our SDL Joystick GUID pattern
    else if (IsJoystickGUID(rguid)) {
        dev->deviceIndex = (int)rguid.Data1;
        dev->sdlJoy = SDL_JoystickOpen(dev->deviceIndex);
        if (!dev->sdlJoy) {
            delete dev;
            *lplpDevice = nullptr;
            return DIERR_NOTFOUND;
        }
        return DI_OK;
    }
    // Fallback: If the game passes IID instead of GUID (buggy game code?), handle gracefully
    else if (memcmp(&rguid, &IID_IDirectInputDeviceA, sizeof(GUID)) == 0 ||
             memcmp(&rguid, &IID_IDirectInputDevice2A, sizeof(GUID)) == 0) {
        // Just return a dummy device, likely keyboard or mouse logic required by game
        return DI_OK;
    }
    
    delete dev;
    *lplpDevice = nullptr;
    return DIERR_NOTINITIALIZED;
}

HRESULT IDirectInputA::EnumDevices(DWORD dwDevType,
                    LPDIENUMDEVICESCALLBACK lpCallback,
                    LPVOID pvRef,
                    DWORD dwFlags)
{
    // same SDL-backed enumeration as before
    if (!lpCallback) return DIERR_GENERIC;

    if (dwDevType & DIDEVTYPE_MOUSE) {
        DIDEVICEINSTANCE inst{};
        inst.dwSize = sizeof(inst);
        inst.guidInstance = GUID_SysMouse;
        inst.guidProduct = GUID_SysMouse;
        inst.dwDevType = DIDEVTYPE_MOUSE;
        snprintf(inst.tszInstanceName, MAX_PATH, "SDL Mouse");
        snprintf(inst.tszProductName, MAX_PATH, "System Mouse");
        lpCallback(&inst, pvRef);
    }

    if (dwDevType & DIDEVTYPE_JOYSTICK) {
        int numJoy = SDL_NumJoysticks();
        for (int i = 0; i < numJoy; ++i) {
            DIDEVICEINSTANCE inst{};
            inst.dwSize = sizeof(inst);
            // Embed index in GUID: { Index, MAGIC_HI, MAGIC_LO, ... }
            inst.guidInstance.Data1 = (unsigned long)i;
            inst.guidInstance.Data2 = (unsigned short)(SDL_JOY_GUID_MAGIC >> 16);
            inst.guidInstance.Data3 = (unsigned short)(SDL_JOY_GUID_MAGIC & 0xFFFF);
            
            inst.dwDevType = DIDEVTYPE_JOYSTICK;
            snprintf(inst.tszInstanceName, MAX_PATH, "SDL Joystick %d", i);
            snprintf(inst.tszProductName, MAX_PATH, "%s", SDL_JoystickNameForIndex(i));
            lpCallback(&inst, pvRef);
        }
    }
    return DI_OK;
}

HRESULT IDirectInputA::GetDeviceStatus(const GUID& rguid) 
{
    // For now we only distinguish joystick vs mouse
    if (rguid.Data1 == DIDEVTYPE_MOUSE) {
        // SDL always exposes the system mouse
        return DI_OK;
    }
    else if (rguid.Data1 == DIDEVTYPE_JOYSTICK) 
    {
        // Check if any joystick is still attached
        int numJoy = SDL_NumJoysticks();
        if (numJoy > 0) {
            // Optionally: loop through all and check SDL_JoystickGetAttached
            for (int i = 0; i < numJoy; ++i) {
                SDL_Joystick* joy = SDL_JoystickOpen(i);
                if (joy) {
                    bool attached = SDL_JoystickGetAttached(joy);
                    SDL_JoystickClose(joy);
                    if (attached) return DI_OK;
                }
            }
            return DI_NOTATTACHED;
        } else {
            return DI_NOTATTACHED;
        }
    }
    // Unknown GUID → pretend not attached
    return DI_NOTATTACHED;
}

HRESULT IDirectInputA::RunControlPanel(HWND hwndOwner, DWORD dwFlags)
{
    return DI_OK;
}

HRESULT DirectInputCreate(HINSTANCE hinst,
                                 uint32_t dwVersion,
                                 LPDIRECTINPUT* ppDI,
                                 LPUNKNOWN punkOuter) {
    if (ppDI) {
        *ppDI = new IDirectInputA();
    }
    return DI_OK;
}
