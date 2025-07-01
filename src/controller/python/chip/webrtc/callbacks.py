import logging

from chip.webrtc.types import ErrorCallback_t, IceCallback_t, SdpAnswerCallback_t, SdpOfferCallback_t, StatsCallback_t

future_offer = None
future_ice = None
candidates = []

logger = logging.getLogger(__name__)

def on_ice(candidates, peer):
    logger.info(f"on_ice received ICE candidates: {candidates}")
    if future_ice is not None:
        future_ice.set_result(candidates)


def on_offer(offer, peer):
    logger.info(f"on_offer received SDP offer: {offer}")
    offer_string = offer.decode('utf-8')
    if future_offer is not None:
        future_offer.set_result(offer_string)


def on_answer(answer, peer):
    logger.info(f"on_answer received SDP answer: {answer}")


def on_error(error, peer):
    logger.error(f"on_error received error: {error}")


def on_stats(stats, peer):
    logger.info(f"on_stats received stats: {stats}")


offer_callback = SdpOfferCallback_t(on_offer)
ice_callback = IceCallback_t(on_ice)
answer_callback = SdpAnswerCallback_t(on_answer)
error_callback = ErrorCallback_t(on_error)
stats_callback = StatsCallback_t(on_stats)
