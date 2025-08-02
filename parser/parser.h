
#ifndef PARSER_H
#define PARSER_H


#ifndef INTS_DOT_H
#define INTS_DOT_H

    //
    // ints.h - just some ints that are nice to have
    //
    // Author   - Fletcher M
    //
    // Created  - 21/03/2025
    // Modified - 11/07/2025
    //

    #include <stdint.h>

    typedef uint64_t   u64;
    typedef uint32_t   u32;
    typedef uint16_t   u16;
    typedef uint8_t    u8;

    typedef int64_t    s64;
    typedef int32_t    s32;
    typedef int16_t    s16;
    typedef int8_t     s8;

    typedef u32        bool32;

    typedef float      f32;
    typedef double     f64;

    #define True       (0 == 0)
    #define False      (0 != 0)

#endif // INTS_DOT_H


#include "String_Helper.h"



///////////////////////////////////////////////////////////
//               Structures and Defines
///////////////////////////////////////////////////////////

#ifndef MAX_PEEK_COUNT
    // how far the parser can look ahead.
    #define MAX_PEEK_COUNT 64
#endif

typedef enum {
    // ASCII types are not referenced by name, just use ';' to reference them.

    // end of file.
    TK_EOF = 256,

    TK_Ident = 257,
    TK_String = 258,
} Token_Kind;

typedef struct {
    Token_Kind kind;

    SV text;

    // starts at 1
    s64 line_number;
    // starts at 1
    s64 col_number;
} Token;


typedef struct {
    SV original_text;

    SV parseing;


    s64 running_line_number;
    s64 running_col_number;

    s64 current_peek_count;
    // TODO make into a ring buffer
    Token peek_buffer[MAX_PEEK_COUNT];
} Parser;


///////////////////////////////////////////////////////////
//                   Function Headers
///////////////////////////////////////////////////////////

// return a new parser, it dose not allocate at any point.
Parser new_parser(SV text);

// just peeks the next token.
// wrapper for peek_token, equivlenent to peek_token(t, 0);
Token parser_peek_next(Parser *p);

// peek 'peek_index' tokens ahead,
// will assert if you ran past EOF, or you overran the buffer
// peek_index = 0   =is equal to=   next token
Token parser_peek(Parser *p, s64 index);


// take a token, and shift the peek buffer. works like a 'get_next_token'
Token parser_take_next(Parser *p);

// takes count tokens, returns the last
// returns the same as peeking the 'count-1' th token
Token parser_take(Parser *p, s64 count);


// the parser will sometimes take a moment to parse the input before it
// returns the next token. you can fill the peek buffer if you want to
// avoid a potential slowdown.
//
// will stop on TK_EOF
void parser_fill_peek_buffer(Parser *p);

#endif // PARSER_H


#ifdef PARSER_IMPLEMENTATION

#ifndef PARSER_IMPLEMENTATION_GUARD
#define PARSER_IMPLEMENTATION_GUARD


// TODO get rid of this thing?
#include <assert.h>


#define internal static


internal inline bool32 is_space(char c) {
    if (c == ' ')  return True;
    if (c == '\f') return True;
    if (c == '\n') return True;
    if (c == '\r') return True;
    if (c == '\t') return True;
    if (c == '\v') return True;
    return False;
}

internal inline bool32 is_alpha(char c) {
    if ('a' <= c && c <= 'z') return True;
    if ('A' <= c && c <= 'Z') return True;
    return False;
}

internal inline bool32 is_digit(char c) {
    if ('0' <= c && c <= '9') return True;
    return False;
}

// what a identifier is made of
internal inline bool32 is_ident_char(char c) {
    if (is_alpha(c)) return True;
    if (is_digit(c)) return True;
    if (c == '_')    return True;
    return False;
}


// returns a SV that has advanced past the whitespace.
internal SV chop_whitespace(Parser *p, SV parseing) {
    while (parseing.size > 0 && is_space(*parseing.data)) SV_advance(&parseing, 1);

    // TODO maybe chop '//' and '/*' as well...
    (void) p;

    return parseing;
}

internal void parser_advance(Parser *p, s64 count) {
    assert(p->parseing.size >= count); // dont do anything stupid

    // count new lines.
    s64 last_i_at_line = -1;
    for (s64 i = 0; i < count; i++) {
        if (p->parseing.data[i] == '\n') {
            p->running_line_number += 1;
            last_i_at_line = i;
        }
    }

    if (last_i_at_line == -1) {
        // we didn't cross a line boundary
        p->running_col_number += count;
    } else {
        p->running_col_number = count - last_i_at_line;
    }

    p->parseing.data += count;
    p->parseing.size -= count;
}

internal void parser_chop_whitespace(Parser *p) {
    SV new_parseing = chop_whitespace(p, p->parseing);
    s64 how_far_forward = new_parseing.data - p->parseing.data;
    parser_advance(p, how_far_forward);
}

