import re

PATTERN_SANITIZE = re.compile(r'[\r\n]+')


def sanitize(text):
    return ' '.join(PATTERN_SANITIZE.split(text or ''))
