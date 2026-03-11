import math


def calibrated_fps(calibrate):
    """Calibration of the dynamic frames per second engine.

    I've started with the equation y = log10(x + m) * k + n, where:
      y is the desired fps, m and n are horizontal and vertical translation,
      k is a calibration factor, computed from some user input c (see readme for details).

    Considering minfps and maxfps as given constants, I came to:
      fps = log10(x + 1) * k + minfps, which must be equal to maxfps for x = c,
    so the factor k = (maxfps - minfps) / log10(c + 1), and
      fps = log10(x + 1) * (maxfps - minfps) / log10(c + 1) + minfps

    Neat! ;)

    Args:
        calibrate (float): user provided

    Returns:
        a callable to calculate the fps

    """
    min_fps, max_fps = 2., 60.
    calibrate = max(1e-6, calibrate)
    adjust_log_curve = 100. / min(calibrate, 100.)  # adjust the curve for small numbers
    factor = (max_fps - min_fps) / math.log10((calibrate * adjust_log_curve) + 1.)

    def fps(rate):
        if rate <= 0:
            return 10.  # bootstrap speed
        if rate < calibrate:
            return math.log10((rate * adjust_log_curve) + 1.) * factor + min_fps
        return max_fps

    return fps


def custom_fps(refresh_secs):
    def fps(_rate):
        return refresh_secs

    refresh_secs = 1 / refresh_secs
    return fps
