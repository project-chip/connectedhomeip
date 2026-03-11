from enum import Enum

from grapheme.grapheme_property_group import GraphemePropertyGroup as GraphGroup
from grapheme.grapheme_property_group import get_group
from grapheme.incb_property_group import InCBPropertyGroup as InCBGroup
from grapheme.incb_property_group import get_group as get_group_incb


class BreakPossibility(Enum):
    CERTAIN = "certain"
    POSSIBLE = "possible"
    NO_BREAK = "nobreak"


def get_break_possibility(a, b):
    # Probably most common, included as short circuit before checking all else
    if a is GraphGroup.OTHER and b is GraphGroup.OTHER:
        return BreakPossibility.CERTAIN

    assert isinstance(a, GraphGroup)
    assert isinstance(b, GraphGroup)

    # Only break if preceeded by an uneven number of REGIONAL_INDICATORS
    # sot (RI RI)* RI × RI
    # [^RI] (RI RI) * RI × RI
    if a is GraphGroup.REGIONAL_INDICATOR and b is GraphGroup.REGIONAL_INDICATOR:
        return BreakPossibility.POSSIBLE

    # (Control | CR | LF) ÷
    #  ÷ (Control | CR | LF)
    if a in [GraphGroup.CONTROL, GraphGroup.CR, GraphGroup.LF] or b in [
        GraphGroup.CONTROL,
        GraphGroup.CR,
        GraphGroup.LF,
    ]:
        # CR × LF
        if a is GraphGroup.CR and b is GraphGroup.LF:
            return BreakPossibility.NO_BREAK
        else:
            return BreakPossibility.CERTAIN

    # L × (L | V | LV | LVT)
    if a is GraphGroup.L and b in [GraphGroup.L, GraphGroup.V, GraphGroup.LV, GraphGroup.LVT]:
        return BreakPossibility.NO_BREAK

    # (LV | V) × (V | T)
    if a in [GraphGroup.LV, GraphGroup.V] and b in [GraphGroup.V, GraphGroup.T]:
        return BreakPossibility.NO_BREAK

    # (LVT | T)    ×    T
    if a in [GraphGroup.LVT, GraphGroup.T] and b is GraphGroup.T:
        return BreakPossibility.NO_BREAK

    # × (Extend | ZWJ)
    # × SpacingMark
    # Prepend ×
    if b in [GraphGroup.EXTEND, GraphGroup.ZWJ, GraphGroup.SPACING_MARK] or a is GraphGroup.PREPEND:
        return BreakPossibility.NO_BREAK

    # \p{Extended_Pictographic} Extend* ZWJ × \p{Extended_Pictographic}
    if a is GraphGroup.ZWJ and b is GraphGroup.EXTENDED_PICTOGRAPHIC:
        return BreakPossibility.POSSIBLE

    # everything else, assumes all other rules are included above
    return BreakPossibility.CERTAIN


def get_break_possibility_incb(a, b):
    # Probably most common, included as short circuit before checking all else
    if a is InCBGroup.OTHER and b is InCBGroup.OTHER:
        return BreakPossibility.CERTAIN

    if a in [InCBGroup.LINKER, InCBGroup.EXTEND] and b is InCBGroup.CONSONANT:
        return BreakPossibility.NO_BREAK

    if a in [InCBGroup.LINKER, InCBGroup.EXTEND, InCBGroup.CONSONANT] and b is InCBGroup.LINKER:
        return BreakPossibility.NO_BREAK

    assert isinstance(a, InCBGroup)
    assert isinstance(b, InCBGroup)

    # everything else, assumes all other rules are included above
    return BreakPossibility.POSSIBLE


def get_last_certain_break_index(string, index):
    if index >= len(string):
        return len(string)

    prev = get_group(string[index])
    prev_incb = get_group_incb(string[index])
    while True:
        if index <= 0:
            return 0
        index -= 1
        cur = get_group(string[index])
        cur_incb = get_group_incb(string[index])
        if (
            get_break_possibility(cur, prev) == BreakPossibility.CERTAIN
            and get_break_possibility_incb(cur_incb, prev_incb) != BreakPossibility.NO_BREAK
        ):
            return index + 1
        prev = cur
        prev_incb = cur_incb


class UState(Enum):
    DEFAULT = 0  # No special case
    GB9c_Consonant = 10
    GB9c_Extend = 11
    GB9c_Linker = 12
    GB11_Picto = 20
    GB11_ZWJ = 21
    GB12_First = 30
    GB12_Second = 31


