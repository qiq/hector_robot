#include <config.h>

#include <vector>
#include "Tokenizer.h"
#include "TextResource.h"

using namespace std;

extern "C" int fixup_init() {
	return 2;
}

extern "C" void fixup(vector<Token*> &tokens, int index) {
	tokens[index]->SetFlag(TextResource::TOKEN_SENTENCE_START);
}
