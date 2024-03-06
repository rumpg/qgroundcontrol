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

const char* QGCFencePolygon::_jsonInclusionKey = "inclusion";

QGCFencePolygon::QGCFencePolygon(bool inclusion, QObject* parent)
    : QGCMapPolygon (parent)
    , _inclusion    (inclusion)
    , _d1 (_defaultFenceMargin)
    , _d2 (30.0)
    , _contingencyZone ()
    , _groundBuffer ()
{
    _init();
}

QGCFencePolygon::QGCFencePolygon(const QGCFencePolygon& other, QObject* parent)
    : QGCMapPolygon (other, parent)
    , _inclusion    (other._inclusion)
    , _d1 (other._d1)
    , _d2 (other._d2)
    , _contingencyZone ()
    , _groundBuffer ()
{
    _init();
}

const QGCFencePolygon& QGCFencePolygon::operator=(const QGCFencePolygon& other)
{
    QGCMapPolygon::operator=(other);

    setInclusion(other._inclusion);
    setD1(other._d1);
    setD2(other._d2);

    return *this;
}

void QGCFencePolygon::_init(void)
{
    connect(this, &QGCFencePolygon::inclusionChanged, this, &QGCFencePolygon::_setDirty);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateGroundBuffer);
    connect(this, &QGCFencePolygon::pathChanged, this, &QGCFencePolygon::_updateContingencyZone);
    _updateGroundBuffer();
    _updateContingencyZone();
}

void QGCFencePolygon::_setDirty(void)
{
    setDirty(true);
}

void QGCFencePolygon::_updateGroundBuffer(void)
{
    auto groundBuffer = QGCMapPolygon{*this};  // work on a temporary variable to avoid multiple update signals
    groundBuffer.offset(_d2);
    _groundBuffer = groundBuffer;  // TODO: move-assignment?
}

void QGCFencePolygon::_updateContingencyZone(void)
{
    auto contingencyZone = QGCMapPolygon{*this};  // work on a temporary variable to avoid multiple update signals
    contingencyZone.offset(-_d1);
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

void QGCFencePolygon::setD1(double d1)
{
    _d1 = d1;
    _updateContingencyZone();
}

void QGCFencePolygon::setD2(double d2)
{
    _d2 = d2;
    _updateGroundBuffer();
}

void QGCFencePolygon::setInclusion(bool inclusion)
{
    if (inclusion != _inclusion) {
        _inclusion = inclusion;
        emit inclusionChanged(inclusion);
    }
}
