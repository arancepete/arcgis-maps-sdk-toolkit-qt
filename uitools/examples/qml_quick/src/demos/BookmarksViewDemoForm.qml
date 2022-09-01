/*******************************************************************************
 *  Copyright 2012-2022 Esri
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ******************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12
import Esri.ArcGISRuntime
import Esri.ArcGISRuntime.Toolkit

DemoPage {
    mapViewContents: Component {
        MapView {
            id: view

            Map {
                PortalItem {
                   itemId: "16f1b8ba37b44dc3884afc8d5f454dd2"
                }
            }

            BookmarksView {
                id: bookmarksView
                geoView: view
                anchors {
                    left: parent.left
                    top: parent.top
                    margins: 10
                }
            }
        }
    }
}
