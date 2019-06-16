#include "indi_rotctl.h"
#include <indicom.h>

#include <cmath>
#include <memory>
#include <indilogger.h>

#include <indistandardproperty.h>

#include "coordinate_conversion.h"
#include <unistd.h>

#include <hamlib/rotator.h>

/**
 * Boilerplate code, required for driver.
 **/

static std::unique_ptr<RotctlDriver> simpleScope(new RotctlDriver());

void ISGetProperties(const char *dev)
{
    simpleScope->ISGetProperties(dev);
}

void ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n)
{
    simpleScope->ISNewSwitch(dev, name, states, names, n);
}

void ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    simpleScope->ISNewText(dev, name, texts, names, n);
}

void ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n)
{
    simpleScope->ISNewNumber(dev, name, values, names, n);
}

void ISNewBLOB(const char *dev, const char *name, int sizes[], int blobsizes[], char *blobs[], char *formats[],
               char *names[], int n)
{
    simpleScope->ISNewBLOB(dev, name, sizes, blobsizes, blobs, formats, names, n);
}

void ISSnoopDevice(XMLEle *root)
{
    simpleScope->ISSnoopDevice(root);
}

/**
 RotctlDriver functions.
 */

RotctlDriver::RotctlDriver()
{
    SetTelescopeCapability(TELESCOPE_CAN_GOTO | TELESCOPE_CAN_ABORT | TELESCOPE_HAS_LOCATION, 0);

    setTelescopeConnection(CONNECTION_NONE);
}

bool RotctlDriver::initProperties()
{
    INDI::Telescope::initProperties();

    //set active connection to custom connection wrapped around hamlib rotator.
    //(INDI interface won't work without a valid active Connection instance.)
    rotator_connection = new HamlibRotator(this);
    INDI::DefaultDevice::registerConnection(rotator_connection);

    return true;
}

bool RotctlDriver::Handshake()
{
    return true;
}

const char *RotctlDriver::getDefaultName()
{
    return "Rotctld interface";
}

bool RotctlDriver::Goto(double ra, double dec)
{
    //lock onto new RA, DEC
    target_ra  = ra;
    target_dec = dec;

    //start motion
    indi_ra_dec_to_hamlib_az_el(observer, ra, dec, &target_azimuth, &target_elevation);
    rot_set_position(rotator_connection->getRotator(), target_azimuth, target_elevation);

    //mark state as slewing
    TrackState = SCOPE_SLEWING;
    return true;
}

bool RotctlDriver::Abort()
{
    rot_stop(rotator_connection->getRotator());
    TrackState = SCOPE_IDLE;
    return true;
}

bool RotctlDriver::ReadScopeStatus()
{
    //update times since last call
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    double time_since_last_call = current_time.tv_sec - last_rotctl_read_call.tv_sec + 1.0e-06*(current_time.tv_usec - last_rotctl_read_call.tv_usec);

    //get current az, el position and RA, DEC from hamlib
    float last_azimuth = current_azimuth;
    float last_elevation = current_elevation;
    rot_get_position(rotator_connection->getRotator(), &current_azimuth, &current_elevation);
    hamlib_az_el_to_indi_ra_dec(observer, current_azimuth, current_elevation, &current_ra, &current_dec);

    //calculate elevation and azimuth change rates
    double az_rate = (last_azimuth - current_azimuth)/time_since_last_call;
    double el_rate = (last_elevation - current_elevation);

    const double RATE_THRESHOLD = 0.001;

    switch (TrackState) {
        case SCOPE_SLEWING:
            //wait until we are at target az, el (rates are zero when rotctld
            //has reached what it can reach), then change state to TRACKING
            if ((fabs(az_rate) < RATE_THRESHOLD) && (fabs(el_rate) < RATE_THRESHOLD)) {
                TrackState = SCOPE_TRACKING;
            }
            break;

        case SCOPE_TRACKING:
            //restart rotctld motion and slew whenever target radec doesn't
            //match current radec. TODO: Should do this only when within
            //achievable accuracy.
            if ((fabs(target_dec - current_dec) > 0) || (fabs(target_ra - current_ra) > 0)) {
                indi_ra_dec_to_hamlib_az_el(observer, target_ra, target_dec, &target_azimuth, &target_elevation);
                rot_set_position(rotator_connection->getRotator(), target_azimuth, target_elevation);
                TrackState = SCOPE_SLEWING;
            }
            break;

        default:
            break;
    }

    NewRaDec(current_ra, current_dec);
    last_rotctl_read_call = current_time;

    return true;
}

bool RotctlDriver::updateLocation(double latitude, double longitude, double elevation)
{
    INDI_UNUSED(elevation);
    this->observer.lat = latitude;
    this->observer.lng = longitude;
    return true;
}

bool RotctlDriver::updateProperties()
{
    INDI::Telescope::updateProperties();

    //remove switches and stuff that is not needed/supported.
    deleteProperty(MovementNSSP.name); //remove north/south-movement button under motion control
    deleteProperty(MovementWESP.name); //remove west/east movement button under motion control
    deleteProperty(TargetNP.name); //remove slew target thing under motion control (remove entire motion control tab)

    deleteProperty(ScopeParametersNP.name); //remove scope parameters (focal length, ...)
    deleteProperty(DomeClosedLockTP.name); //dome properties
    deleteProperty(ActiveDeviceTP.name); //snoop config
    deleteProperty(ScopeConfigNameTP.name); //scope name
    deleteProperty(ScopeConfigsSP.name); //scope config name
    deleteProperty("USEJOYSTICK"); //joystick enable/disable, found name in indicontroller.cpp.
    return true;
}
