import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12
import VCStyles 1.0

import com.benprisby.vc.vchub 1.0

Tile {
    id: root

    signal played()

    ListView {
        id: playlistsList
        anchors.fill: parent
        anchors.margins: VCMargin.small
        spacing: VCMargin.small
        cacheBuffer: Math.max( 0, contentHeight )
        interactive: contentHeight > height
        ScrollBar.vertical: ScrollBar { policy: playlistsList.interactive ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff }
        model: VCHub.spotify.playlists
        delegate: RowLayout {
            width: parent.width
            height: 50
            spacing: VCMargin.medium

            Image {
                id: playlistImage
                Layout.fillHeight: true
                Layout.preferredWidth: height
                fillMode: Image.PreserveAspectFit
                source: modelData[ "image" ]
            }

            Text {
                id: playlistName
                Layout.fillWidth: true
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: VCFont.body
                color: VCColor.white
                text: modelData[ "name" ]
            }

            Text {
                id: trackCount
                Layout.fillHeight: true
                verticalAlignment: Text.AlignVCenter
                font.pixelSize: VCFont.label
                color: VCColor.white
                text: modelData[ "trackCount" ] + qsTr( " Tracks" )
            }

            Image {
                id: publicPrivateIndicator
                Layout.preferredWidth: 20
                Layout.preferredHeight: width
                Layout.alignment: Qt.AlignVCenter
                sourceSize: Qt.size( Layout.preferredWidth, Layout.preferredHeight )
                source: modelData[ "isPublic" ] ? "qrc:/images/globe.svg" : "qrc:/images/padlock.svg"
            }

            VCButton {
                id: playButton
                Layout.preferredWidth: 80
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: playlistsList.interactive ? VCMargin.medium : 0  // Leave room for the scrollbar
                iconSource: "qrc:/images/play.svg"
                enabled: VCHub.spotify.playlistName !== modelData[ "name" ]

                onClicked: {
                    VCHub.spotify.play( modelData[ "uri" ] )
                    root.played()
                }
            }
        }
    }
}
