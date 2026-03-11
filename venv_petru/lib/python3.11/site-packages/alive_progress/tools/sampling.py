import timeit

from about_time.human_duration import fn_human_duration

from .utils import toolkit
from ..core.configuration import config_handler
from ..core.progress import __alive_bar

human_duration = fn_human_duration(False)


def overhead(total=None, *, calibrate=None, **options):
    number = 400  # timeit number of runs inside each repetition.
    repeat = 300  # timeit how many times to repeat the whole test.

    config = config_handler(disable=True, **options)
    with __alive_bar(config, total, calibrate=calibrate, _cond=__lock, _sampling=True) as loc:
        # the timing of the print_cells function increases proportionately with the
        # number of columns in the terminal, so I want a baseline here `VOID.cols == 0`.
        res = timeit.repeat('alive_repr()', repeat=repeat, number=number, globals=loc)

    return human_duration(min(res) / number)


OVERHEAD_SAMPLING_GROUP = [
    ('definite', dict(total=1)),
    ('manual(b)', dict(total=1, manual=True)),
    ('manual(u)', dict(manual=True)),
    ('unknown', dict()),
]
OVERHEAD_SAMPLING = [
    ('default', dict()),
    ('receipt', dict(receipt_text=True)),
    ('no spinner', dict(spinner=None)),
    ('no elapsed', dict(elapsed=False)),
    ('no monitor', dict(monitor=False)),
    ('no stats', dict(stats=False)),
    ('no bar', dict(bar=None)),
    ('only spinner', dict(bar=None, monitor=False, elapsed=False, stats=False)),
    ('only elapsed', dict(bar=None, spinner=None, monitor=False, stats=False)),
    ('only monitor', dict(bar=None, spinner=None, elapsed=False, stats=False)),
    ('only stats', dict(bar=None, spinner=None, monitor=False, elapsed=False)),
    ('only bar', dict(spinner=None, monitor=False, elapsed=False, stats=False)),
    ('none', dict(bar=None, spinner=None, monitor=False, elapsed=False, stats=False)),
]


def overhead_sampling():
    print('warmup', end='', flush=True)
    for _ in range(5):
        print('.', end='', flush=True)
        overhead()
    print('\r', end='', flush=True)

    max_name = max(len(x) for x, _ in OVERHEAD_SAMPLING)
    print(f'{"":>{max_name}} | {" | ".join(g for g, _ in OVERHEAD_SAMPLING_GROUP)} |')
    for name, config in OVERHEAD_SAMPLING:
        print(f'{name:>{max_name}} ', end='', flush=True)
        for group, data in OVERHEAD_SAMPLING_GROUP:
            print(f'| {overhead(**data, **config):^{len(group)}} ', end='', flush=True)
        print('|')


def __noop_p(_ignore):
    return 0


class __lock:
    def __enter__(self):
        pass

    def __exit__(self, _type, value, traceback):
        pass


if __name__ == '__main__':
    parser, run = toolkit('Estimates the alive_progress overhead per cycle on your system.')

    run(overhead_sampling)
