#include <config.h>

#include <vector>
#include "Tokenizer.h"
#include "TextResource.pb.h"

using namespace std;

extern "C" int fixup_init() {
	return 2;
}

extern "C" void fixup(vector<Token*> &tokens, int index) {
	tokens[index]->SetFlag(hector::resources::TOKEN_SENTENCE_START);
}
