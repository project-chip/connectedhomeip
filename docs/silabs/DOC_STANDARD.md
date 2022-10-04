# Silicon Labs Documentation Standards Using Markdown

## Titles
# This Is a Title
Notice how the main words are capitalized

## Subsections Look Like This
This is a `##` subsection under a `#` title header. Notice how the words are also capitalized.

## Lists

- This is a bulleted list
- Second item in a bulleted list

1. This is a numbered list
2. Second item in a numbered list

## Commands

```shell
$ this is a highlighted command
```

```shell
$ if there is a second line it has it's own entry
```

```shell
$ notice how commands all start with the dollar sign, $, this is to indicate that they are to be used on a command line
```


## Code
```cpp
This is a code block() {
    Not to be confused with a highlighted command as above ^^^
    This should only be used for preformatted code, not for commands.
}
```

## Notes
> **Note:** This is a note that we want to draw attention to, it is similar to a highlighted command except that it does not generally use `preformatted text`.


## Tables
| col 1 | col 2 | col 3 |
| ----- | ----- | ----- |
| don't do | large tables | they don't |
| format well | or look good | and are |
| difficult | to | maintain |

If you need to do a large table, find some other way to represent the information in .md. You could use HTML but this is also frowned upon for the reason listed below...

## HTML
Try to avoid <br> Using HTML <br> Since it doesn't <br> get picked up <br> by .md translators
