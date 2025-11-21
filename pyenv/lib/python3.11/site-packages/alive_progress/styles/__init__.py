from .exhibit import Show, show_bars, show_spinners, show_themes, showtime
from .internal import BARS, SPINNERS, THEMES

__all__ = (
    'BARS', 'SPINNERS', 'THEMES', 'Show', 'showtime', 'show_spinners', 'show_bars', 'show_themes',
)

# I want to create some kind of extension point here. Then users would be able to create
# and share their custom styles with the community! Ideally it should be simple to distribute
# them, don't know for sure how. As a last resort, I could include them all here in this package,
# via the usual fork / pull request contribution, and I'll dynamically find and make them usable.
