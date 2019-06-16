#ifndef INDI_ROTCTL_H_DEFINED
#define INDI_ROTCTL_H_DEFINED

#include <inditelescope.h>
#include "hamlib_connection.h"

/**
 * Makes rotctld pretend to be a telescope driver in INDI for tracking in e.g.
 * KStars. Based on simple_scope example.
 */
class RotctlDriver : public INDI::Telescope
{
  public:
    RotctlDriver();

  protected:
    /**
     * Always returns true. Necessary for driver.
     **/
    bool Handshake() override;

    /**
     * Return name of driver.
     **/
    const char *getDefaultName() override;

    /**
     * Initialize properties of driver.
     *
     * Disables default INDI::Telescope connection and creates a
     * Connection::TCP for rotctld writing.
     **/
    bool initProperties() override;

    /**
     * Read current rotctld status. When tracking, compare against desired RA,
     * DEC and adjust position accordingly.
     */
    bool ReadScopeStatus() override;

    /**
     * Go to new RA, DEC. Starts rotctld motion using az,el at time of call.
     *
     * \param ra RA in hours
     * \param dec Declination in degrees
     */
    bool Goto(double ra, double dec) override;

    /**
     * Abort current motion. Sends 'S' to rotctld.
     */
    bool Abort() override;

    /**
     * Update lat/lon/ele. Gets updates from client, is called since driver has capability TELESCOPE_HAS_LOCATION.
     **/
    bool updateLocation(double latitude, double longitude, double elevation);

    /**
     * Reimplemented for deleting default client properties/GUI things that is not needed for this driver.
     **/
    bool updateProperties();

  private:
    HamlibRotator *rotator_connection;

    ///Time for last call to rotctld, updated in ReadScopeStatus(). Used for
    ///calculating az, el rates
    struct timeval last_rotctl_read_call;

    ///Target RA for current motion
    double target_ra {0};
    ///Target DEC for current motion
    double target_dec {0};

    ///Target azimuth for current motion
    double target_azimuth {0};
    ///Target elevation for current motion
    double target_elevation {0};

    ///Current RA from rotctld, updated in ReadScopeStatus()
    double current_ra {0};
    ///Current RA from rotctld, updated in ReadScopeStatus()
    double current_dec {90};

    ///Current azimuth from rotctld, updated in ReadScopeStatus()
    float current_azimuth {0};
    ///Current elevation from rotctld, updated in ReadScopeStatus()
    float current_elevation {0};

    ///Observer position (latitude, longitude)
    struct ln_lnlat_posn observer;
};

#endif
