/**
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <cstdint>

namespace chip {
namespace app {
namespace Clusters {

// A delegate instance is bound to a single ColorControlCluster (and therefore a single endpoint).
// Matches its siblings LevelControlDelegate / OnOffDelegate.
//
// Scope: only behavior the cluster cannot perform itself and must invoke per call — color-space conversion
// and hardware output. Choices that are fixed for the product are not delegate calls; they are passed once
// through StaticConfig (see ColorControlCluster::Config).
//
// Every method here is called BY the cluster; a delegate never calls into the cluster. Each has an empty
// default, so an implementation overrides only what its hardware needs.
//
// ── Implementing a driver ───────────────────────────────────────────────────────────────────────────
//
// There are two ways to drive the light, and which one you get is decided entirely by which methods you
// override. The cluster makes the same calls either way and never branches on the answer.
//
// SOFTWARE-DRIVEN (the default). Override only the On*Changed methods and write the value to the hardware
// every time. The cluster interpolates each movement and feeds you a value every tick, so the fade is
// produced by those calls. Ignore transitionActive entirely:
//
//     void OnColorXYChanged(uint16_t x, uint16_t y, bool) override { WritePwm(x, y); }
//
// HARDWARE-DRIVEN. Use this when the LED driver, PWM controller or lighting IC can ramp on its own. Also
// override the On*TransitionStarted methods, start the native ramp there, and skip the software frames:
//
//     void OnXYTransitionStarted(uint16_t targetX, uint32_t durXMs, uint16_t targetY, uint32_t durYMs) override
//     {
//         mChip.Fade(targetX, targetY, std::max(durXMs, durYMs)); // convert the ENDPOINT once, not 30x
//     }
//
//     void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) override
//     {
//         if (transitionActive) { return; }  // my fade engine is driving; ignore the software frames
//         mChip.AbortFade();                 // a false frame ENDS any ramp, it does not only set a value
//         mChip.SetImmediate(x, y);          // discrete jump, or the exact endpoint that just settled
//     }
//
// THE ONE RULE: the newest instruction wins. The cluster holds exactly one movement at a time, so any
// start notification — on any axis, XY / CT / hue / saturation / color loop — replaces whatever was
// running, and so does any On*Changed with transitionActive == false. Abort the previous ramp when either
// arrives; there is no separate "cancelled" callback for a superseded movement (OnTransitionStopped is only
// for a stop with nothing taking over). Getting this wrong is the classic bug: the light keeps fading
// toward an abandoned target while the attributes report the new one.
//
// WHAT YOU DO NOT CONTROL: the cluster's tick. It keeps running whatever you do, because
// CurrentX/CurrentHue/RemainingTime, attribute reporting, scene capture and persistence depend on it, and
// those are data-model obligations rather than hardware ones. Overriding these methods saves your driver's
// per-frame work — bus traffic, PWM writes, color-space conversion — not the timer. Never suppress the tick
// by supplying a TimerDelegate that does not fire: it is also the cluster's clock, and starving it freezes
// every attribute mid-transition.
class ColorControlDelegate
{
public:
    virtual ~ColorControlDelegate() = default;

    // ---- Color mode conversion (§3.2.8.2) ----
    // Called when a command switches ColorMode (e.g. XY → HS) and whenever an inactive-mode attribute is
    // read. The cluster passes the current color; the delegate fills the requested representation into the
    // out-params. A device advertising both features must supply the conversion between them: the two
    // representations describe the same physical color, so a mapping always exists.

    virtual void ConvertXYToHueSat(uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat) {}
    virtual void ConvertHueSatToXY(uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds) {}
    virtual void ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertHueSatToMireds(uint8_t hue, uint8_t sat, uint16_t & outMireds) {}
    virtual void ConvertMiredsToHueSat(uint16_t mireds, uint8_t & outHue, uint8_t & outSat) {}

    // ---- Transition start notifications (§3.2.8) ----
    // Fired once, before the first tick, whenever a new movement is installed — by ANY origin: a command,
    // a scene restore, StartUpColorTemperature at boot, or CoupleColorTempToLevel. Hardware with its own
    // fade engine starts the ramp here and then ignores the per-tick On*Changed calls below (see
    // transitionActive), which is what lets a device run a transition without the software tick.
    //
    // These are notifications of intent, not a handover: the cluster ALWAYS runs its own software
    // transition in parallel, because CurrentX/CurrentHue/RemainingTime, reporting, scenes and persistence
    // are data-model obligations. Ignoring them is safe — a start notification carries only FOREKNOWLEDGE
    // (where the movement is going and how long it takes), never a value the On*Changed stream will not
    // deliver anyway — so a delegate overriding none of them behaves as one written before they existed.
    //
    // A movement of zero duration is NOT announced here. §3.2.8 makes a transition continuous and lasting
    // TransitionTime, so a TransitionTime of 0 is a discrete jump: it arrives as a single On*Changed with
    // transitionActive == false and must be applied immediately.
    //
    // Hue and saturation are independent axes (§3.2.5.2) and get separate notifications: either can start,
    // run and finish while the other is idle or still moving.

    // X and Y ramp independently — MoveColor takes a rate per axis, so the two can have different
    // durations and arrive at different times. An axis that is not moving has a duration of 0.
    virtual void OnXYTransitionStarted(uint16_t targetX, uint32_t durationXMs, uint16_t targetY, uint32_t durationYMs) {}

    virtual void OnCTTransitionStarted(uint16_t targetMireds, uint32_t durationMs) {}

    virtual void OnSaturationTransitionStarted(uint8_t targetSat, uint32_t durationMs) {}

    // Hue is circular, so a movement is a SIGNED ARC from wherever the axis is now, not a target: the sign
    // is the direction round the circle the cluster picked (shortest / longest / explicit per §3.2.8.4.5),
    // and hardware that resolves a target on its own may take the other way round. Always 16-bit enhanced
    // hue; legacy CurrentHue is its high byte.
    //
    // durationMs == kIndefiniteHueMoveMs (see ColorControlCluster.h) marks a MoveHue rate move: signedDelta
    // is then hue units per SECOND and the movement has no endpoint — it runs until OnTransitionStopped.
    virtual void OnEnhancedHueTransitionStarted(int32_t signedDelta, uint32_t durationMs) {}

    // ---- Color loop (§3.2.8.1) ----
    // The color loop is not a transition: it is a mode-independent driver that rotates EnhancedCurrentHue
    // around the full circle forever, and only ColorLoopSet(Deactivate) ends it. That makes it the strongest
    // case for hardware to own — a software loop keeps the tick alive indefinitely.
    //
    // These track ColorLoopSet: Activate starts the loop, Deactivate stops it. startEnhancedHue is the hue
    // the rotation begins from, loopTimeSec is the seconds for one full revolution (§3.2.7.16), and
    // directionUp is ColorLoopDirection. A second OnColorLoopStarted without an intervening stop supersedes
    // the first — ColorLoopSet(Activate) on a running loop re-anchors it.
    //
    // A running loop can also go DORMANT without being deactivated (§3.2.8.1): MoveToColor /
    // MoveToColorTemperature hand the output to XY / CT, and a hue command parks the loop. There is no
    // separate notification for that — each of those paths announces the movement that takes over, and by
    // THE ONE RULE that is what supersedes the rotation.
    virtual void OnColorLoopStarted(uint16_t startEnhancedHue, uint16_t loopTimeSec, bool directionUp) {}
    virtual void OnColorLoopStopped() {}

    // A movement was cut short with nothing replacing it, by an explicit stop: StopMoveStep, MoveHue(Stop),
    // MoveSaturation(Stop), MoveColor(Stop) or MoveColorTemp(Stop). Hardware running its own ramp MUST abort
    // it here — otherwise the cluster freezes its value while the light keeps fading and the two disagree
    // permanently. The frozen value is the last one already delivered via On*Changed.
    //
    // NOT fired in the two cases where something else already carries the news:
    //   * a movement reaching its endpoint — that ends with an On*Changed carrying the exact target and
    //     transitionActive == false;
    //   * a command superseding a movement in flight — see THE ONE RULE: the new start notification (or,
    //     for TransitionTime 0, the discrete On*Changed) is the replacement.
    virtual void OnTransitionStopped() {}

    // ---- Hardware output (PWM, LEDs, etc.) ----
    // §3.2.8 requires the physical movement to be continuous, so the cluster feeds the value on every tick.
    //
    // transitionActive describes the movement that produced THIS call: true means an intermediate frame of
    // an ongoing ramp, false means a value that must be applied as-is — a discrete jump, or the exact
    // endpoint of a movement that just settled. Hardware driving its own fade returns early on true; every
    // movement still ends with a false carrying the authoritative value, so any drift between the hardware
    // ramp and the cluster's interpolation is corrected when it settles.
    //
    // A false frame is also an instruction to STOP MOVING, so hardware running its own ramp must abort it
    // rather than only writing the value. That is how a TransitionTime of 0 arriving mid-movement
    // supersedes a hardware fade: a discrete jump produces no start notification and no
    // OnTransitionStopped, so this frame is the whole message.
    //
    // An active color loop (§3.2.8.1) never settles, so its frames are always transitionActive == true.
    //
    // OnColorHSChanged is the only saturation channel, so it carries a hue as well — but an 8-bit one.
    // When EnhancedHue is in play, OnEnhancedHueChanged is authoritative for hue and a delegate should
    // take only the saturation from OnColorHSChanged; its own transitionActive tracks the hue axis, which
    // §3.2.5.2 lets start and finish independently of saturation.
    virtual void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) {}
    virtual void OnColorHSChanged(uint8_t hue, uint8_t sat, bool transitionActive) {}
    virtual void OnColorCTChanged(uint16_t mireds, bool transitionActive) {}
    virtual void OnEnhancedHueChanged(uint16_t enhancedHue, bool transitionActive) {}
};

} // namespace Clusters
} // namespace app
} // namespace chip
