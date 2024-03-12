/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

import QGroundControl
import QGroundControl.ScreenTools
import QGroundControl.Palette
import QGroundControl.Controls
import QGroundControl.FlightMap

/// GeoFence map visuals
Item {
    id: _root
    z: QGroundControl.zOrderMapItems

    property var    map
    property var    myGeoFenceController
    property bool   interactive:            false   ///< true: user can interact with items
    property bool   planView:               false   ///< true: visuals showing in plan view
    property var    homePosition

    property var    _breachReturnPointComponent
    property var    _breachReturnDragComponent
    property var    _paramCircleFenceComponent
    property var    _polygons:                  myGeoFenceController.polygons
    property var    _circles:                   myGeoFenceController.circles
    property color  _borderColor:               "orange"
    property int    _borderWidthInclusion:      2
    property int    _borderWidthExclusion:      0
    property color  _interiorColorExclusion:    "orange"
    property color  _interiorColorInclusion:    "transparent"
    property real   _interiorOpacityExclusion:  0.2 * opacity
    property real   _interiorOpacityInclusion:  1 * opacity

    Component.onCompleted: {
        _breachReturnPointComponent = breachReturnPointComponent.createObject(map)
        map.addMapItem(_breachReturnPointComponent)
        _breachReturnDragComponent = breachReturnDragComponent.createObject(map, { "itemIndicator": _breachReturnPointComponent })
        _paramCircleFenceComponent = paramCircleFenceComponent.createObject(map)
        map.addMapItem(_paramCircleFenceComponent)
    }

    Component.onDestruction: {
        _breachReturnPointComponent.destroy()
        _breachReturnDragComponent.destroy()
        _paramCircleFenceComponent.destroy()
    }

    // By default the parent for Instantiator.delegate item is the Instatiator itself. By there is a bug
    // in Qt which will cause a crash if this delete item has Menu item within it. Since the Menu item
    // doesn't like having a non-visual item as parent. This is likely related to hybrid QQuickWidtget+QML
    // Hence Qt folks are going to care. In order to workaround you have to parent the item to _root Item instead.
    Instantiator {
        model: _polygons

        delegate : QGCMapPolygonVisuals {
            parent:             _root
            mapControl:         map
            mapPolygon:         object
            borderWidth:        object.inclusion ? _borderWidthInclusion : _borderWidthExclusion
            borderColor:        _borderColor
            interiorColor:      object.inclusion ? _interiorColorInclusion : _interiorColorExclusion
            interiorOpacity:    object.inclusion ? _interiorOpacityInclusion : _interiorOpacityExclusion
            interactive:        _root.interactive && mapPolygon && mapPolygon.interactive
        }
    }
    Instantiator {
        model: _polygons

        delegate : QGCMapPolygonVisuals {
            parent:             _root
            mapControl:         map
            mapPolygon:         object.groundBuffer
            borderWidth:        _borderWidthInclusion
            borderColor:        'red'
            interactive:        false
        }
    }
    Instantiator {
        model: _polygons

        delegate : QGCMapPolygonVisuals {
            parent:             _root
            mapControl:         map
            mapPolygon:         object.contingencyZone
            borderWidth:        _borderWidthInclusion
            borderColor:        'blue'
            interactive:        false
        }
    }

    Instantiator {
        model: _circles

        delegate : QGCMapCircleVisuals {
            parent:             _root
            mapControl:         map
            mapCircle:          object
            borderWidth:        object.inclusion ? _borderWidthInclusion : _borderWidthExclusion
            borderColor:        _borderColor
            interiorColor:      object.inclusion ? _interiorColorInclusion : _interiorColorExclusion
            interiorOpacity:    object.inclusion ? _interiorOpacityInclusion : _interiorOpacityExclusion
            interactive:        _root.interactive && mapCircle && mapCircle.interactive
        }
    }
    Instantiator {
        model: _circles

        delegate : QGCMapCircleVisuals {
            parent:             _root
            mapControl:         map
            mapCircle:          object.groundBuffer
            borderWidth:        _borderWidthInclusion
            borderColor:        'red'
            interactive:        false
        }
    }
    Instantiator {
        model: _circles

        delegate : QGCMapCircleVisuals {
            parent:             _root
            mapControl:         map
            mapCircle:          object.contingencyZone
            borderWidth:        _borderWidthInclusion
            borderColor:        'blue'
            interactive:        false
        }
    }

    // Circular geofence specified from parameter
    Component {
        id: paramCircleFenceComponent

        MapCircle {
            color:          _interiorColorInclusion
            opacity:        _interiorOpacityInclusion
            border.color:   _borderColor
            border.width:   _borderWidthInclusion
            center:         homePosition
            radius:         _radius
            visible:        homePosition.isValid && _radius > 0

            property real _radius: myGeoFenceController.paramCircularFence

            on_RadiusChanged: console.log("_radius", _radius, homePosition.isValid, homePosition)
        }
    }

    Component {
        id: breachReturnDragComponent

        MissionItemIndicatorDrag {
            mapControl:     map
            itemCoordinate: myGeoFenceController.breachReturnPoint
            visible:        _root.interactive

            onItemCoordinateChanged: myGeoFenceController.breachReturnPoint = itemCoordinate
        }
    }


    // Breach return point
    Component {
        id: breachReturnPointComponent

        MapQuickItem {
            anchorPoint.x:  sourceItem.anchorPointX
            anchorPoint.y:  sourceItem.anchorPointY
            z:              QGroundControl.zOrderMapItems
            coordinate:     myGeoFenceController.breachReturnPoint
            opacity:        _root.opacity

            sourceItem: MissionItemIndexLabel {
                label:      qsTr("B", "Breach Return Point item indicator")
                checked:    true
            }
        }
    }
}
