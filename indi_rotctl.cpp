#include "indi_rotctl.h"
#include <indicom.h>

#include <cmath>
#include <memory>
#include <indilogger.h>

#include "rotctld_communication.h"
#include "coordinate_conversion.h"
#include <unistd.h>

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
    //disable default connections, to be replaced by rotctld write connection
    //below
    setTelescopeConnection(CONNECTION_NONE);
    INDI::Telescope::initProperties();

    //create connection against rotctld: this will be the write stream.  can
    //only create one Connection, read stream is therefore created in a
    //different way in Connect(), but also need at least one Connection in
    //order for INDI::Telescope to work properly.  This also defines host and
    //port GUI fields for us.
    rotctld_write_connection = new Connection::TCP(this);
    INDI::DefaultDevice::registerConnection(rotctld_write_connection);

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

bool RotctlDriver::Connect()
{
    //do other necessary connection stuff and connect to rotctld for
    //independent writing (uses Connection::TCP interface for connection)
    bool connection_success = INDI::Telescope::Connect();

    //connect to rotctld for independent reading (direct socket creation)
    rotctld_read_socket = rotctld_connect(rotctld_hostname.c_str(), rotctld_port.c_str());
    if (rotctld_read_socket == -1) {
        LOG_ERROR("Failed to connect rotctld read socket\n");
        connection_success = false;
    }
    return connection_success;

}

bool RotctlDriver::ISNewText(const char *dev, const char *name, char *texts[], char *names[], int n)
{
    //unable to get port from Connection::TCP::port() (I got only zero, bug?):
    //Need to snoop out rotctld address and port information every time it is
    //set before it gets propagated to the Connection instance.
    if (std::string(name) == std::string("DEVICE_ADDRESS")) {
        rotctld_hostname = std::string(texts[0]);
        rotctld_port = std::string(texts[1]);
    }

    return INDI::Telescope::ISNewText(dev, name, texts, names, n);
}

bool RotctlDriver::Disconnect()
{
    close(rotctld_read_socket);
    return INDI::Telescope::Disconnect();
}

bool RotctlDriver::Goto(double ra, double dec)
{
    //lock onto new RA, DEC
    target_ra  = ra;
    target_dec = dec;

    //start motion
    indi_ra_dec_to_hamlib_az_el(observer, ra, dec, &target_azimuth, &target_elevation);
    rotctld_set_position(rotctld_write_connection->getPortFD(), target_azimuth, target_elevation);

    //mark state as slewing
    TrackState = SCOPE_SLEWING;
    return true;
}

bool RotctlDriver::Abort()
{
    rotctld_stop(rotctld_write_connection->getPortFD());
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
    rotctld_get_position(rotctld_read_socket, &current_azimuth, &current_elevation);
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
                rotctld_set_position(rotctld_write_connection->getPortFD(), target_azimuth, target_elevation);
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
