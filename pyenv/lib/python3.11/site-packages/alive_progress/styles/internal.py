from ..animations.bars import bar_factory
from ..animations.spinners import alongside_spinner_factory, bouncing_spinner_factory, \
    delayed_spinner_factory, frame_spinner_factory, scrolling_spinner_factory, \
    sequential_spinner_factory


def _filter(context):
    return {k: v for k, v in context.items() if not k.startswith('_')}


def __create_spinners():
    classic = frame_spinner_factory(r'-\|/')
    stars = scrolling_spinner_factory('*', 4, 1, hide=False)
    twirl = frame_spinner_factory('←↖↑↗→↘↓↙')
    twirls = delayed_spinner_factory(twirl, 3)
    horizontal = frame_spinner_factory('▏▎▍▌▋▊▉█').reshape(1).bounce().reshape(7)
    vertical = frame_spinner_factory('▁▂▃▄▅▆▇█').reshape(1).bounce().reshape(7)
    waves = delayed_spinner_factory(vertical, 3, 2)
    waves2 = delayed_spinner_factory(vertical, 3, 5)
    waves3 = delayed_spinner_factory(vertical, 3, 7)
    dots = frame_spinner_factory('⠁⠈⠐⠠⢀⡀⠄⠂')
    dots_waves = delayed_spinner_factory(dots, 5)
    dots_waves2 = delayed_spinner_factory(dots, 5, 2)

    _balloon = bouncing_spinner_factory('🎈', 12, background='⠁⠈⠐⠠⢀⡀⠄⠂', overlay=True)
    it = sequential_spinner_factory(
        _balloon,
        _balloon,  # makes the balloon twice as common.
        bouncing_spinner_factory('🤡', background='⠁⠈⠐⠠⢀⡀⠄⠂', overlay=False),
        intermix=False
    ).randomize()

    ball_belt = bouncing_spinner_factory('●', 8, 0, '< >', hide=False)
    balls_belt = bouncing_spinner_factory('●', 8, 1, r'/~\_', hide=False)
    triangles = bouncing_spinner_factory(('▶', '◀'), 6, 2, hide=False)
    brackets = bouncing_spinner_factory(('>', '<'), 8, 3, hide=False)
    bubbles = bouncing_spinner_factory(('∙●⦿', '○'), 10, 5, hide=False)
    circles = bouncing_spinner_factory('●', 8, background='○', hide=False)
    squares = bouncing_spinner_factory('■', 8, background='□', hide=False)
    flowers = bouncing_spinner_factory('💐🌷🌸🌹🌺🌻🌼', 12, (2, 4)).pause(center=6).randomize()
    elements = bouncing_spinner_factory(('🔥💨', '🌊⚡️'), 6, 2)
    loving = bouncing_spinner_factory(('😍🥰', '⭐️🤩'), 8, (2, 3), '. ', hide=False, overlay=True)

    notes = bouncing_spinner_factory(('♩♪', '♫♬'), 8, 2, hide=False).pause(other=2)
    notes2 = delayed_spinner_factory(scrolling_spinner_factory('♩♪♫♬'), 3)

    arrow = scrolling_spinner_factory('>>----->', 15)
    arrows = bouncing_spinner_factory(('→', '←'), 6, 3)
    arrows2 = scrolling_spinner_factory('→➜➞➣➤➩➪➮', 5, 2, hide=False)
    _arrows_left = scrolling_spinner_factory('.˱·˂°❮', 6, 3, right=False)
    _arrows_right = scrolling_spinner_factory('.˲·˃°❯', 6, 3, right=True)
    arrows_in = alongside_spinner_factory(_arrows_right, _arrows_left)
    arrows_out = alongside_spinner_factory(_arrows_left, _arrows_right)

    _core = frame_spinner_factory('∙○⦿●')
    radioactive = alongside_spinner_factory(_arrows_left, _core, _arrows_right)

    boat = bouncing_spinner_factory((r'*|___/', r'\___|*'), 12, background='_.--.',
                                    hide=False, overlay=True)
    fish = scrolling_spinner_factory("><((('>", 15, hide=False)
    fish2 = bouncing_spinner_factory(("><('>", "<')><"), 12, hide=False)
    _fish_trail = scrolling_spinner_factory('¸.·´¯`·.·´¯`·.¸¸.·´¯`·.><(((º>', 15)
    _small_fishes = bouncing_spinner_factory(('><>     ><>', '<><  <><    <><'), 15)
    fishes = sequential_spinner_factory(_small_fishes, _small_fishes, _fish_trail,
                                        intermix=False).randomize()
    crab = bouncing_spinner_factory((r'Y (••) Y', r'Y (  ) Y'), 15, background='⠠⢀⡀⡀⢀⠄⡀⡀',
                                    hide=False, overlay=True)  # hey it's Ferris #rustacean!

    _look = bouncing_spinner_factory(('Look!', "It's moving!"))
    _alive = bouncing_spinner_factory(("It's alive!", "IT'S ALIVE!!"))
    alive = sequential_spinner_factory(_look, _alive, intermix=False)  # yep, frankenstein...

    wait = scrolling_spinner_factory('please wait...', right=False)
    wait2 = bouncing_spinner_factory(('please', 'wait'), 15, hide=False).pause()
    wait3 = bouncing_spinner_factory(('please', 'wait'), 15).pause(center=8)
    wait4 = bouncing_spinner_factory(('processing', 'this is not easy, please hold on'), 15)

    pulse = frame_spinner_factory((
        r'•––––––––––––', r'•––––––––––––', r'•––––––––––––', r'•––––––––-–––',
        r'–•–––––––––––', r'–•–––––––––––', r'–•–––––––––––', r'–•–––––––––––',
        r'––•––––––––––', r'––√––––––––––', r'––•––––––––––', r'––•––––––––––',
        r'–––•–––––––––', r'––√•–––––––––', r'–––•–––––––––', r'–––•–––––––––',
        r'––––•––––––––', r'––√-•––––––––', r'––––√––––––––', r'––––•––––––––',
        r'–––––•–––––––', r'––√--•–––––––', r'––––√\–––––––', r'–––––•–––––––',
        r'––––––•––––––', r'––√--–•––––––', r'––––√\/––––––', r'––––––•––––––',
        r'–––––––•–––––', r'–––--––•–––––', r'––––√\/•–––––', r'–––––––√–––––',
        r'––––––––•––––', r'––––-–––•––––', r'––––√\/–•––––', r'–––––––√\––––',
        r'–––––––––•–––', r'–––––––––•–––', r'–––––\/––•–––', r'–––––––√\•–––',
        r'––––––––––•––', r'––––––––––•––', r'––––––/–––•––', r'–––––––√\-•––',
        r'–––––––––––•–', r'–––––––––––•–', r'–––––––––––•–', r'–––––––√\-–•–',
        r'––––––––––––•', r'––––––––––––•', r'––––––––––––•', r'––––––––\-––•',
    )).reshape(4).transpose().randomize()

    return _filter(locals())


