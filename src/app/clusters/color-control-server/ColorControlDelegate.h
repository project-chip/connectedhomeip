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
// Scope: only what the cluster cannot do itself and must invoke per call — color-space conversion and
// hardware output. Fixed values for the product go through StaticConfig instead (see
// ColorControlCluster::Config). Every method is handed what it needs by the cluster, delegate doesn't read state back
// from the cluster, though doing so is allowed.
// Convert* and the per-tick notifications run while an update is
// in progress, so attributes read from inside them may still hold the pre-update value.
//
// There are two ways to drive the light, and the cluster makes the same calls either way.
//
// 1. Override only the On*Changed methods, ignore transitionActive, and write every value. The cluster
//    interpolates each movement and feeds a value per tick, so those calls produce the fade:
//
//        void OnColorXYChanged(uint16_t x, uint16_t y, bool) override { WritePwm(x, y); }
//
// 2. If the LED driver, PWM controller or lighting IC ramps on its own, also override the
//    On*TransitionStarted methods to start the native ramp and skip the software frames. Both halves are
//    needed — the transitionActive == false frame is what ends the ramp, so it must abort the fade
//    ather than just write it
//
//        void OnXYTransitionStarted(uint16_t tX, uint32_t durXMs, uint16_t tY, uint32_t durYMs) override
//        {
//            mChip.Fade(tX, tY, std::max(durXMs, durYMs)); // convert the ENDPOINT once, not 30x
//        }
//
//        void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) override
//        {
//            if (transitionActive) { return; } // my fade engine is driving
//            mChip.AbortFade();
//            mChip.SetImmediate(x, y);
//        }
//
// THE ONE RULE: the newest instruction wins. The cluster holds exactly one movement at a time, so any start
// notification — on any axis, XY / CT / hue / saturation / color loop — replaces whatever was running, and so
// does any On*Changed with transitionActive == false. Abort the previous ramp when either arrives; there is
// no "cancelled" callback for a superseded movement (OnTransitionStopped is only for a stop with nothing
// taking over). Getting this wrong is the classic bug: the light keeps fading toward an abandoned target
// while the attributes report the new one.
//
// The cluster's tick will keep running, because CurrentX/CurrentHue/RemainingTime, reporting,
// scene capture and persistence depend on it. Overriding these methods saves your driver's per-frame work,
// not the timer. Never suppress the tick with a TimerDelegate that does not fire: it is also the cluster's
// clock, and starving it freezes every attribute mid-transition.
class ColorControlDelegate
{
public:
    virtual ~ColorControlDelegate() = default;

    // ---- Color mode conversion (§3.2.8.2) ----
    // Called when a command switches ColorMode (e.g. XY → HS) and whenever an inactive-mode attribute is
    // read: the cluster passes the current color, the delegate fills the requested representation into the
    // out-params. A device advertising both features must supply the conversion between them.

