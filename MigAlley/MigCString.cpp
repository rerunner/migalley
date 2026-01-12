#include "GeneratedResources.h"
#include "CSTRING.H" //RERUN

int CString::LoadString(int resID) {
    auto it = g_resStrings.find(resID);
    if (it != g_resStrings.end()) {
        *this = it->second;
        return GetLength();
    }
    *this = "<missing resource>";
    return 0;
}
