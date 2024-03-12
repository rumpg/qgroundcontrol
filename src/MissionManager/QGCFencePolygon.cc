/****************************************************************************
 *
 * (c) 2009-2020 QGROUNDCONTROL PROJECT <http://www.qgroundcontrol.org>
 *
 * QGroundControl is licensed according to the terms in the file
 * COPYING.md in the root of the source code directory.
 *
 ****************************************************************************/

#include "QGCFencePolygon.h"
#include "JsonHelper.h"

#include "QGCFenceCommon.h"


const char* QGCFencePolygon::_jsonInclusionKey = "inclusion";

QGCFencePolygon::QGCFencePolygon(bool inclusion, Vehicle* vehicle, QObject* parent)
    : QGCMapPolygon (parent)
    , _inclusion    (inclusion)
    , _vehicle (vehicle)
    , _fenceMargin (getFenceMargin(vehicle))
    , _groundBufferMargin (getGroundBufferMargin(vehicle))
    , _contingencyZone ()
    , _groundBuffer ()
{
    _init();
}

QGCFencePolygon::QGCFencePolygon(const QGCFencePolygon& other, QObject* parent)
    : QGCMapPolygon (other, parent)
    , _inclusion    (other._inclusion)
    , _vehicle (other._vehicle)
    , _fenceMargin (other._fenceMargin)
    , _groundBufferMargin (other._groundBufferMargin)
    , _contingencyZone ()
    , _groundBuffer ()
{
    _init();
}

const QGCFencePolygon& QGCFencePolygon::operator=(const QGCFencePolygon& other)
{
    QGCMapPolygon::operator=(other);

    setInclusion(other._inclusion);
    setVehicle(other._vehicle);

    return *this;
}

void QGCFencePolygon::saveToJson(QJsonObject& json)
{
    json[JsonHelper::jsonVersionKey] = _jsonCurrentVersion;
    json[_jsonInclusionKey] = _inclusion;
    QGCMapPolygon::saveToJson(json);
}

bool QGCFencePolygon::loadFromJson(const QJsonObject& json, bool required, QString& errorString)
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
        errorString = tr("GeoFence Polygon only supports version %1").arg(_jsonCurrentVersion);
        return false;
    }

    if (!QGCMapPolygon::loadFromJson(json, required, errorString)) {
        return false;
    }

    setInclusion(json[_jsonInclusionKey].toBool());

    return true;
}

void QGCFencePolygon::setVehicle(const Vehicle* vehicle)
{
    _vehicle = vehicle;
    _fenceMargin = getFenceMargin(vehicle);
    _updateContingencyZone();
    _groundBufferMargin = getGroundBufferMargin(vehicle);
    _updateGroundBuffer();
}

void QGCFencePolygon::setInclusion(bool inclusion)
{
    if (inclusion != _inclusion) {
        _inclusion = inclusion;
        emit inclusionChanged(inclusion);
    }
}

void QGCFencePolygon::_init(void)
{
    connect(this, &QGCFencePolygon::inclusionChanged, this, &QGCFencePolygon::_setDirty);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateGroundBuffer);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateContingencyZone);
    _updateContingencyZone();
    _updateGroundBuffer();
}

void QGCFencePolygon::_setDirty(void)
{
    setDirty(true);
}

void QGCFencePolygon::_updateGroundBuffer(void)
{
    auto groundBuffer = QGCMapPolygon{*this};  // work on a temporary variable to avoid multiple update signals
    groundBuffer.offset(_groundBufferMargin);
    _groundBuffer = groundBuffer;
}

void QGCFencePolygon::_updateContingencyZone(void)
{
    auto contingencyZone = QGCMapPolygon{*this};  // work on a temporary variable to avoid multiple update signals
    contingencyZone.offset(-_fenceMargin);
    _contingencyZone = contingencyZone;
}
