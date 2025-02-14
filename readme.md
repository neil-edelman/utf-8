# Results

![Korean Bible word count.](korean.svg)

![65 Bibles in different languages concatenated.](all.svg)

![Executable file size.](filesizes.svg)

# What is this?

The model of words being delimited by white-space—specifically the `isspace` set: { ' ', '\f', '\n', '\f', '\t', '\v' }—is an approximation that does reasonably well on English texts. Of this simple model's detractions are: it doesn't actually point out word boundaries; it doesn't always work; it does poorly with more structured input (are "+" and "{" really words?); and it was developed for ascii when (rightfully) most modern text files are utf-8.

Wikipedia says that text-segmentation is "non-trivial." I looked up how to do this and every example was in Swift, so I wrote a a programme in Swift to compare with C. I initially was going to use a Patrica trie, but I wanted an upper-bound fixed binary-search. The Swift documentation seems to say that `[\w]` is `[\p{L}\p{Nd}]`, which is obviously an oversimplification. I settled for `[\w]` is `[\p{L}\p{M}\p{N}\u200b\u200c\u200d\u2060]`.

Then I realized that this would be the ideal use-case for [re2c](https://github.com/skvadrik/re2c). Which I found conveniently updated to include Unicode category definitions in 2019; this made it trivial to have a richer regular-expression definition of a word. Very useful because, not only may graphemes be several code-points, but it allows the context of the code-point to be easily taken into account—the word context is limited to a regular language that can be recognized by a finite-state automaton, not a type-1 context-sensitive language.

# Binary class

This is the one I was testing.

For the code-point U+uvwxyz, <https://en.wikipedia.org/wiki/UTF-8>. These
are consumed.

```
"11111---" Error (for now?)
"11110uvv" "10vvwwww" "10xxxxyy" "10yyzzzz" 4-byte,
"11110---" "10------" "10------"            otherwise error,
"11110---" "10------"                       otherwise error,
"11110---"                                  otherwise error.
"1110wwww" "10xxxxyy" "10yyzzzz" 3-byte,
"1110----" "10------"            otherwise error,
"1110----"                       otherwise error.
"110xxxyy" "10yyxxxx" 2-byte,
"110-----"            otherwise error.
"10------" Continuation-byte not in the context of the above is in error.
"0yyyzzzz" 1-byte, (with checks for nul-termination if applicable.)
```

Errors are groups of bytes that are well-defined. How to parse these errors is
ultimately user-defined. A random continuation byte requires up to 3 back
reads to tell if it's an error. Surrogates, over-long encodings, and unicode
code-points that are not assigned (yet?) is further up to the application.

Utf-8 pays careful attention to preserving the order of unicode, therefore we
have side-stepped the issue by working directly in utf-8-space—don't apply a
narrowing conversion to unicode; it is unicode-agnostic?  In a binary search
method, the tables are alternating is in this class or not. Erroneous indices,
therefore, pick up the surrounding properties. In this case, it likely
considers them among the class of not-words (code-points that can be in words
are generally grouped together in a block?)—an error in a word would probably
split it in two.

Chunking a file in blocks (>= 4 bytes) might create truncated code-points.
So for seeking to the end-byte, we (might) be in the middle of a valid
code-point. We keep an auxiliary 3-byte buffer to (possibly) complete the
code-point next time around. Using the above scheme, we backtrack the
following.

```
				  110-----
				  1110----
				  11110---
		 1110---- 10------
		 11110--- 10------
11110--- 10------ 10------
```

All others are complete or errors, so they can be cut on the last byte.

# What one will need

Unicode data placed in the root of one's project. <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt>. Specifically, I used <https://www.unicode.org/Public/16.0.0/ucd/UnicodeData.txt>. It is 2.2MB.

Every `.c` in `test/` generates a new executable. I should probably update the `Makefile` instead of explaining.

The [re2c](https://re2c.org/)-generated file is large, but the gnu extension `--case-ranges` shortens this a lot.

`re2c [src/*.re.c test/*.re.c] -o build/[*.c] -8 --case-ranges --input-encoding utf8 -i`

I wanted to test my code in multiple languages and a large book that's freely available. [List of literary works by number of translations](https://en.wikipedia.org/wiki/List_of_literary_works_by_number_of_translations) lists the Bible as by far number 1. I downloaded at [Bible Super Search](https://www.biblesupersearch.com/bible-downloads/) and concatenated all of them.
