/**
FeaturamaTagger.la, simple, native
Tag TextResources sentences with Featurama tagger (first, we apply morphology
and then we run tagger).

Dependencies: libczmorphology, libfeaturama

Parameters:
items			r/o	Total items processed
morphologyPrefix	init	Prefix of the Czech morphology files:
				*{e.cpd,w.cpd,g.txt}, for guesser also *u.cpd
useGuesser		init	Use guesser or not
taggerPrefix		init	Tagger data files prefix: *{alpha,dict,f}
*/

#ifndef _MODULES_FEATURAMA_TAGGER_H_
#define _MODULES_FEATURAMA_TAGGER_H_

#include <config.h>

#include <string>
#include "common.h"
#include "Module.h"
#include "ObjectProperties.h"

class FeaturamaTagger : public Module {
public:
	FeaturamaTagger(ObjectRegistry *objects, const char *id, int threadIndex);
	~FeaturamaTagger();
	bool Init(std::vector<std::pair<std::string, std::string> > *params);
	Module::Type GetType();
	Resource *ProcessSimpleSync(Resource *resource);

private:
	int items;		// ObjectLock, items processed
	char *morphologyPrefix;
	bool useGuesser;
	char *taggerPrefix;

	char *GetItems(const char *name);
	char *GetMorphologyPrefix(const char *name);
        void SetMorphologyPrefix(const char *name, const char *value);
	char *GetUseGuesser(const char *name);
        void SetUseGuesser(const char *name, const char *value);
	char *GetTaggerPrefix(const char *name);
        void SetTaggerPrefix(const char *name, const char *value);

	ObjectProperties<FeaturamaTagger> *props;
	char *GetPropertySync(const char *name);
	bool SetPropertySync(const char *name, const char *value);
	std::vector<std::string> *ListPropertiesSync();

	std::string MorphologyToOffer(const char *s);
	std::string CstsEncode(const char *src);
	std::string CstsEncodeLemma(const char *src, int len);

	PERC *perc;
	const char *csts_encode_table[256];
};

inline Module::Type FeaturamaTagger::GetType() {
	return SIMPLE;
}

inline char *FeaturamaTagger::GetPropertySync(const char *name) {
	return props->GetProperty(name);
}

inline bool FeaturamaTagger::SetPropertySync(const char *name, const char *value) {
	return props->SetProperty(name, value);
}

inline std::vector<std::string> *FeaturamaTagger::ListPropertiesSync() {
	return props->ListProperties();
}

#endif
