# Results

![Korean Bible word count.](korean.svg)

![65 Bibles in different languages concatenated.](all.svg)

![Executable file size.](filesizes.svg)

# What is this?

The model of words being delimited by white-space—specifically the `isspace` set: { ' ', '\f', '\n', '\f', '\t', '\v' }—is an approximation that does reasonably well on English texts. Of this simple model's detractions are: it doesn't actually point out word boundaries; it doesn't always work; it does poorly with more structured input (are "+" and "{" really words?); and it was developed for ascii when (rightfully) most modern text files are utf-8.

Wikipedia says that text-segmentation is "non-trivial." I looked up how to do this and every example was in Swift, so I wrote a a programme in Swift to compare with C. I initially was going to use a Patrica trie, but I wanted an upper-bound fixed binary-search. The Swift documentation seems to say that `[\w]` is `[\p{L}\p{Nd}]`, which is obviously an oversimplification. I settled for `[\w]` is `[\p{L}\p{M}\p{N}\u200b\u200c\u200d\u2060]`.

Then I realized that this would be the ideal use-case for [re2c](https://github.com/skvadrik/re2c). Which I found conveniently updated to include Unicode category definitions in 2019; this made it trivial to have a richer regular-expression definition of a word. Very useful because, not only may graphemes be several code-points, but it allows the context of the code-point to be easily taken into account—the word context is limited to a regular language that can be recognized by a finite-state automaton, not a type-1 context-sensitive language.

# What one will need

Unicode data placed in the root of one's project. <https://www.unicode.org/Public/UNIDATA/UnicodeData.txt>. Specifically, I used <https://www.unicode.org/Public/16.0.0/ucd/UnicodeData.txt>. It is 2.2MB.

Every `.c` in `test/` generates a new executable. I should probably update the `Makefile` instead of explaining.

The [re2c](https://re2c.org/)-generated file is large, but the gnu extension `--case-ranges` shortens this a lot.

`re2c [src/*.re.c test/*.re.c] -o build/[*.c] -8 --case-ranges --input-encoding utf8 -i`

I wanted to test my code in multiple languages and a large book that's freely available. [List of literary works by number of translations](https://en.wikipedia.org/wiki/List_of_literary_works_by_number_of_translations) lists the Bible as by far number 1. I downloaded at [Bible Super Search](https://www.biblesupersearch.com/bible-downloads/) and concatenated all of them.
