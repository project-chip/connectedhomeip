/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package com.google.chip.chiptool

import android.content.Intent
import android.widget.EditText
import androidx.test.espresso.Espresso.onView
import androidx.test.espresso.action.ViewActions.click
import androidx.test.espresso.action.ViewActions.replaceText
import androidx.test.espresso.assertion.ViewAssertions.matches
import androidx.test.espresso.matcher.RootMatchers.isDialog
import androidx.test.espresso.matcher.ViewMatchers.isDisplayed
import androidx.test.espresso.matcher.ViewMatchers.withId
import androidx.test.espresso.matcher.ViewMatchers.withText
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import androidx.test.rule.GrantPermissionRule
import org.hamcrest.Matchers.equalToIgnoringCase
import org.junit.Assert
import org.junit.Rule
import org.junit.Test
import org.junit.runner.RunWith

/**
 * Instrumented test, which will execute on an Android device.
 */
@RunWith(AndroidJUnit4::class)
class OnOffUiTest {
  @get:Rule
  val permissionRule: GrantPermissionRule = GrantPermissionRule.grant(
    android.Manifest.permission.ACCESS_FINE_LOCATION,
    android.Manifest.permission.CAMERA
  )

  private lateinit var activity: CHIPToolActivity

  @Test
  fun commissionAndControl() {
    // Context of the app under test.
    val appContext = InstrumentationRegistry.getInstrumentation().targetContext
    Assert.assertEquals("com.google.chip.chiptool", appContext.packageName)

    val intent = Intent(appContext, CHIPToolActivity::class.java)
    intent.flags = Intent.FLAG_ACTIVITY_NEW_TASK
    activity =
      InstrumentationRegistry.getInstrumentation().startActivitySync(intent) as CHIPToolActivity

    // A "Waited for the root of the view hierarchy to have window focus" error can occur
    // if a system dialog is shown, so make sure we close those.
    activity.sendBroadcast(Intent(Intent.ACTION_CLOSE_SYSTEM_DIALOGS));

    commissionDevice()
    performOnOffControl()
  }

  /**
   * Starting from the home screen (SelectActionFragment), perform IP commissioning for device, then
   * return to home screen and dismiss success dialog.
   */
  private fun commissionDevice() {
    onView(withId(R.id.provisionThreadCredentialsBtn)).perform(click())
    onView(withId(R.id.inputAddressBtn)).perform(click())
    val ipAddress = InstrumentationRegistry.getArguments().getString("endDeviceIpAddress")
    onView(withId(R.id.addressEditText)).perform(replaceText(ipAddress))
    onView(withText("Commission")).perform(click())

    onView(withText("Commissioning completed with result: 0"))
      .inRoot(isDialog())
      .check(matches(isDisplayed()))

    // Dismiss dialog
    onView(withText(equalToIgnoringCase("Ok"))).inRoot(isDialog()).perform(click())
  }

  /**
   * Starting from the home screen (SelectActionFragment), go to the on/off fragment, update the
   * address, and perform an ON command.
   */
  private fun performOnOffControl() {
    onView(withId(R.id.onOffClusterBtn)).perform(click())
    onView(withText(equalToIgnoringCase("Update address"))).perform(click())

    val nodeId = activity.findViewById<EditText>(R.id.deviceIdEd).text.toString().toULong().toLong()
    onView(withText("Address update complete for nodeId $nodeId with code 0")).check(
      matches(
        isDisplayed()
      )
    )

    onView(withId(R.id.onBtn)).perform(click())
    onView(withText("ON command success")).check(matches(isDisplayed()))
  }
}
