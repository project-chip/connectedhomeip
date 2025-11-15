"""
Very basic color implementation, just to print fixed messages on screen.

It's very hard to support colors inside any moving parts of alive-progress, as I would need
to implement operations like len, indexing, slicing, concatenation and revert, while maintaining
the color information correct! There's an impedance mismatch between what we see and how we
represent it (yeah, similar to unicode grapheme clusters 😅).

I'd say it is possible now with the new cell architecture, I'd just need some new rules.
I would need to split color information just like grapheme clusters and mark them like cells,
but with the opposite effect. In cell the marks increase the len for the wide chars, but here
they must not change it somehow.
Also, the ansi escape codes the terminal receives changes its state, so I couldn't simply truncate
the line anymore, it'd need another escape code to finalize that one, and return it to its previous
state!
So, these special cells would require yet more work.

"""


def color_factory(color_code):
    def apply(text: str, format_spec=''):
        return f'{color_code}{text:{format_spec}}\x1b[0m'

    def mix(*colors):
        return [color_factory(c.color_code + color_code) for c in colors]

    apply.mix, apply.color_code = mix, color_code
    return apply


# colors
BLUE = color_factory('\x1b[94m')
GREEN = color_factory('\x1b[92m')
YELLOW = color_factory('\x1b[93m')
RED = color_factory('\x1b[91m')
MAGENTA = color_factory('\x1b[95m')
CYAN = color_factory('\x1b[96m')
ORANGE = color_factory('\x1b[38;5;208m')

# modifiers
BOLD = color_factory('\x1b[1m')
DIM = color_factory('\x1b[2m')
ITALIC = color_factory('\x1b[3m')
UNDERLINE = color_factory('\x1b[4m')

BLUE_BOLD, BLUE_DIM, BLUE_IT, BLUE_UNDER = BLUE.mix(BOLD, DIM, ITALIC, UNDERLINE)
GREEN_BOLD, GREEN_DIM, GREEN_IT, GREEN_UNDER = GREEN.mix(BOLD, DIM, ITALIC, UNDERLINE)
YELLOW_BOLD, YELLOW_DIM, YELLOW_IT, YELLOW_UNDER = YELLOW.mix(BOLD, DIM, ITALIC, UNDERLINE)
RED_BOLD, RED_DIM, RED_IT, RED_UNDER = RED.mix(BOLD, DIM, ITALIC, UNDERLINE)
MAGENTA_BOLD, MAGENTA_DIM, MAGENTA_IT, MAGENTA_UNDER = MAGENTA.mix(BOLD, DIM, ITALIC, UNDERLINE)
CYAN_BOLD, CYAN_DIM, CYAN_IT, CYAN_UNDER = CYAN.mix(BOLD, DIM, ITALIC, UNDERLINE)
ORANGE_BOLD, ORANGE_DIM, ORANGE_IT, ORANGE_UNDER = ORANGE.mix(BOLD, DIM, ITALIC, UNDERLINE)
