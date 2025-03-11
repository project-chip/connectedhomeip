import base64
import json
import logging
from functools import partial
import signal
import concurrent.futures

from chip.native import PyChipError
from chip.webrtc.types import SdpOfferCallback_t
from chip.webrtc.types import IceCallback_t
from chip.webrtc.types import SdpAnswerCallback_t
from chip.webrtc.types import ErrorCallback_t
from chip.webrtc.types import PeerConnectedCallback_t
from chip.webrtc.types import PeerDisconnectedCallback_t
from chip.webrtc.types import StatsCallback_t

future_offer = None
future_ice = None
candidates = []


def on_ice(candidates, peer):
    print("on_ice called")
    if future_ice is not None:
        future_ice.set_result(candidates)

def on_offer(offer, peer):
    print("on_offer called")
    offer_string = offer.decode('utf-8')
    print(offer_string)
    if future_offer is not None:
        future_offer.set_result(offer_string)
   
def on_answer(answer, peer):
    print("on_answer called")

def on_error(error, peer):
    print("on_error called")

def on_stats(stats, peer):
    print(stats)


offer_callback = SdpOfferCallback_t(on_offer)
ice_callback = IceCallback_t(on_ice) 
answer_callback = SdpAnswerCallback_t(on_answer)
error_callback = ErrorCallback_t(on_error)
stats_callback = StatsCallback_t(on_stats) 