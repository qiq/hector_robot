struct scanner_state {
	int line;
};

enum token_type {
	TOK_EOF = 0,
	TOK_EOLN = 1,
	TOK_UNKNOWN = 2,
	TOK_STRING = 3,
	TOK_INT = 4,
	TOK_RULE_SEPARATOR = 5,
	TOK_TEST_SEPARATOR = 6,
	TOK_ACTION_SEPARATOR = 7,
	TOK_EQ = 8,
	TOK_NE = 9,
	TOK_GT = 10,
	TOK_LT = 11,
	TOK_GE = 12,
	TOK_LE = 13,
	TOK_RE = 14,
	TOK_NR = 15,
	TOK_ASSIGN = 16,
	TOK_REGEX = 17,
	TOK_REGEX_I = 18,
	TOK_REGEX_G = 19,
	TOK_REGEX_SUBST = 20,
	TOK_TEST_ANY = 21,
	TOK_IP4 = 22,
	TOK_IP6 = 23,
	TOK_IP_PREFIX = 24,
	TOK_LABEL = 25,
	TOK_FUNCTION = 26,
	TOK_ARRAY = 27,
	TOK_ALL = 28,
	TOK_ANY = 29,
};

void scanner_create(struct scanner_state *state, void **scanner);
enum token_type scanner_scan(char **text, void *scanner);
void scanner_set_buffer(const char *data, int size, struct scanner_state *state, void *scanner);
void scanner_destroy(void *scanner);
