/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCFenceCircle.h"
#include "JsonHelper.h"

#include "QGCFenceCommon.h"

const char* QGCFenceCircle::_jsonInclusionKey = "inclusion";

QGCFenceCircle::QGCFenceCircle(Vehicle* vehicle, QObject* parent)
    : QGCMapCircle          (parent)
    , _inclusion            (true)
    , _vehicle              (vehicle)
    , _fenceMargin          (getFenceMargin(vehicle))
    , _groundBufferMargin   (getGroundBufferMargin(vehicle))
    , _contingencyZone      (*this)
    , _groundBuffer         (*this)
{
    _init();
}

QGCFenceCircle::QGCFenceCircle(const QGeoCoordinate& center, double radius, bool inclusion, Vehicle* vehicle, QObject* parent)
    : QGCMapCircle          (center, radius, false /* showRotation */, true /* clockwiseRotation */, parent)
    , _inclusion            (inclusion)
    , _vehicle              (vehicle)
    , _fenceMargin          (getFenceMargin(vehicle))
    , _groundBufferMargin   (getGroundBufferMargin(vehicle))
    , _contingencyZone      (*this)
    , _groundBuffer         (*this)
{
    _updateMarginZoneCenters(this->center());
    _updateMarginZoneRadii(this->radius()->rawValue());
    _init();
}

QGCFenceCircle::QGCFenceCircle(const QGCFenceCircle& other, QObject* parent)
    : QGCMapCircle          (other, parent)
    , _inclusion            (other._inclusion)
    , _vehicle              (other._vehicle)
    , _fenceMargin          (other._fenceMargin)
    , _groundBufferMargin   (other._groundBufferMargin)
    , _contingencyZone      (other._contingencyZone)
    , _groundBuffer         (other._groundBuffer)
{
    _init();
}

void QGCFenceCircle::_init(void)
{
    connect(this, &QGCFenceCircle::inclusionChanged, this, &QGCFenceCircle::_setDirty);
    connect(this, &QGCMapCircle::centerChanged, this, &QGCFenceCircle::_updateMarginZoneCenters);
    connect(radius(), &Fact::rawValueChanged, this, &QGCFenceCircle::_updateMarginZoneRadii);
}

const QGCFenceCircle& QGCFenceCircle::operator=(const QGCFenceCircle& other)
{
    QGCMapCircle::operator=(other);

    setInclusion(other._inclusion);
    setVehicle(other._vehicle);

    return *this;
}

void QGCFenceCircle::_setDirty(void)
{
    setDirty(true);
}

void QGCFenceCircle::_updateMarginZoneRadii(QVariant newRadius)
{
    const auto newGroundBufferRadius = newRadius.toDouble() + _groundBufferMargin;
    _groundBuffer.radius()->setRawValue(newGroundBufferRadius);
    const auto newContinencyZoneRadius = newRadius.toDouble() - _fenceMargin;
    _contingencyZone.radius()->setRawValue(newContinencyZoneRadius);
}

void QGCFenceCircle::_updateMarginZoneCenters(QGeoCoordinate newCenter)
{
    _groundBuffer.setCenter(newCenter);
    _contingencyZone.setCenter(newCenter);
}

void QGCFenceCircle::saveToJson(QJsonObject& json)
{
    json[JsonHelper::jsonVersionKey] = _jsonCurrentVersion;
    json[_jsonInclusionKey] = _inclusion;
    QGCMapCircle::saveToJson(json);
}

bool QGCFenceCircle::loadFromJson(const QJsonObject& json, QString& errorString)
{
    errorString.clear();

    QList<JsonHelper::KeyValidateInfo> keyInfoList = {
        { JsonHelper::jsonVersionKey,   QJsonValue::Double, true },
        { _jsonInclusionKey,            QJsonValue::Bool,   true },
    };
    if (!JsonHelper::validateKeys(json, keyInfoList, errorString)) {
        return false;
    }

    if (json[JsonHelper::jsonVersionKey].toInt() != _jsonCurrentVersion) {
        errorString = tr("GeoFence Circle only supports version %1").arg(_jsonCurrentVersion);
        return false;
    }

    if (!QGCMapCircle::loadFromJson(json, errorString)) {
        return false;
    }

    setInclusion(json[_jsonInclusionKey].toBool());

    return true;
}

void QGCFenceCircle::setVehicle(const Vehicle* vehicle)
{
    _vehicle = vehicle;
    _fenceMargin = getFenceMargin(vehicle);
    _groundBufferMargin = getGroundBufferMargin(vehicle);
    _updateMarginZoneRadii(radius()->rawValue());
}

void QGCFenceCircle::setInclusion(bool inclusion)
{
    if (inclusion != _inclusion) {
        _inclusion = inclusion;
        emit inclusionChanged(inclusion);
    }
}
