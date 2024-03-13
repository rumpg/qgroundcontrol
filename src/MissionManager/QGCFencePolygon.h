/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QGCMapPolygon.h"

class Vehicle;

/// The QGCFencePolygon class provides a polygon used by GeoFence support.
class QGCFencePolygon : public QGCMapPolygon
{
    Q_OBJECT

public:
    QGCFencePolygon(bool inclusion, Vehicle* vehicle, QObject* parent = nullptr);
    QGCFencePolygon(const QGCFencePolygon& other, QObject* parent = nullptr);

    const QGCFencePolygon& operator=(const QGCFencePolygon& other);

    Q_PROPERTY(bool                 inclusion       READ inclusion          WRITE setInclusion  NOTIFY inclusionChanged)
    Q_PROPERTY(const QGCMapPolygon* groundBuffer    READ groundBuffer                           CONSTANT)
    Q_PROPERTY(const QGCMapPolygon* contingencyZone READ contingencyZone                        CONSTANT)

    /// Saves the QGCFencePolygon to the json object.
    ///     @param json Json object to save to
    void saveToJson(QJsonObject& json);

    /// Load a QGCFencePolygon from json
    ///     @param json Json object to load from
    ///     @param required true: no polygon in object will generate error
    ///     @param errorString Error string if return is false
    /// @return true: success, false: failure (errorString set)
    bool loadFromJson(const QJsonObject& json, bool required, QString& errorString);

    void setVehicle(const Vehicle* vehicle);

    // Property methods

    bool inclusion (void) const { return _inclusion; }
    void setInclusion (bool inclusion);
    const QGCMapPolygon* groundBuffer (void) const { return &_groundBuffer; }
    const QGCMapPolygon* contingencyZone (void) const { return &_contingencyZone; }

signals:
    void inclusionChanged       (bool inclusion);

private slots:
    void _setDirty(void);
    void _updateContingencyZone(void);
    void _updateGroundBuffer(void);

private:
    void _init(void);

    bool _inclusion;
    const Vehicle* _vehicle;
    double _fenceMargin;
    double _groundBufferMargin;
    QGCMapPolygon _contingencyZone;
    QGCMapPolygon _groundBuffer;

    static const char* _jsonInclusionKey;

    static const int _jsonCurrentVersion = 1;
};
