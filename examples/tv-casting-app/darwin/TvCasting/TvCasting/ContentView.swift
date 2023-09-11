/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import SwiftUI

struct ContentView: View {
    var body: some View {
        NavigationView {
            if ProcessInfo.processInfo.environment["CHIP_CASTING_SIMPLIFIED"] == "0"
            {
                StartFromCacheView()
            }
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
