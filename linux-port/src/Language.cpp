
#include "Language.h"
#include "Structs.h"

Language::Language()
{
    lngCode = "en_US";
}

/**
 * Load given language file
 *
 * @param lngCode
 * @return true on successful otherwise false
 */
bool Language::load(string lngCode)
{
    this->lngCode = lngCode;

    translations.clear();

    string fileName = "data/" + lngCode + ".csv";
    FILE *f = fopen(fileName.c_str(), "rt");
    if (!f) {
        return false;
    }

    wchar_t buffer[MAX_LANG_LENGTH];
    while (fgetws(buffer, MAX_LANG_LENGTH, f) != NULL) {
        wstring line = buffer;
        if (line.length() < wstring(L"\"\",\"\"").length()) {
            continue;
        }

        size_t commaPos = line.find(L"\",\"", 0);
        if (commaPos == wstring::npos) {
            continue;
        }

        commaPos += 1; // skip "

        wstring keyText = line.substr(0, commaPos);
        // remove " "
        keyText = keyText.substr(1);
        keyText = keyText.substr(0, keyText.length() - 1);
        if (!keyText.empty()) {
            if (line[line.length() - 1] == L'\n') {
                // remove \n on end of line
                line = line.substr(0, line.length() - 1);
            }
            // check next block with comments
            size_t commaPos2 = line.find(L"\",\"", commaPos + 3);
            if (commaPos2 != wstring::npos) {
                commaPos2 += 1; // skip "
                commaPos2 = commaPos2 - (keyText.length() + 3);
            }

            wstring valueText = line.substr(commaPos + 1, commaPos2);
            valueText = valueText.substr(1);
            valueText = valueText.substr(0, valueText.length() - 1);

            translations[keyText] = valueText;
        }
    }

    fclose(f);

    return true;
}

wchar_t Language::getYesKeyborad() const
{
    if (lngCode == "pl_PL") {
        return L'T'; // Tak
    }

    // en_US
    return L'Y'; // Yes
}

const wstring Language::cut(wstring text, int maxLen)
{
    if (text.length() <= maxLen) {
        return text;
    }

    text[maxLen - 1] = L'.';
    text[maxLen] = L'\0';
    return text;
}
