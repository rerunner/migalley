#pragma once
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <stdexcept>
#include <cstdarg>
#include <cstdio>
#include <cstring>

class CString : public std::string {
public:
    // Constructors
    CString() = default;
    CString(const CString& other) = default;
    CString(const std::string& s) : std::string(s) {}
    CString(const char* s) : std::string(s ? s : "") {}
    CString(char ch, int nRepeat = 1) : std::string(nRepeat, ch) {}
    CString(int i) { *this = std::to_string(i); }
    CString(long l) { *this = std::to_string(l); }
    CString(unsigned long ul) { *this = std::to_string(ul); }

    // RERUN Added for OVERLAY
    operator const char*() const { return this->c_str(); }
    //operator char*() { return this->data(); }

    // Assignment operators
    CString& operator=(const CString& rhs) = default;
    CString& operator=(const std::string& rhs) { std::string::operator=(rhs); return *this; }
    CString& operator=(const char* rhs) { std::string::operator=(rhs ? rhs : ""); return *this; }
    CString& operator=(char ch) { this->assign(1, ch); return *this; }
    CString& operator=(int i) { *this = std::to_string(i); return *this; }
    CString& operator=(long l) { *this = std::to_string(l); return *this; }
    CString& operator=(unsigned long ul) { *this = std::to_string(ul); return *this; }

    // Length / empty
    int GetLength() const { return static_cast<int>(this->size()); }
    bool IsEmpty() const { return this->empty(); }
    void Empty() { this->clear(); }

    // Character access
    char GetAt(int nIndex) const { return this->at(nIndex); }
    //void SetAt(int nIndex, char ch) { (*this)[nIndex] = ch; }
    void SetAt(int nIndex, char ch) { (*this)[static_cast<std::string::size_type>(nIndex)] = ch; }

    // Concatenation
    CString& operator+=(const CString& rhs) { append(rhs); return *this; }
    CString& operator+=(char ch) { push_back(ch); return *this; }
    CString& operator+=(const unsigned char* psz) { append(reinterpret_cast<const char*>(psz)); return *this; }

    // Comparison
    int Compare(const char* s) const { return this->compare(s); }
    int CompareNoCase(const char* s) const {
        std::string a = *this, b = s;
        std::transform(a.begin(), a.end(), a.begin(), ::tolower);
        std::transform(b.begin(), b.end(), b.begin(), ::tolower);
        return a.compare(b);
    }

    // Substrings
    CString Mid(int first, int count) const { return this->substr(first, count); }
    CString Mid(int first) const { return this->substr(first); }
    CString Left(int count) const { return this->substr(0, count); }
    CString Right(int count) const { return this->substr(this->size() - count); }

    // Case conversion
    void MakeUpper() { std::transform(begin(), end(), begin(), ::toupper); }
    void MakeLower() { std::transform(begin(), end(), begin(), ::tolower); }
    void MakeReverse() { std::reverse(begin(), end()); }

    // Trimming
    void TrimLeft() {
        erase(begin(), std::find_if(begin(), end(), [](unsigned char ch){ return !std::isspace(ch); }));
    }
    void TrimRight() {
        erase(std::find_if(rbegin(), rend(), [](unsigned char ch){ return !std::isspace(ch); }).base(), end());
    }
    void Trim() { TrimLeft(); TrimRight(); }

    // Replace / Remove
    int Replace(char oldCh, char newCh) {
        int count = 0;
        for (auto& c : *this) if (c == oldCh) { c = newCh; ++count; }
        return count;
    }
    int Replace(const char* oldStr, const char* newStr) {
        int count = 0;
        size_t pos = 0, oldLen = strlen(oldStr);
        while ((pos = this->find(oldStr, pos)) != npos) {
            this->replace(pos, oldLen, newStr);
            pos += strlen(newStr);
            ++count;
        }
        return count;
    }
    int Remove(char ch) {
        auto oldSize = size();
        erase(std::remove(begin(), end(), ch), end());
        return static_cast<int>(oldSize - size());
    }

    // Find
    int Find(char ch, int start = 0) const {
        auto pos = this->find(ch, start);
        return (pos == npos) ? -1 : static_cast<int>(pos);
    }
    int ReverseFind(char ch) const {
        auto pos = this->rfind(ch);
        return (pos == npos) ? -1 : static_cast<int>(pos);
    }
    int Find(const char* sub, int start = 0) const {
        auto pos = this->find(sub, start);
        return (pos == npos) ? -1 : static_cast<int>(pos);
    }

    // Format (printf-style)
    void Format(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        *this = buf;
    }

    // Buffer access (stubbed)
    char* GetBuffer(int nMinBufLength = -1) {
        reserve(nMinBufLength > 0 ? nMinBufLength : size());
        return data();
    }
    void ReleaseBuffer(int nNewLength = -1) {
        if (nNewLength >= 0 && nNewLength <= static_cast<int>(size()))
            resize(nNewLength);
        else if (nNewLength < 0)
            resize(strlen(c_str()));
    }

    // --- Extended helpers from your original header ---

    // Split into parts by delimiter
    CString Part(char cDelimiter, int nIndex) const {
        size_t start = 0, end;
        int idx = 0;
        while ((end = this->find(cDelimiter, start)) != npos) {
            if (idx == nIndex)
                return this->substr(start, end - start);
            start = end + 1;
            ++idx;
        }
        if (idx == nIndex)
            return this->substr(start);
        return CString(); // empty if not found
    }

    int PartCount(char cDelimiter) const {
        if (empty()) return 0;
        return static_cast<int>(std::count(begin(), end(), cDelimiter) + 1);
    }

    int PartBegin(char cDelimiter, int nIndex) const {
        size_t start = 0, end;
        int idx = 0;
        while ((end = this->find(cDelimiter, start)) != npos) {
            if (idx == nIndex)
                return static_cast<int>(start);
            start = end + 1;
            ++idx;
        }
        if (idx == nIndex)
            return static_cast<int>(start);
        return -1;
    }

    // Escape/unescape (basic URL-style %xx encoding)
    void Escape() {
        std::ostringstream oss;
        for (unsigned char c : *this) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
                oss << c;
            else {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", c);
                oss << buf;
            }
        }
        *this = oss.str();
    }

    void UnEscape() {
        std::string out;
        for (size_t i = 0; i < size(); ++i) {
            if ((*this)[i] == '%' && i + 2 < size()) {
                unsigned int val;
                sscanf(this->substr(i+1,2).c_str(), "%x", &val);
                out.push_back(static_cast<char>(val));
                i += 2;
            } else {
                out.push_back((*this)[i]);
            }
        }
        *this = out;
    }

    int LoadString(int resID);
};
