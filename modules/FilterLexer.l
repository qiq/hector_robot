%{
#include "FilterLexer.h"
%}

NUMBER	[[:digit:]]+
LABEL	[[:alpha:]][[:alnum:]]*

%x regex regex_flags regex_subst regex_subst2 ip_prefix ip_prefix2
%x array function function2
%option stack reentrant noyywrap extra-type="struct scanner_state *"

%%

	/* empty and comment lines, white space */
\n			{ yyextra->line++; }
^[[:space:]]*$		;
[[:space:]]*#.*		;
[[:space:]]+		;

	/* misc */
;			{ return TOK_EOLN; }
-\>			{ return TOK_RULE_SEPARATOR; }
&&			{ return TOK_TEST_SEPARATOR; }
,			{ return TOK_ACTION_SEPARATOR; }
\*			{ return TOK_ALL; }

	/* variable names, functions, arrays */
{LABEL}			{ return TOK_LABEL; }
{LABEL}\[		{ yytext[yyleng-1] = '\0'; yy_push_state(array, yyscanner); return TOK_ARRAY; }
clear\(			{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_CLEAR; }
delete\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_DELETE; }
count\(			{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_COUNT; }
length\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_LENGTH; }
delete_key\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_DELETE_KEY; }
delete_value\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_DELETE_VALUE; }
keep_key\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_KEEP_KEY; }
keep_value\(		{ yytext[yyleng-1] = '\0'; yy_push_state(function, yyscanner); return TOK_FUNCTION_KEEP_VALUE; }

	/* array */
<array>{
\*			{ return TOK_ALL; }
\?			{ return TOK_ANY; }
{LABEL}			{ return TOK_LABEL; }
\]			{ yy_pop_state(yyscanner);}
.			{ return TOK_UNKNOWN; }
}

<function>{
{LABEL}			{ return TOK_LABEL; }
\)			{ yy_pop_state(yyscanner); }
.			{ return TOK_UNKNOWN; }
}

<function2>{
{LABEL}			{ return TOK_LABEL; }
,			;
\/			{ yy_push_state(regex, yyscanner); }
}

	/* operators */
==			{ return TOK_EQ; }
!=			{ return TOK_NE; }
>			{ return TOK_GT; }
\<			{ return TOK_LT; }
>=			{ return TOK_GE; }
\<=			{ return TOK_LE; }
=~			{ return TOK_RE; }
!~			{ return TOK_NR; }
=			{ return TOK_ASSIGN; }

	/* string and int */
<INITIAL,array>{
\"(\\.|[^\"])*\"	{ yytext++; yytext[yyleng-2] = '\0'; return TOK_STRING; }
'(\\.|[^'])*'		{ yytext++; yytext[yyleng-2] = '\0'; return TOK_STRING; }
{NUMBER}		{ return TOK_INT; }
}


	/* regexes */
\/			{ yy_push_state(regex, yyscanner); }
s\/			{ yy_push_state(regex_subst, yyscanner); }
<regex>{
(\\.|[^\/])*		{ return TOK_REGEX; }
\/			{ BEGIN(regex_flags); }
}
<regex_flags>{
i			{ return TOK_REGEX_I; }
g			{ return TOK_REGEX_G; }
.			{ unput(yytext[0]); yy_pop_state(yyscanner); return TOK_REGEX_NOFLAGS; }
}
<regex_subst>{
(\\.|[^\/])*		{ return TOK_REGEX; }
\/			{ BEGIN(regex_subst2); }
}
<regex_subst2>{
(\\.|[^\/])*		{ return TOK_REGEX_SUBST; }
\/			{ BEGIN(regex_flags); }
}

	/* IP addresses */
[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}\.[[:digit:]]{1,3}	{ BEGIN(ip_prefix); return TOK_IP4; }
[a-fA-F[:digit:]]*:[a-fA-F[:digit:]]*:[:a-fA-F[:digit:]]*		{ BEGIN(ip_prefix); return TOK_IP6; }
<ip_prefix>{
\/			{ BEGIN(ip_prefix2); }
.			{ unput(yytext[0]); BEGIN(INITIAL); return TOK_IP_NOPREFIX; }
}
<ip_prefix2>{
[[:digit:]]+		{ BEGIN(INITIAL); return TOK_IP_PREFIX; }
.			{ unput(yytext[0]); BEGIN(INITIAL); }
}

	/* \n			; */
.			{ return TOK_UNKNOWN; }

%%

void scanner_create(struct scanner_state *state, void **scanner) {
	yylex_init_extra(state, scanner);
}

void scanner_set_buffer(const char *data, int size, struct scanner_state *state, void *scanner) {
	yy_scan_bytes(data, size, scanner);
	state->line = 1;
}

enum token_type scanner_scan(char **text, void *scanner) {
	int tok = yylex(scanner);
	if (tok == 0)
		return TOK_EOF;
	*text = yyget_text(scanner);
	return (enum token_type)tok;
}

void scanner_destroy(void *scanner) {
	yylex_destroy(scanner);
}

char buffer[1024*1024];
int main(int argc, char **argv) {
	if (argc < 1)
		return 1;
	yyscan_t scanner;
	struct scanner_state state;
	scanner_create(&state, &scanner);
	FILE *f = fopen(argv[1], "r");
	if (!f)
		return 1;
	size_t size = fread(buffer, 1, sizeof(buffer), f);
	fclose(f);
	scanner_set_buffer(buffer, size, &state, scanner);
	int tok;
	char *text;
	while ((tok = scanner_scan(&text, scanner)) != TOK_EOF) {
		if (tok == TOK_UNKNOWN)
			printf("error: unknown token line %d: %s\n", state.line, text);
		else
			printf("%d: %s\n", tok, text);
	}
	scanner_destroy(scanner);
	return 0;
}
