"""
This module must always be importable, even without the required libs for install!
It's because I import metadata from main init, directly in setup.py, which imports this.
"""
import os
import sys
from collections import namedtuple
from string import Formatter
from types import FunctionType

from ..utils import sanitize

ERROR = object()  # represents a config value not accepted.


def _spinner_input_factory(default):
    from ..animations import spinner_compiler
    from ..styles.internal import SPINNERS
    return __style_input(SPINNERS, spinner_compiler, 'spinner_compiler_dispatcher_factory', default)


def _bar_input_factory():
    from ..animations import bars
    from ..styles.internal import BARS
    return __style_input(BARS, bars, 'bar_assembler_factory', None)


def __style_input(key_lookup, module_lookup, inner_name, default):
    def _input(x):
        return name_lookup(x) or func_lookup(x) or default

    name_lookup = __name_lookup(key_lookup)
    func_lookup = __func_lookup(module_lookup, inner_name)
    _input.err_help = f'Expected a custom factory or one of: {tuple(key_lookup)}'
    return _input


def __name_lookup(name_lookup):
    def _input(x):
        if isinstance(x, str):
            return name_lookup.get(x) or ERROR

    return _input


def __func_lookup(module_lookup, inner_name):
    def _input(x):
        if isinstance(x, FunctionType):
            func_file, _ = os.path.splitext(module_lookup.__file__)
            if x.__code__.co_name == inner_name \
                    and func_file.endswith(os.path.splitext(x.__code__.co_filename)[0]):
                return x
            return ERROR

    return _input


def _int_input_factory(lower, upper):
    def _input(x):
        try:
            x = int(x)
            return x if lower <= x <= upper else ERROR
        except TypeError:
            return ERROR

    _input.err_help = f'Expected an int between {lower} and {upper}'
    return _input


def _float_input_factory(lower, upper):
    def _input(x):
        try:
            x = float(x)
            return x if lower <= x <= upper else ERROR
        except TypeError:
            return ERROR

    _input.err_help = f'Expected a float between {lower} and {upper}'
    return _input


def _bool_input_factory():
    def _input(x):
        return bool(x)

    return _input


def _tri_state_input_factory():
    def _input(x):
        return None if x is None else bool(x)

    return _input


def _text_input_factory():
    def _input(x):
        return None if x is None else sanitize(str(x))

    return _input


def _options_input_factory(valid: tuple, alias: dict):
    def _input(x):
        x = alias.get(x, x)
        return x if x in valid else ERROR

    assert all(v in valid for v in alias.values()), f'invalid aliases: {alias.values()}'
    _input.err_help = f'Expected one of: {valid + tuple(alias)}'
    return _input


def _format_input_factory(allowed):
    def _input(x):
        if not isinstance(x, str):
            return bool(x)
        fvars = parser.parse(x)
        if any(f[1] not in allowed_all for f in fvars):
            # f is a tuple (literal_text, field_name, format_spec, conversion)
            return ERROR
        return x

    allowed = allowed.split()
    # I want to accept only some field names, and pure text.
    allowed_all = set(allowed + [None])
    parser = Formatter()
    _input.err_help = f'Expected only the fields: {tuple(allowed)}'
    return _input


def _file_input_factory():
    def _input(x):
        return x if all(hasattr(x, m) for m in ('write', 'flush')) else ERROR

    _input.err_help = 'Expected sys.stdout, sys.stderr, or a similar TextIOWrapper object'
    return _input


Config = namedtuple('Config', 'title length max_cols spinner bar unknown force_tty disable manual '
                              'enrich_print enrich_offset receipt receipt_text monitor elapsed stats '
                              'title_length spinner_length refresh_secs monitor_end elapsed_end '
                              'stats_end ctrl_c dual_line unit scale precision file')