internal Token parser_internal_get_next_token(Parser *p) {
    assert(p->parseing.size >= 0);

    parser_chop_whitespace(p);

    Token result = {0};
    result.line_number = p->running_line_number;
    result.col_number  = p->running_col_number;

    if (p->parseing.size == 0) {
        result.kind = TK_EOF;
        // the size is zero but the user might enjoy this.
        result.text = p->parseing;
        return result;
    }

    char next_char = *p->parseing.data;

    // identifier
    if (is_alpha(next_char)) {
        result.kind = TK_Ident;

        result.text = (SV){.data = p->parseing.data, .size = 1};
        for (s64 i = 1; i < p->parseing.size; i++) {
            if (!is_ident_char(p->parseing.data[i])) break;
            result.text.size += 1;
        }

        return result;
    }

    // TODO parse numbers?

    if (next_char == '"') {
        SV thing = {.data = p->parseing.data + 1, .size = p->parseing.size - 1};

        while (True) {
            s64 index = SV_find_index_of_char(thing, '"');

            if (index == -1) {
                // TODO return a TK_ERROR
                assert(False);
            }

            // TODO make a note about how we dont detect the newline in this string.

            // count number of prev /
            s64 n = 0;
            for (s64 i = index-1; i > 0; i--) {
                if (thing.data[i] != '\\') break;
                n += 1;
            }
            thing.data += index + 1;
            thing.size -= index + 1;
            if (n % 2 == 0) break;
        }

        result.kind = TK_String;
        result.text = (SV) {
            .data = p->parseing.data,
            .size = p->parseing.size - thing.size,
        };
        return result;
    }


    result.kind = (Token_Kind) next_char;
    result.text = (SV){.data = p->parseing.data, .size = 1};
    return result;
}


internal void parser_internal_next_token_into_peek_buffer(Parser *p) {
    assert(p->current_peek_count < MAX_PEEK_COUNT && "parser_internal_next_token_into_peek_buffer: Cannot get more than 'MAX_PEEK_COUNT' peeks ahead");

    if (p->current_peek_count > 0) {
        // TODO we could just check parseing.size == 0?
        // ^ might not work.
        Token last_token = p->peek_buffer[p->current_peek_count - 1];
        assert(last_token.kind != TK_EOF && "Cannot get next token when the last token is currently EOF");
    }

    Token token = parser_internal_get_next_token(p);
    // advance past the token
    parser_advance(p, token.text.size);

    p->peek_buffer[p->current_peek_count] = token;
    p->current_peek_count += 1;
}

// dose not actually do memmove
internal void Parser_internal_memmove(void *dest, void *src, u64 n) {
    assert(dest < src && "This is not a real memmove");

    char *d = dest; char *s = src;
    for (u64 i = 0; i < n; i++) d[i] = s[i];
}



///////////////////////////////////////////////////////////
//             Function Implementations
///////////////////////////////////////////////////////////


Parser new_parser(SV text) {
    Parser result = {
        .original_text = text,

        .parseing = text,

        .running_line_number = 1,
        .running_col_number  = 1,

        .current_peek_count = 0,
        // init this to zero, even though we probably dont have to.
        .peek_buffer = {0},
    };

    return result;
}


Token parser_peek_next(Parser *p) { return parser_peek(p, 0); }

Token parser_peek(Parser *p, s64 index) {
    assert(index < MAX_PEEK_COUNT && "parser_peek: index is more than the maximum allowed peeks. we have nowhere to store the peeks.");

    // generate until we have enough tokens.
    while (index >= p->current_peek_count) {
        parser_internal_next_token_into_peek_buffer(p);
    }

    return p->peek_buffer[index];
}


Token parser_take_next(Parser *p) { return parser_take(p, 1); }

Token parser_take(Parser *p, s64 count) {
    assert(count > 0 && "Cannot take 0 tokens, maybe we could, but what would we return?");

    // TODO support this.
    assert(count < MAX_PEEK_COUNT && "Cannot take more than the max buffer count, why would you do this anyway? how do you know where your going? we could support this...");

    Token token = parser_peek(p, count-1);

    // shift the buffer.

    p->current_peek_count -= count;
    Parser_internal_memmove(p->peek_buffer, p->peek_buffer + count, p->current_peek_count * sizeof(*p->peek_buffer));

    return token;
}


void parser_fill_peek_buffer(Parser *p) {
    while (p->current_peek_count < MAX_PEEK_COUNT) {
        // check if the last token was EOF, if so exit.
        if (p->current_peek_count > 0) {
            Token last_token = p->peek_buffer[p->current_peek_count-1];
            if (last_token.kind == TK_EOF) break;
        }
        parser_internal_next_token_into_peek_buffer(p);
    }
}






#endif // PARSER_IMPLEMENTATION_GUARD
#endif // PARSER_IMPLEMENTATION
