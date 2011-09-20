#include <config.h>

#include "unicode/normlzr.h"
#include "unicode/unistr.h"
#include "unicode/ustring.h"
#define CLD_WINDOWS
#include "googlecld/encodings/lang_enc.h"
#include "googlecld/languages/public/languages.h"
#include "googlecld/encodings/public/encodings.h"
#include "googlecld/encodings/compact_lang_det/win/cld_unicodetext.h"
#include "googlecld/encodings/compact_lang_det/compact_lang_det.h"
#include "LanguageTools.h"

using namespace std;

LanguageTools::LanguageTools() {
	//InitLangEnc();
	//InitEncodings();
}

LanguageTools::~LanguageTools() {
}


std::string LanguageTools::Detect(std::string &s) {
	std::string data;
	// convert string to UTF-16
	icu::UnicodeString s16 = icu::UnicodeString::fromUTF8(s);
	// canonicalize + lower-case
	icu::UnicodeString normalized;
	UErrorCode status = U_ZERO_ERROR;
	icu::Normalizer::normalize(s16, UNORM_NFC, 0, normalized, status);
	if (U_FAILURE(status)) {
		data = s;
	} else {
		normalized.toLower();
		// convert back to UTF-8
		data.reserve(s.length()*3);
		data = normalized.toUTF8String(data);
	}
	// detect language
	Language language3[3] = {
		UNKNOWN_LANGUAGE, UNKNOWN_LANGUAGE, UNKNOWN_LANGUAGE
	};
	int percent3[3] = { 0, 0, 0 };
	int text_bytes_tmp = 0;
	bool is_reliable;
	CompactLangDet::DetectLanguageSummary(NULL, data.c_str(), data.length(),
		true, language3, percent3, &text_bytes_tmp, &is_reliable);
	const int kMinTextPercentToCountLanguage = 20;
	Language lang = UNKNOWN_LANGUAGE;
	for (int i = 0; i < 3; i++) {
		if (IsValidLanguage(language3[i]) && !IS_LANGUAGE_UNKNOWN(language3[i]) &&
			percent3[i] >= kMinTextPercentToCountLanguage) {
			lang = language3[i];
			break;
		}
	}
	return (lang != UNKNOWN_LANGUAGE) ? LanguageCodeISO639_1(lang) : "";
}
