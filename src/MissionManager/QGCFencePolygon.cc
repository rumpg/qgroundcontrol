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
#include "Vehicle.h"
#include "ParameterManager.h"

#include <cmath>


const auto apmParamFenceMargin = "FENCE_MARGIN";
const auto apmParamHasParachute = "SCR_USER2";  // custom parameter to indicate presence/absence of parachute
const auto apmParamMaxFenceAltitude = "FENCE_ALT_MAX";

const char* QGCFencePolygon::_jsonInclusionKey = "inclusion";

QGCFencePolygon::QGCFencePolygon(bool inclusion, Vehicle* vehicle, QObject* parent)
    : QGCMapPolygon (parent)
    , _inclusion    (inclusion)
    , _vehicle (vehicle)
    , _fenceMargin (_defaultFenceMargin)
    , _groundBufferMargin (_defaultGroundBufferMargin)
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

void QGCFencePolygon::_init(void)
{
    connect(this, &QGCFencePolygon::inclusionChanged, this, &QGCFencePolygon::_setDirty);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateGroundBuffer);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateContingencyZone);
    _updateFenceMargin();
    _updateGroundBufferMargin();
}

void QGCFencePolygon::_updateFenceMargin(void)
{
    _fenceMargin = _defaultFenceMargin;
    if (_vehicle
        && !_vehicle->isOfflineEditingVehicle()
        && _vehicle->apmFirmware()
        && _vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamFenceMargin)) {
        _fenceMargin = _vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamFenceMargin)->rawValue().toDouble();
    }
    _updateContingencyZone();
}

void QGCFencePolygon::_updateGroundBufferMargin(void)
{
    constexpr auto formFactor = 4.;
    constexpr auto maxVelocity = 18.;
    constexpr auto parachuteGlideFactor = 2.506;
    constexpr auto gravitationalAcceleration = 9.81;

    _groundBufferMargin = _defaultGroundBufferMargin;
    if (_vehicle
        && !_vehicle->isOfflineEditingVehicle()
        && _vehicle->apmFirmware()
        && _vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamMaxFenceAltitude)) {
        auto maxFenceAltitudeParam = _vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamMaxFenceAltitude);
        double maxFenceAltitude = maxFenceAltitudeParam->rawValue().toDouble();
        bool hasParachute = false;
        if (_vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamHasParachute)) {
            auto hasParachuteParam = _vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamHasParachute);
            hasParachute = hasParachuteParam->rawValue().toDouble() > 0;
        }
        if (hasParachute) {
            _groundBufferMargin = formFactor + maxFenceAltitude * parachuteGlideFactor;
        } else {
            _groundBufferMargin = formFactor + maxVelocity * std::sqrt(2 * maxFenceAltitude / gravitationalAcceleration);
        }
    }
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
    _updateFenceMargin();
    _updateGroundBufferMargin();
}

void QGCFencePolygon::setInclusion(bool inclusion)
{
    if (inclusion != _inclusion) {
        _inclusion = inclusion;
        emit inclusionChanged(inclusion);
    }
}
