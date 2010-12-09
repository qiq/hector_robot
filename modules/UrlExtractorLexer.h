struct scanner_state {
	int inBase;
	int inRefresh;
};

enum token_type {
	TOK_EOF = 0,
	TOK_URL = 1,
	TOK_BASE = 2,
	TOK_REDIRECT = 3,
};

void scanner_create(struct scanner_state *state, void **scanner);
enum token_type scanner_scan(char **text, void *scanner);
void scanner_set_buffer(const char *data, int size, struct scanner_state *state, void *scanner);
void scanner_destroy(void *scanner);
