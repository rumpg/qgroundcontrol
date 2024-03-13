/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#pragma once

#include "QGCMapCircle.h"

class Vehicle;

/// The QGCFenceCircle class provides a cicle used by GeoFence support.
class QGCFenceCircle : public QGCMapCircle
{
    Q_OBJECT

public:
    QGCFenceCircle(Vehicle* vehicle, QObject* parent = nullptr);
    QGCFenceCircle(const QGeoCoordinate& center, double radius, bool inclusion, Vehicle* vehicle, QObject* parent = nullptr);
    QGCFenceCircle(const QGCFenceCircle& other, QObject* parent = nullptr);

    const QGCFenceCircle& operator=(const QGCFenceCircle& other);

    Q_PROPERTY(bool                 inclusion       READ inclusion          WRITE setInclusion  NOTIFY inclusionChanged)
    Q_PROPERTY(const QGCMapCircle*  groundBuffer    READ groundBuffer                           CONSTANT)
    Q_PROPERTY(const QGCMapCircle*  contingencyZone READ contingencyZone                        CONSTANT)

    /// Saves the QGCFenceCircle to the json object.
    ///     @param json Json object to save to
    void saveToJson(QJsonObject& json);

    /// Load a QGCFenceCircle from json
    ///     @param json Json object to load from
    ///     @param errorString Error string if return is false
    /// @return true: success, false: failure (errorString set)
    bool loadFromJson(const QJsonObject& json, QString& errorString);

    void setVehicle(const Vehicle* vehicle);

    // Property methods

    bool inclusion (void) const { return _inclusion; }
    void setInclusion (bool inclusion);
    const QGCMapCircle* groundBuffer (void) const { return &_groundBuffer; }
    const QGCMapCircle* contingencyZone (void) const { return &_contingencyZone; }

signals:
    void inclusionChanged(bool inclusion);

private slots:
    void _setDirty(void);
    void _updateMarginZoneRadii(QVariant newRadius);
    void _updateMarginZoneCenters(QGeoCoordinate newCenter);

private:
    void _init(void);

    bool _inclusion;
    const Vehicle* _vehicle;
    double _fenceMargin;
    double _groundBufferMargin;
    QGCMapCircle _contingencyZone;
    QGCMapCircle _groundBuffer;

    static const int _jsonCurrentVersion = 1;

    static const char* _jsonInclusionKey;
};
