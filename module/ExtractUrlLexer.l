%{
#include "ExtractUrlLexer.h"
%}

%s head body a area img frame iframe head_base meta meta_type meta_content
%x cdata script comment url meta_content_url
%option stack reentrant noyywrap extra-type="struct scanner_state *"

%%

\<!\[CDATA\[		yy_push_state(cdata, yyscanner);
<cdata>{
]]\>			yy_pop_state(yyscanner);
.|\n			;
}

\<script[^>]*\/\>	;
\<script		yy_push_state(script, yyscanner);
<script>{
\<\/script\>		yy_pop_state(yyscanner);
.|\n			;
}

\<!--			yy_push_state(comment, yyscanner);
<comment>{
--\>			yy_pop_state(yyscanner);
.|\n			;
}

<INITIAL>{
\<head[^>]*\/>		;
\<head			yy_push_state(head, yyscanner);
\<body[[:space:]]	{ yy_push_state(body, yyscanner); yyextra->inImage = 1; }
\<a[[:space:]]		yy_push_state(a, yyscanner);
\<area[[:space:]]	yy_push_state(area, yyscanner);
\<img[[:space:]]	{ yy_push_state(img, yyscanner); yyextra->inImage = 1; }
\<frame[[:space:]]	yy_push_state(frame, yyscanner);
\<iframe[[:space:]]	yy_push_state(iframe, yyscanner);
}

<head>{
\<base[[:space:]]	{ yy_push_state(head_base, yyscanner); yyextra->inBase = 1; }
\<meta[[:space:]]	yy_push_state(meta, yyscanner);
\<\/head\>		BEGIN(INITIAL);
}

<meta>{
http-equiv[[:space:]]*=[[:space:]]*['"]?	yy_push_state(meta_type, yyscanner);
content[[:space:]]*=[[:space:]]*['"]?		yy_push_state(meta_content, yyscanner);
\>						{ yy_pop_state(yyscanner); yyextra->inRefresh = 0; }
}
<meta_type>{
refresh			yyextra->inRefresh = 1;
.			yy_pop_state(yyscanner);
}
<meta_content>{
[[:digit:]]*;?url[[:space:]]*=		yy_push_state(meta_content_url, yyscanner);
.					yy_pop_state(yyscanner);
}
<meta_content_url>{
[^'">]+			{ if (yyextra->inRefresh) return TOK_REDIRECT; }
.			yy_pop_state(yyscanner);
}

<body>{
\>						{ yy_pop_state(yyscanner); yyextra->inImage = 0; }
background[[:space:]]*=[[:space:]]*['"]?	yy_push_state(url, yyscanner);
}

<img,frame,iframe>{
\>						{ yy_pop_state(yyscanner); yyextra->inImage = 0; }
src[[:space:]]*=[[:space:]]*['"]?		yy_push_state(url, yyscanner);
}

<a,area,head_base>{
\>						yy_pop_state(yyscanner);
href[[:space:]]*=[[:space:]]*['"]?		yy_push_state(url, yyscanner);
}

<url>{
[^'">]+			{ return yyextra->inBase ? TOK_BASE : ( yyextra->inImage ? TOK_IMG_URL : TOK_URL); }
.			{ yy_pop_state(yyscanner); yyextra->inBase = 0; yyextra->inImage = 0; }
}

.|\n			;

%%

void scanner_create(struct scanner_state *state, void **scanner) {
	yylex_init_extra(state, scanner);
}

void scanner_set_buffer(const char *data, int size, struct scanner_state *state, void *scanner) {
	yy_scan_bytes(data, size, scanner);
	state->inBase = 0;
	state->inRefresh = 0;
	state->inImage = 0;
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

/*
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
		printf("%d: %s\n", tok, text);
	}
	scanner_destroy(scanner);
	return 0;
}*/
