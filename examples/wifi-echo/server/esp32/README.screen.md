# Simple Screen UI Framework

## Overview

This is a framework for creating simple user interfaces for devices with tiny
screens and just a few buttons.

For example, the [M5Stack](http://m5stack.com/) ESP32 Basic Core IoT device has
a 320x240 TFT display and three push buttons.

This framework enables a UI such as the following, where focus is indicated by
color, the left and middle buttons function as a single axis navpad, the right
button function as an action button, and the back button is provided by the
framework onscreen:

    +--------------------------------------+
    |  <  SCREEN TITLE                     |
    |     ------------                     |
    |                                      |
    |     ITEM 1                           |
    |     ITEM 2                           |
    |     ITEM 3                           |
    |                                      |
    |                                      |
    |                                      |
    |                                      |
    |   PREVIOUS      NEXT       ACTION    |
    +--------------------------------------+

## Features

-   stack of screens with system back button
-   labeled buttons for navigation and action
-   focus handling
-   custom screen content
-   virtual LEDs (VLEDs)

## Screen Manager

The screen manager maintains a stack of screens that are pushed and popped for
interaction. Each is drawn with a title, system back button, three button
labels, and custom screen content. The screen manager dispatches events to the
topmost screen, such as:

-   enter/exit events (including whether the screen was pushed or popped)
-   focus events (for navigation)
-   action events
-   display events (lazily)

If configured, virtual LEDs (VLEDs) are omnipresent and can be toggled at will.

## Screen

Screens provide a title and three button labels to the screen manager. They
handle events from the screen manager, and cooperate to ensure focus behaves
nicely.

## Focus Handling

Screens are created without focus, and indicate to the screen manager whether
they can receive focus. If so, the screen manager focuses them when they are
pushed, and subsequently dispatches focus next/previous events which the screen
can use for navigation before any action is performed.

The screen can request the screen manager to focus the system back button, which
is always available except when there are no covered screens on the stack. In
this way, a list screen can wrap its focus of list items through the system back
button if it is available, while skipping it otherwise.

Blur and unblur focus events allow a screen's focus state to be preserved when
it is covered by a pushed screen and restored when it is subsequently uncovered.

In summary, screens collaborate with the screen manager to handle focus via the
following focus event types:

-   NONE: remove focus from screen completely
-   BLUR: unfocus screen (but retain focus state)
-   UNBLUR: restore focus state (that was retained when blurred)
-   NEXT: navigate focus forward in screen (possibly requesting focus back
    button)
-   PREVIOUS: navigate focus forward (possibly requesting back button focus)

## Typical Usage

A list screen can provide multiple options to the user. Each list item can push
another screen on the stack with more items. In this way a hierarchy of screens
can be formed.

The back button dismisses a screen much as "escape" or "cancel" would. It's
possible to push an informational screen that is not focusable and has no
interaction, such that the only action available is back.
