import QtQuick 2.15
import QtQuick.Controls 2.15
import QtLocation 6.0
import QtPositioning 6.0

Rectangle {
    id: root
    anchors.fill: parent
    color: "#1a1a2e"

    property var    municipalityModel: []
    property string searchQuery:       ""
    property string riskFilter:        "all"

    // ── Taille du point selon le zoom ─────────────────────────
    function pointRadius(zoomLevel) {
        if (zoomLevel >= 14) return 200
        if (zoomLevel >= 12) return 400
        if (zoomLevel >= 10) return 600
        if (zoomLevel >= 8)  return 900
        return 1200
    }

    // ── Recherche vide → vue initiale ─────────────────────────
    onSearchQueryChanged: {
        var q = searchQuery.toLowerCase().trim()
        if (!q) {
            map.center    = QtPositioning.coordinate(35.5,9.5)
            map.zoomLevel = 7
            return
        }
        var num = parseInt(q)
        for (var i = 0; i < municipalityModel.length; i++) {
            var d = municipalityModel[i]
            if ((!isNaN(num) && d.id === num) ||
                 d.name.toLowerCase().indexOf(q) !== -1) {
                map.center    = QtPositioning.coordinate(d.lat, d.lng)
                map.zoomLevel = 14
                break
            }
        }
    }

    // ── Filtre risque ─────────────────────────────────────────
    onRiskFilterChanged: {
        var updated = []
        for (var i = 0; i < municipalityModel.length; i++) {
            var d = municipalityModel[i]
            var copy = Object.assign({}, d)
            if (riskFilter === "all") {
                copy.haloVisible = true
            } else if (riskFilter === "aucun") {
                copy.haloVisible = false
            } else {
                copy.haloVisible = (copy.level === riskFilter)
            }
            updated.push(copy)
        }
        municipalityModel = updated
    }

    // ── Carte OpenStreetMap ───────────────────────────────────
    Map {
        id: map
        anchors.fill: parent
        zoomLevel: 7
        center: QtPositioning.coordinate(35.5,9.5)

        // ── Déplacement à la souris ───────────────────────────
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton
            property real lastX: 0
            property real lastY: 0

            onPressed: function(mouse) {
                lastX = mouse.x
                lastY = mouse.y
            }

            onPositionChanged: function(mouse) {
                if (pressed) {
                    var dx = mouse.x - lastX
                    var dy = mouse.y - lastY
                    var newCenter = map.toCoordinate(
                        Qt.point(map.width/2 - dx, map.height/2 - dy))
                    var lat = Math.max(30.2, Math.min(37.5, newCenter.latitude))
                    var lng = Math.max(7.5,  Math.min(11.6, newCenter.longitude))
                    map.center = QtPositioning.coordinate(lat, lng)
                    lastX = mouse.x
                    lastY = mouse.y
                }
            }

            onWheel: function(wheel) {
                var delta = wheel.angleDelta.y > 0 ? 0.5 : -0.5
                map.zoomLevel = Math.max(6, Math.min(16, map.zoomLevel + delta))
            }
        }

        plugin: Plugin {
            name: "osm"
            PluginParameter {
                    name: "osm.mapping.providersrepository.disabled"
                    value: true
                }
            PluginParameter {
                name: "osm.mapping.host"
                value: "https://tile.openstreetmap.org/"
            }
            PluginParameter {
                    name: "osm.useragent"
                    value: "QtApp"
                }
        }

        // ── 1. Halos heatmap ──────────────────────────────────
        MapItemView {
            model: root.municipalityModel
            delegate: MapCircle {
                center: QtPositioning.coordinate(
                    modelData.lat, modelData.lng
                )
                radius: Math.max(modelData.radius, 800)
                color: getColor(modelData.level, 0.5)
                border.color: getColor(modelData.level, 0.8)
                border.width: 0
                opacity: 1
                visible: modelData.haloVisible !== false &&
                         riskFilter !== "aucun"
            }
        }

        // ── 2. Zone survol invisible ──────────────────────────
        MapItemView {
            model: root.municipalityModel
            delegate: MapCircle {
                center: QtPositioning.coordinate(
                    modelData.lat, modelData.lng
                )
                radius: Math.max(modelData.radius, 800)
                color: "transparent"
                border.width: 0
                visible: true

                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: {
                        tooltip.mdata   = modelData
                        tooltip.visible = true
                    }
                    onExited: {
                        tooltip.visible = false
                    }
                }
            }
        }

        // ── 3. Points roses toujours au dessus ────────────────
        MapItemView {
            model: root.municipalityModel
            delegate: MapQuickItem {
                coordinate: QtPositioning.coordinate(
                    modelData.lat, modelData.lng
                )
                anchorPoint.x: sourceItem.width  / 2
                anchorPoint.y: sourceItem.height / 2
                zoomLevel: 0

                sourceItem: Rectangle {
                    width:  10
                    height: 10
                    radius: 5
                    color:  "#C2185B"
                    border.color: "white"
                    border.width: 1
                }
            }
        }
    }

    // ── Tooltip survol ────────────────────────────────────────
    Rectangle {
        id: tooltip
        visible: false
        width: 230
        height: tooltipCol.implicitHeight + 20
        radius: 10
        color: "#E8101019"
        border.color: "#33ffffff"
        border.width: 1
        x: 20
        y: 20
        z: 100

        property var mdata: null

        Column {
            id: tooltipCol
            anchors {
                left: parent.left
                top: parent.top
                margins: 10
            }
            spacing: 5

            Text {
                text: tooltip.mdata ? tooltip.mdata.name : ""
                font.bold: true
                font.pixelSize: 14
                color: "white"
            }
            Text {
                text: tooltip.mdata
                      ? "Region: " + tooltip.mdata.region : ""
                font.pixelSize: 12
                color: "#cccccc"
            }
            Text {
                text: tooltip.mdata
                      ? "Address: " + tooltip.mdata.address : ""
                font.pixelSize: 12
                color: "#cccccc"
                wrapMode: Text.WordWrap
                width: 210
            }
            Text {
                text: tooltip.mdata
                      ? "📞 " + tooltip.mdata.phone : ""
                font.pixelSize: 12
                color: "#cccccc"
            }
            Text {
                text: tooltip.mdata
                      ? "✉️ " + tooltip.mdata.email : ""
                font.pixelSize: 12
                color: "#cccccc"
            }
            Text {
                text: tooltip.mdata
                      ? "Flood risk: " + tooltip.mdata.risk + "%"
                      : ""
                font.pixelSize: 12
                font.bold: true
                color: tooltip.mdata
                       ? getColor(tooltip.mdata.level, 1.0)
                       : "white"
            }
            Rectangle {
                width:  badgeText.implicitWidth + 16
                height: badgeText.implicitHeight + 8
                radius: 10
                color: tooltip.mdata
                       ? getBadgeColor(tooltip.mdata.level)
                       : "#7f8c8d"
                Text {
                    id: badgeText
                    anchors.centerIn: parent
                    text: tooltip.mdata
                          ? getLabel(tooltip.mdata.level) : ""
                    font.pixelSize: 11
                    font.bold: true
                    color: "white"
                }
            }
        }
    }

    // ── Légende ───────────────────────────────────────────────
    Rectangle {
        anchors {
            right: parent.right
            bottom: parent.bottom
            margins: 12
        }
        width: 185
        height: legendCol.implicitHeight + 20
        radius: 10
        color: "#E8101019"
        border.color: "#33ffffff"
        border.width: 1
        z: 100

        Column {
            id: legendCol
            anchors { fill: parent; margins: 10 }
            spacing: 6

            Text {
                text: "Flood level"
                color: "white"
                font.bold: true
                font.pixelSize: 13
            }
            Repeater {
                model: [
                    { label: "Critical (>60%)",  col: "#e74c3c" },
                    { label: "High (40-60%)",     col: "#f39c12" },
                    { label: "Moderate (25-40%)", col: "#3498db" },
                    { label: "Low (<25%)",        col: "#2ecc71" },
                    { label: "None",              col: "#95a5a6" }
                ]
                Row {
                    spacing: 8
                    Rectangle {
                        width: 12; height: 12; radius: 6
                        color: modelData.col
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Text {
                        text: modelData.label
                        color: "#dddddd"
                        font.pixelSize: 11
                    }
                }
            }
        }
    }

    // ── Fonctions utilitaires ─────────────────────────────────
    function getColor(level, opacity) {
        var l = level ? level.toString() : "aucun"
        if (l === "critique") return Qt.rgba(0.91, 0.30, 0.24, opacity)
        if (l === "eleve")    return Qt.rgba(0.95, 0.61, 0.07, opacity)
        if (l === "modere")   return Qt.rgba(0.20, 0.60, 0.86, opacity)
        if (l === "faible")   return Qt.rgba(0.18, 0.80, 0.44, opacity)
        return Qt.rgba(0.58, 0.65, 0.65, opacity)
    }

    function getBadgeColor(level) {
        var l = level ? level.toString() : "aucun"
        if (l === "critique") return "#c0392b"
        if (l === "eleve")    return "#d35400"
        if (l === "modere")   return "#2980b9"
        if (l === "faible")   return "#27ae60"
        return "#7f8c8d"
    }

    function getLabel(level) {
        var l = level ? level.toString() : "aucun"
        if (l === "critique") return "Critical (>60%)"
        if (l === "eleve")    return "High (40-60%)"
        if (l === "modere")   return "Moderate (25-40%)"
        if (l === "faible")   return "Low (<25%)"
        return "None"
    }

    // ── Réinitialisation vue ──────────────────────────────
    function resetView() {
        map.center    = QtPositioning.coordinate(35.5, 9.5)
        map.zoomLevel = 7
    }
}
