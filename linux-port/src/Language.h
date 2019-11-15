
#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <map>
#include <string>

#define MAX_LANG_LENGTH 2048

using namespace std;

class Language {
public:
    Language();

    bool load(string lngCode);

    const wstring& get(const wstring& key) {
        map<wstring, wstring>::iterator it = translations.find(key);
        if (it == translations.end()) {
            return key; // not found, return original text
        }
        return it->second;
    }

    const string& getLngCode() const { return lngCode; }
    wchar_t getYesKeyborad() const;
    const wstring cut(wstring text, int maxLen);

private:
    // english text => destination language text
    map<wstring, wstring> translations;

    string lngCode;
};

#endif /* LANGUAGE_H */