    virtual void ConvertXYToHueSat(uint16_t x, uint16_t y, uint8_t & outHue, uint8_t & outSat) {}
    virtual void ConvertHueSatToXY(uint8_t hue, uint8_t sat, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertXYToMireds(uint16_t x, uint16_t y, uint16_t & outMireds) {}
    virtual void ConvertMiredsToXY(uint16_t mireds, uint16_t & outX, uint16_t & outY) {}
    virtual void ConvertHueSatToMireds(uint8_t hue, uint8_t sat, uint16_t & outMireds) {}
    virtual void ConvertMiredsToHueSat(uint16_t mireds, uint8_t & outHue, uint8_t & outSat) {}

    // ---- Transition start notifications (§3.2.8) ----
    // Fired once, before the first tick, whenever a new movement is installed by ANY origin: a command, a
    // scene restore, StartUpColorTemperature at boot, or CoupleColorTempToLevel.
    //
    // They are notifications of intent, not a handover: the cluster ALWAYS runs its own software transition
    // in parallel for its data-model obligations, and they carry only FOREKNOWLEDGE (where the movement is
    // going and how long it takes), never a value the On*Changed stream will not deliver anyway — so a
    // delegate overriding none of them behaves as one written before they existed.
    //
    // A movement of zero duration is NOT announced here: per §3.2.8 a TransitionTime of 0 is a discrete jump
    // and arrives as a single On*Changed with transitionActive == false. Hue and saturation are independent
    // axes (§3.2.5.2) and get separate notifications.

    // X and Y ramp independently — MoveColor takes a rate per axis. An axis that is not moving has duration 0.
    virtual void OnXYTransitionStarted(uint16_t targetX, uint32_t durationXMs, uint16_t targetY, uint32_t durationYMs) {}

    virtual void OnCTTransitionStarted(uint16_t targetMireds, uint32_t durationMs) {}

    virtual void OnSaturationTransitionStarted(uint8_t targetSat, uint32_t durationMs) {}

    // Hue is circular, so a movement is a SIGNED ARC from wherever the axis is now, not a target: the sign is
    // the direction round the circle the cluster picked (§3.2.8.4.5), and hardware resolving a target on its
    // own may take the other way round. Always 16-bit enhanced hue; legacy CurrentHue is its high byte.
    // durationMs == kIndefiniteHueMoveMs (see ColorControlCluster.h) marks a MoveHue rate move: signedDelta is
    // then hue units per SECOND and the movement has no endpoint — it runs until OnTransitionStopped.
    virtual void OnEnhancedHueTransitionStarted(int32_t signedDelta, uint32_t durationMs) {}

    // ---- Color loop (§3.2.8.1) ----
    // Not a transition: a mode-independent rotation of EnhancedCurrentHue around the full circle that only
    // ColorLoopSet(Deactivate) ends, which makes it the strongest case for hardware to own — a software loop
    // keeps the tick alive indefinitely. startEnhancedHue is the hue the rotation begins from, loopTimeSec the
    // seconds for one revolution (§3.2.7.16), directionUp the ColorLoopDirection. A second start without an
    // intervening stop re-anchors a running loop. A loop can also go DORMANT without being deactivated —
    // MoveToColor / MoveToColorTemperature or a hue command park it — with no separate notification: by THE
    // ONE RULE, the movement that takes over is what supersedes the rotation.
    virtual void OnColorLoopStarted(uint16_t startEnhancedHue, uint16_t loopTimeSec, bool directionUp) {}
    virtual void OnColorLoopStopped() {}

    // A movement was cut short with nothing replacing it, by an explicit stop: StopMoveStep, MoveHue(Stop),
    // MoveSaturation(Stop), MoveColor(Stop) or MoveColorTemp(Stop). Hardware running its own ramp MUST abort
    // it here — otherwise the cluster freezes its value (the last one delivered via On*Changed) while the
    // light keeps fading, and the two disagree permanently. NOT fired when a movement reaches its endpoint
    // (an On*Changed carries the exact target with transitionActive == false) or when a command supersedes
    // one in flight (THE ONE RULE: the new start notification is the replacement).
    virtual void OnTransitionStopped() {}

    // ---- Hardware output (PWM, LEDs, etc.) ----
    // §3.2.8 requires the physical movement to be continuous, so the cluster feeds a value on every tick.
    //
    // transitionActive describes the movement that produced THIS call: true is an intermediate frame of an
    // ongoing ramp, false a value that must be applied as-is — a discrete jump, or the exact endpoint of a
    // movement that just settled. Hardware driving its own fade returns early on true; every movement still
    // ends with a false carrying the authoritative value, so drift between the hardware ramp and the cluster's
    // interpolation is corrected when it settles. A false frame is also an instruction to STOP MOVING: it is
    // the whole message for a TransitionTime of 0 arriving mid-movement, which produces no start notification
    // and no OnTransitionStopped. An active color loop never settles, so its frames are always true.
    //
    // OnColorHSChanged is the only saturation channel, so it carries a hue as well — but an 8-bit one. When
    // EnhancedHue is in play, OnEnhancedHueChanged is authoritative for hue and a delegate should take only
    // the saturation from OnColorHSChanged; its own transitionActive tracks the hue axis, which §3.2.5.2 lets
    // start and finish independently of saturation.
    virtual void OnColorXYChanged(uint16_t x, uint16_t y, bool transitionActive) {}
    virtual void OnColorHSChanged(uint8_t hue, uint8_t sat, bool transitionActive) {}
    virtual void OnColorCTChanged(uint16_t mireds, bool transitionActive) {}
    virtual void OnEnhancedHueChanged(uint16_t enhancedHue, bool transitionActive) {}
};

} // namespace Clusters
} // namespace app
} // namespace chip
