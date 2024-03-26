#include "Vehicle.h"
#include "ParameterManager.h"

#include <cmath>


constexpr double defaultFenceMargin = 4.;
constexpr double defaultGroundBufferMargin = 15.;

const auto apmParamFenceMargin = "FENCE_MARGIN";
const auto apmParamHasParachute = "SCR_USER2";  // custom parameter to indicate presence/absence of parachute
const auto apmParamMaxFenceAltitude = "FENCE_ALT_MAX";


inline double getFenceMargin(const Vehicle* vehicle)
{
    if (vehicle
        && !vehicle->isOfflineEditingVehicle()
        && vehicle->apmFirmware()
        && vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamFenceMargin)) {
        return vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamFenceMargin)->rawValue().toDouble();
    }
    return defaultFenceMargin;
}

inline double getGroundBufferMargin(const Vehicle* vehicle)
{
    constexpr auto formFactor = 4.;
    constexpr auto maxVelocity = 18.;
    constexpr auto parachuteGlideFactor = 2.506;
    constexpr auto gravitationalAcceleration = 9.81;

    if (vehicle
        && !vehicle->isOfflineEditingVehicle()
        && vehicle->apmFirmware()
        && vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamMaxFenceAltitude)) {
        auto maxFenceAltitudeParam = vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamMaxFenceAltitude);
        double maxFenceAltitude = maxFenceAltitudeParam->rawValue().toDouble();
        bool hasParachute = false;
        if (vehicle->parameterManager()->parameterExists(FactSystem::defaultComponentId, apmParamHasParachute)) {
            auto hasParachuteParam = vehicle->parameterManager()->getParameter(FactSystem::defaultComponentId, apmParamHasParachute);
            hasParachute = hasParachuteParam->rawValue().toDouble() > 0.;
        }
        if (hasParachute) {
            return formFactor + maxFenceAltitude * parachuteGlideFactor;
        } else {
            return formFactor + maxVelocity * std::sqrt(2 * maxFenceAltitude / gravitationalAcceleration);
        }
    }
    return defaultGroundBufferMargin;
}