class GraphemeIterator:
    def __init__(self, string: str):
        self.str_iter = iter(string)
        self.buffer = ""
        self.lastg = None
        self.state = UState.DEFAULT
        try:
            self.buffer = next(self.str_iter)
        except StopIteration:
            self.buffer = None
        else:
            lastg = get_group(self.buffer)
            self.lastg = lastg
            if lastg is GraphGroup.EXTENDED_PICTOGRAPHIC:
                self.state = UState.GB11_Picto
            elif lastg is GraphGroup.REGIONAL_INDICATOR:
                self.state = UState.GB12_First
            else:
                lastincb = get_group_incb(self.buffer)
                if lastincb is InCBGroup.CONSONANT:
                    self.state = UState.GB9c_Consonant

    def __iter__(self):
        return self

    def default_should_break(self, nextg, nextincb):
        should_break = None
        next_state = UState.DEFAULT
        # First the most common
        if (
            self.lastg is GraphGroup.OTHER
            and nextg is GraphGroup.OTHER
            and nextincb is InCBGroup.OTHER
        ):
            # GB999     Any ÷ Any
            # Otherwise, break everywhere
            return True, UState.DEFAULT
        elif self.lastg is GraphGroup.CR and nextg is GraphGroup.LF:
            # GB3       CR × LF
            # Do not break between a CR and LF
            should_break = False
        elif self.lastg in (GraphGroup.CONTROL, GraphGroup.CR, GraphGroup.LF):
            # GB4       (Control | CR | LF) ÷
            # break before and after controls
            should_break = True
        elif nextg in (GraphGroup.CONTROL, GraphGroup.CR, GraphGroup.LF):
            # GB5        ÷ (Control | CR | LF)
            # break before and after controls.
            should_break = True
        elif self.lastg is GraphGroup.L and nextg in (
            GraphGroup.L,
            GraphGroup.V,
            GraphGroup.LV,
            GraphGroup.LVT,
        ):
            # GB6       L × (L | V | LV | LVT)
            # Do not break Hangul syllable or other conjoining sequences.
            should_break = False
        elif self.lastg in (GraphGroup.LV, GraphGroup.V) and nextg in (GraphGroup.V, GraphGroup.T):
            # GB7       (LV | V) × (V | T)
            # Do not break Hangul syllable or other conjoining sequences.
            should_break = False
        elif self.lastg in (GraphGroup.LVT, GraphGroup.T) and nextg is GraphGroup.T:
            # GB8       (LVT | T) × T
            # Do not break Hangul syllable or other conjoining sequences.
            should_break = False
        elif nextg in (GraphGroup.EXTEND, GraphGroup.ZWJ, GraphGroup.SPACING_MARK):
            # GB9        × (Extend | ZWJ)
            # Do not break before extending characters or ZWJ.
            # GB9a       × SpacingMark
            # Do not break before SpacingMarks
            should_break = False
        elif self.lastg is GraphGroup.PREPEND:
            # GB9b      Prepend ×
            # Do not break after Prepend characters
            should_break = False
        # Next State
        if nextg is GraphGroup.OTHER and nextincb is InCBGroup.OTHER:
            pass
        elif nextg is GraphGroup.EXTENDED_PICTOGRAPHIC:
            next_state = UState.GB11_Picto
        elif nextg is GraphGroup.REGIONAL_INDICATOR:
            next_state = UState.GB12_First
        elif nextincb is InCBGroup.CONSONANT:
            next_state = UState.GB9c_Consonant
        return should_break, next_state

    def __next__(self):
        for codepoint in self.str_iter:
            nextg = get_group(codepoint)
            next_inbc = get_group_incb(codepoint)
            sb, next_state = self.default_should_break(nextg, next_inbc)
            if self.state is UState.DEFAULT:
                pass
            # GB11
            elif self.state is UState.GB11_Picto:
                if nextg is GraphGroup.EXTEND:
                    next_state = UState.GB11_Picto
                    sb = False
                elif nextg is GraphGroup.ZWJ:
                    next_state = UState.GB11_ZWJ
                    sb = False
            elif self.state is UState.GB11_ZWJ and nextg is GraphGroup.EXTENDED_PICTOGRAPHIC:
                next_state = UState.DEFAULT
                sb = False
            # GB12  sot   (RI RI)* RI × RI
            # GB13  [^RI] (RI RI)* RI × RI
            # Do not break within emoji flag sequences.
            # That is, do not break between regional indicator (RI) symbols
            # if there is an odd number of RI characters before the break point.
            elif (
                self.state in (UState.GB12_First, UState.GB12_Second)
                and nextg is GraphGroup.REGIONAL_INDICATOR
            ):
                sb = self.state is not UState.GB12_First
                next_state = (
                    UState.GB12_First if self.state is UState.GB12_Second else UState.GB12_Second
                )
            # GB9c  Consonant [ Extend Linker ]* Linker [ Extend Linker ]* × Consonant
            elif self.state is UState.GB9c_Consonant:
                if next_inbc is InCBGroup.EXTEND:
                    sb = False
                    next_state = UState.GB9c_Consonant
                elif next_inbc is InCBGroup.LINKER:
                    sb = False
                    next_state = UState.GB9c_Linker
            elif self.state is UState.GB9c_Linker:
                if next_inbc is InCBGroup.LINKER:
                    sb = False
                    next_state = UState.GB9c_Linker
                elif next_inbc is InCBGroup.CONSONANT:
                    sb = False
                    next_state = UState.GB9c_Consonant
                elif next_inbc is InCBGroup.EXTEND:
                    sb = False
                    next_state = UState.GB9c_Linker
            # Handle results
            self.state = next_state
            self.lastg = nextg
            if sb is True or sb is None:
                return self._break(codepoint)
            self.buffer += codepoint  # type: ignore

        if self.buffer:
            # GB2  Any ÷ eot
            # Break at the end of text, unless the text is empty.
            return self._break(None)

        raise StopIteration()

    def _break(self, new):
        """Return the current buffer, start with a new one"""
        old_buffer = self.buffer
        self.buffer = new
        return old_buffer