def __create_bars():
    smooth = bar_factory('▏▎▍▌▋▊▉█')
    classic = bar_factory('=', tip='>', borders='[]', errors='!x')
    classic2 = bar_factory('#', background='.', borders='[]', errors='!x')
    brackets = bar_factory('>')
    blocks = bar_factory('▏▎▍▌▋▊▉')
    bubbles = bar_factory('∙○⦿●', borders='<>')
    solid = bar_factory('∙□☐■', borders='<>')
    checks = bar_factory('✓')
    circles = bar_factory('●', background='○', borders='<>')
    squares = bar_factory('■', background='□', borders='<>')
    halloween = bar_factory('🎃', background='   👻   💀', errors=('😱', '🗡🗡🗡🗡'))
    filling = bar_factory('▁▂▃▄▅▆▇█')
    notes = bar_factory('♩♪♫♬', errors='♭♯')
    ruler = bar_factory(tip='┃', background='∙∙∙∙.')
    ruler2 = bar_factory(tip='┃', background='∙∙∙∙+')
    fish = bar_factory(tip="><('>", background='¸.·´¯`·.·´¯`·.¸¸.·´¯`·.')
    scuba = bar_factory(tip='>=≗)o', background='⠠⢀⡀⡀⢀⠄⡀⡀')

    return _filter(locals())


def __create_themes():
    smooth = dict(bar='smooth', spinner='waves', unknown='triangles')
    classic = dict(bar='classic', spinner='classic', unknown='brackets')
    scuba = dict(bar='scuba', spinner='fish2', unknown='fishes')  # I love scuba-diving.
    musical = dict(bar='notes', spinner='notes', unknown='notes2')

    return _filter(locals())


SPINNERS = __create_spinners()
BARS = __create_bars()
THEMES = __create_themes()
