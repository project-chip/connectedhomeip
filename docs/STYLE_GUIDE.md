# Matter Documentation Style Guide

Matter documentation lives here:

-   **GitHub** — All guides and tutorials across the complete
    [Matter organization](https://github.com/project-chip).

See
[CONTRIBUTING.md](https://github.com/project-chip/connectedhomeip/blob/master/CONTRIBUTING.md)
for general information on contributing to this project.

## Location

Place all documentation contributions in the appropriate location in the
[`/docs`](../docs) directory. Most contributions should go into the
`/docs/guides` subdirectory, which covers conceptual and usage content.

Current documentation structure:

| Directory               | Description                                                                                  |
| ----------------------- | -------------------------------------------------------------------------------------------- |
| `/actions`              | Custom GitHub actions                                                                        |
| `/docs/guides`          | Conceptual or usage content that doesn't fit within a subdirectory, and high-level tutorials |
| `/docs/guides/images`   | All images included in guide content                                                         |
| `/docs/guides/profiles` | Content describing or illustrating use of Matter profiles                                    |
| `/docs/guides/test`     | Content related to testing Matter                                                            |
| `/docs/guides/tools`    | Content describing or illustrating use of Matter tools                                       |
| `/docs/guides/primer`   | Matter Primer content                                                                        |
| `/docs/presentations`   | PDF presentations on Matter features                                                         |
| `/docs/specs`           | PDFs of Matter specifications                                                                |
| `/images`               | Top-level Matter images, such as logos                                                       |

If you are unsure of the best location for your contribution, create an Issue
and ask, or let us know in your Pull Request.

## Style

Style to come

## Links

For consistency, all document links should point to the content on GitHub.

The text of a link should be descriptive, so it's clear what the link is for:

> For more information, see the [Matter Style Guide](./STYLE_GUIDE.md).

## Markdown guidelines

Use standard Markdown when authoring Matter documentation. While HTML may be
used for more complex content such as tables, use Markdown as much as possible.

> Note: Edit this file to see the Markdown behind the examples.

### Headers

The document title should be an h1 header (#) and in title case (all words are
capitalized). All section headers should be h2 (##) or lower and in sentence
case (only the first word and proper nouns are capitalized).

The best practice for document clarity is to not go lower than h3, but h4 is
fine on occasion. Try to avoid using h4 often, or going lower than h4. If this
happens, the document should be reorganized or broken up to ensure it stays at
h3 with the occasional h4.

### Command line examples

Feel free to use either `$` or `%` to preface command line examples, but be
consistent within the same doc or set of docs:

```
$ git clone https://github.com/project-chip/connectedhomeip.git
% git clone https://github.com/project-chip/connectedhomeip.git
```

### Terminal prompts

If you need to use a full terminal prompt with username and hostname, use the
format of `root@{hostname}{special-characters}#`.

For example, when logged into a Docker container, you might have a prompt like
this:

```
root@c0f3912a74ff:/#
```

### Commands and output

All example commands and output should be in code blocks with backticks:

```
code in backticks
```

...unless the code is within a step list. In a step list, indent the code
blocks:

    code indented

### Code blocks in step lists

When writing procedures that feature code blocks, indent the content for the
code blocks:

1.  Step one:

        $ git clone https://github.com/project-chip/connectedhomeip.git
        $ cd connectedhomeip

1.  Step two, do something else:

        $ ./configure

For clarity in instructions, avoid putting additional step commands after a code
sample within a step item. Instead rewrite the instruction so this is not
necessary.

For example, avoid this:

1.  Step three, do this now:

        $ ./configure

    And then you will see that thing.

Instead, do this:

1.  Step three, do this now, and you will see that thing:

        $ ./configure

### Inline code

Use backticks for `inline code`. This includes file paths and file or binary
names.

### Code Comments

Use uppercase `CHIP` in comments, as it is an acronym.

Supported keywords:

| Keyword | Description |
| ------- | ----------- |
| alarm   | Alarm       |