def create_config():
    def reset():
        """Resets global configuration to the default one."""
        set_global(  # this must have all available config vars.
            title=None,
            length=40,
            max_cols=80,
            theme='smooth',  # includes spinner, bar and unknown.
            force_tty=None,
            file=sys.stdout,
            disable=False,
            manual=False,
            enrich_print=True,
            enrich_offset=0,
            receipt=True,
            receipt_text=False,
            monitor=True,
            elapsed=True,
            stats=True,
            monitor_end=True,
            elapsed_end=True,
            stats_end=True,
            title_length=0,
            spinner_length=0,
            refresh_secs=0,
            ctrl_c=True,
            dual_line=False,
            unit='',
            scale=None,
            precision=1,
        )

    def set_global(theme=None, **options):
        """Update the global configuration, to be used in subsequent alive bars.

        See Also:
            alive_progress#alive_bar(**options)

        """
        lazy_init()
        global_config.update(_parse(theme, options))

    def create_context(theme=None, **options):
        """Create an immutable copy of the current configuration, with optional customization."""
        lazy_init()
        local_config = {**global_config, **_parse(theme, options)}
        return Config(**local_config)

    def _parse(theme, options):
        """Validate and convert some configuration options."""

        def validator(key, value):
            try:
                validation = validations[key]
            except KeyError:
                raise ValueError(f'Invalid config key: {key!r}')

            try:
                result = validation(value)
                if result is ERROR:
                    raise UserWarning(validation.err_help)
                return result
            except UserWarning as e:
                raise ValueError(f'Invalid config value: {key}={value!r}\n{e}') from None
            except Exception as e:
                raise ValueError(f'Error in config value: {key}={value!r}\nCause: {e!r}') from None

        from ..styles.internal import THEMES
        if theme:
            if theme not in THEMES:
                raise ValueError(f'invalid theme name={theme}')
            swap = options
            options = dict(THEMES[theme])
            options.update(swap)
        return {k: validator(k, v) for k, v in options.items()}

    def lazy_init():
        if validations:
            return

        validations.update(  # the ones the user can configure.
            title=_text_input_factory(),
            length=_int_input_factory(3, 1000),
            max_cols=_int_input_factory(3, 1000),
            spinner=_spinner_input_factory(None),  # accept empty.
            bar=_bar_input_factory(),
            unknown=_spinner_input_factory(ERROR),  # do not accept empty.
            force_tty=_tri_state_input_factory(),
            file=_file_input_factory(),
            disable=_bool_input_factory(),
            manual=_bool_input_factory(),
            enrich_print=_bool_input_factory(),
            enrich_offset=_int_input_factory(0, sys.maxsize),
            receipt=_bool_input_factory(),
            receipt_text=_bool_input_factory(),
            monitor=_format_input_factory('count total percent'),
            monitor_end=_format_input_factory('count total percent'),
            elapsed=_format_input_factory('elapsed'),
            elapsed_end=_format_input_factory('elapsed'),
            stats=_format_input_factory('rate eta'),
            stats_end=_format_input_factory('rate'),
            title_length=_int_input_factory(0, 1000),
            spinner_length=_int_input_factory(0, 1000),
            refresh_secs=_float_input_factory(0, 60 * 60 * 24),  # maximum 24 hours.
            ctrl_c=_bool_input_factory(),
            dual_line=_bool_input_factory(),
            # title_effect=_enum_input_factory(),  # TODO someday.
            unit=_text_input_factory(),
            scale=_options_input_factory((None, 'SI', 'IEC', 'SI2'),
                                         {'': None, False: None, True: 'SI',
                                          10: 'SI', '10': 'SI',
                                          2: 'IEC', '2': 'IEC'}),
            precision=_int_input_factory(0, 2),
        )
        assert all(k in validations for k in Config._fields)  # ensures all fields have validations.

        reset()
        assert all(k in global_config for k in Config._fields)  # ensures all fields have been set.

    global_config, validations = {}, {}
    create_context.set_global, create_context.reset = set_global, reset
    return create_context


config_handler = create_config()
