#include <map>
#include <string>
#include "CSTRING.H" //RERUN

extern std::map<int, std::string> g_resStringsMap;

int CString::LoadString(int resID) {
    auto it = g_resStringsMap.find(resID);
    if (it != g_resStringsMap.end()) {
        *this = it->second;
        return GetLength();
    }
    *this = "<missing resource>";
    return 0;
}
