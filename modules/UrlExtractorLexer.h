struct scanner_state {
	int inBase;
	int inRefresh;
	int inImage;
};

enum token_type {
	TOK_EOF = 0,
	TOK_URL = 1,
	TOK_IMG_URL = 2,
	TOK_BASE = 3,
	TOK_REDIRECT = 4,
};

void scanner_create(struct scanner_state *state, void **scanner);
enum token_type scanner_scan(char **text, void *scanner);
void scanner_set_buffer(const char *data, int size, struct scanner_state *state, void *scanner);
void scanner_destroy(void *scanner);
