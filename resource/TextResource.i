%include "std_string.i"
%include "std_vector.i"

%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%{
#include "TextResource.h"
%}

class TextResourceInfo : public ResourceInfo {
public:
        TextResourceInfo();
};

class TextResource : public Resource {
public:
        // keep synchronized with TextResource.proto
        enum Flags {
                TOKEN_NONE = 0,
                TOKEN_PARAGRAPH_START = 1,
                TOKEN_SENTENCE_START = 2,
                TOKEN_NO_SPACE = 4,
                TOKEN_ABBR = 8,
                TOKEN_PUNCT = 16,
                TOKEN_TITLECASE = 32,
                TOKEN_UPPERCASE = 64,
                TOKEN_NUMERIC = 128,
                TOKEN_UNRECOGNIZED = 256,
        };

        TextResource();
        TextResource(const TextResource &wr);
        ~TextResource();
        Resource *Clone();
        void Clear();
        bool Serialize(ResourceOutputStream &output);
        bool Deserialize(ResourceInputStream &input, bool headerOnly);
        bool Skip(ResourceInputStream &input);
        int GetSize();
        ResourceInfo *GetResourceInfo();
        std::string ToString(Object::LogLevel = Object::INFO);

        void SetText(const std::string &text);
        const std::string GetText();
        void ClearText();
        void SetTextId(const std::string &text);
        const std::string GetTextId();
        void ClearTextId();
        void SetFlags(int index, int flags);
        int GetFlags(int index);
        void ClearFlags();
        int GetFlagsCount();
        void SetForm(int index, const std::string &text);
        const std::string GetForm(int index);
        void ClearForm();
        int GetFormCount();
        void SetLemma(int index, const std::string &text);
        const std::string GetLemma(int index);
        void ClearLemma();
        int GetLemmaCount();
        void SetPosTag(int index, const std::string &text);
        const std::string GetPosTag(int index);
        void ClearPosTag();
        int GetPosTagCount();
        void SetHead(int index, int flags);
        int GetHead(int index);
        void ClearHead();
        int GetHeadCount();
        void SetDepRel(int index, const std::string &text);
        const std::string GetDepRel(int index);
        void ClearDepRel();
        int GetDepRelCount();

        static bool IsInstance(Resource *resource);
};

%inline %{
TextResource *ResourceToTextResource(Resource *r) {
        return r && TextResource::IsInstance(r) ? static_cast<TextResource*>(r) : NULL;
}
%}
