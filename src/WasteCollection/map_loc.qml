import QtQuick 2.15
import QtQuick.Controls 2.15
import QtPositioning 5.15
import QtLocation 5.15

Item {
    id: root
    //width: parent.width
    //height: parent.height
    anchors.fill: parent

    property double markerLat: 33.8869
    property double markerLng: 9.5375
    property bool   markerPlaced: false

    // Limites de la région (rayon autorisé en degrés ~20km)
    property double centerLat: markerLat
    property double centerLng: markerLng
    property double maxDelta: 0.3  // ~30km autour du centre

    signal locationPicked(double lat, double lng)

    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.host"
            value: "https://tile.openstreetmap.org/"
        }
        PluginParameter {
            name: "osm.mapping.copyright"
            value: "© OpenStreetMap contributors"
        }
    }

    Map {
        id: map
        anchors.fill: parent
        plugin: mapPlugin
        center: QtPositioning.coordinate(root.centerLat, root.centerLng)
        zoomLevel: 12

        // Bloquer le zoom min/max
        minimumZoomLevel: 11
        maximumZoomLevel: 17

        // Bloquer le pan en dehors de la région
        onCenterChanged: {
            var lat = map.center.latitude
            var lng = map.center.longitude

            var clampedLat = Math.max(root.centerLat - root.maxDelta,
                             Math.min(root.centerLat + root.maxDelta, lat))
            var clampedLng = Math.max(root.centerLng - root.maxDelta,
                             Math.min(root.centerLng + root.maxDelta, lng))

            if (lat !== clampedLat || lng !== clampedLng) {
                map.center = QtPositioning.coordinate(clampedLat, clampedLng)
            }
        }

        // Zoom avec la molette
        WheelHandler {
            onWheel: function(event) {
                if (event.angleDelta.y > 0)
                    map.zoomLevel = Math.min(map.zoomLevel + 0.5, map.maximumZoomLevel)
                else
                    map.zoomLevel = Math.max(map.zoomLevel - 0.5, map.minimumZoomLevel)
            }
        }

        // Marker point rouge
        MapQuickItem {
            id: markerItem
            visible: root.markerPlaced
            coordinate: QtPositioning.coordinate(root.markerLat, root.markerLng)
            anchorPoint.x: pointMarker.width  / 2
            anchorPoint.y: pointMarker.height / 2

            sourceItem: Item {
                width: 24
                height: 24

                Rectangle {
                    id: pointMarker
                    width: 16
                    height: 16
                    radius: 8
                    color: "#CC1F497D"
                    border.color: "white"
                    border.width: 2
                    anchors.centerIn: parent

                    // Animation pulse
                    SequentialAnimation on scale {
                        running: root.markerPlaced
                        loops: Animation.Infinite
                        NumberAnimation { to: 1.3; duration: 600; easing.type: Easing.InOutQuad }
                        NumberAnimation { to: 1.0; duration: 600; easing.type: Easing.InOutQuad }
                    }
                }

                // Cercle extérieur
                Rectangle {
                    width: 24
                    height: 24
                    radius: 12
                    color: "transparent"
                    border.color: "#661F497D"
                    border.width: 2
                    anchors.centerIn: parent
                }
            }
        }

        // Clic sur la carte
        // Navigation + Marqueur
        // Navigation + Marqueur
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            property point lastPos

            // Clic droit → placer le marqueur de municipalité
            onClicked: function(mouse) {
                if (mouse.button === Qt.RightButton) {
                    var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y))
                    var lat   = coord.latitude
                    var lng   = coord.longitude

                    if (Math.abs(lat - root.centerLat) > root.maxDelta ||
                        Math.abs(lng - root.centerLng) > root.maxDelta) {
                        return
                    }

                    root.markerLat    = lat
                    root.markerLng    = lng
                    root.markerPlaced = true
                    root.locationPicked(lat, lng)
                }
            }

            // Clic gauche maintenu → déplacer la carte (pan)
            onPressed: function(mouse) {
                if (mouse.button === Qt.LeftButton) {
                    lastPos = Qt.point(mouse.x, mouse.y)
                }
            }

            onPositionChanged: function(mouse) {
                if (pressedButtons & Qt.LeftButton) {
                    var dx = mouse.x - lastPos.x
                    var dy = mouse.y - lastPos.y
                    lastPos = Qt.point(mouse.x, mouse.y)
                    map.pan(-dx, -dy)
                }
            }
        }    }

    // Texte d'instruction en haut
    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 10
        color: "#CC1F497D"
        radius: 8
        width: instructionText.width + 20
        height: 36

        Text {
            id: instructionText
            anchors.centerIn: parent
            text: root.markerPlaced
                  ? "📍 Location selected — click Confirm"
                  : "Right Click on the map to place a marker"
            color: "white"
            font.pixelSize: 13
            font.bold: true
        }
    }
}
