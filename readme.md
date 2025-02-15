# Results

<table><tr>
	<th>programme</th>
	<th>model of word</th>
	<th>comments</th>
</tr><tr>
	<td>wc class</td>
	<td>[^ \f\n\r\t\v]+</td>
	<td>only ascii; super-successful; limited accuracy</td>
</tr><tr>
	<td>swift class</td>
	<td>[\w]+</td>
	<td>I don't know what \w is, documentation appears to say that it's equivalent to [\p{W}\p{Nd}]+, but I know this is a simplification</td>
</tr><tr>
	<td>binary class</td>
	<td>[\p{L}\p{M}\p{N}\p{Pc}\u200b\u200c\u200d\u2060]+</td>
	<td>this was the programme I came up with using upper-bound binary-search</td>
</tr><tr>
	<td>re2c class</td>
	<td>[\p{L}\p{M}\p{N}\p{Pc}\u200b\u200c\u200d\u2060]+</td>
	<td>this is an equivalent with re2c</td>
</tr><tr>
	<td>re2c regex</td>
	<td>((word_begin=(\p{L}\p{M}* | (\p{N}\[0-9])) (word_mid* word_end)?) | number=(binary|hex|integer|real|money))+</td>
	<td>see src/re2c_next_regex.re.c</td>
</tr>
</table>

I used my system's `wc`. I ran them though `time <programme> < input`. For `input`, I wanted to test multiple languages and a large book that's freely available. From the [list of literary works by number of translations](https://en.wikipedia.org/wiki/List_of_literary_works_by_number_of_translations) the Bible is probably the best bet, which I downloaded from [Bible Super Search](https://www.biblesupersearch.com/bible-downloads/). (I confusingly didn't see the version, but it's not really important for our purposes.)

![Korean Bible word count.](korean.svg)

The word counts agree except `wc`. From what limited knowledge I have of Korean, I think this is not surprising. The `swift` programme is a lot slower; I would like to think that this is an inherent problem with the Swift language, but I'm forced to admit I have never seen Swift code before and it's possible that I don't know what I'm doing.

![65 Bibles in different languages concatenated.](all.svg)

I concatenated all 65 versions of the Bible in different languages. I would have been interested to see what word count the `swift` programme gave, but it spectacularly balked. The expanded regex only merges a few thousandth's of words together in this source. The `re2c` and `wc` have similar run-times.

The `re2c` strategy of breaking a code-point up into bytes causes a huge amount of state fragmentation. On the other hand, I think of `binary` as being `O(log n)`, but with a fixed and small `n`, I was curious to see whether taking in whole code-points and doing a binary search would be that much slower. It's about twice as slow, in this case. I optimized this by skipping the nul-check on non-words and having two tables, but it didn't make much difference, so I took that complication out.

![Executable file size.](filesizes.svg)

It looks like `swift` and `binary` have the same algorithmic size. I would guess it uses a multi-stage hash-table. That's probably reading to much into it.

# What is this?

The model of words being delimited by white-space—specifically the `isspace` set: { ' ', '\f', '\n', '\f', '\t', '\v' }—is an approximation that does reasonably well on English texts. Of this simple model's detractions are: it doesn't actually point out word boundaries; it doesn't always work; it does poorly with more structured input (are "+" and "{" really words?); and it was developed for ascii when (rightfully) most modern text files are utf-8.

Wikipedia says that text-segmentation is "non-trivial." I looked up how to do this and every example was in Swift, so I wrote a a programme in Swift to compare with C. I initially was going to use a Patrica trie, but I wanted an upper-bound fixed binary-search.

Then I realized that this would be the ideal use-case for [re2c](https://github.com/skvadrik/re2c). Which I found conveniently updated to include Unicode category definitions in 2019; this made it trivial to have a richer regular-expression definition of a word. Very useful because, not only may graphemes be several code-points, but it allows the context of the code-point to be easily taken into account—the word context is limited to a regular language that can be recognized by a finite-state automaton, not a type-1 context-sensitive language.

# Binary class

I did this manually, and for that, turns out easier to build a model of my encoding. I am no expert on utf-8, but here's what I came up with. For the code-point [U+uvwxyz](https://en.wikipedia.org/wiki/UTF-8). These
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

Utf-8 pays careful attention to preserving the order of unicode, therefore I
have side-stepped the issue by working directly in utf-8-space—don't apply a
narrowing conversion to unicode; it is unicode-agnostic?  In a binary search
method, the tables are alternating is in this class or not. Erroneous indices,
therefore, pick up the surrounding properties. In this case, it likely
considers them among the class of not-words—an error in a word would probably
split it in two.

Chunking a file in blocks (>= 4 bytes) might create truncated code-points.
So for seeking to the end-byte, we (might) be in the middle of a valid
code-point. I keep an auxiliary 3-byte buffer to (possibly) complete the
code-point next time around. Using the above scheme, store any of the following truncations in the back of the buffer for the next read.

```
                  110-----
                  1110----
                  11110---
         1110---- 10------
         11110--- 10------
11110--- 10------ 10------
```

All others are complete or errors. This is simplified by not caring about distinguishing 2-byte, 3-byte, _etc_, errors, which in practice give little information: they are just errors. If one did care, it would be longer.

# Duplicating my results

Unicode data placed in the root of one's project. <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt>. Specifically, I used <https://www.unicode.org/Public/16.0.0/ucd/UnicodeData.txt>. It is 2.2MB.

Every `.c` in `test/` generates a new executable. I should probably update the `Makefile` instead of explaining.

The [re2c](https://re2c.org/)-generated file is large, but the gnu extension `--case-ranges` shortens this a lot.

`re2c [src/*.re.c test/*.re.c] -o build/[*.c] -8 --case-ranges --input-encoding utf8 -i`
