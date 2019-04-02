#include "coordinate_conversion.h"
#include <libnova/transform.h>
#include <libnova/julian_day.h>

void indi_ra_dec_to_hamlib_az_el(struct ln_lnlat_posn observer, double ra, double dec, double *az, double *el)
{
    //convert from hours to decimals
    double ra_decimal = ra*360.0/24.0;
    double dec_decimal = dec;

    //need time and position for conversion
    double JD = ln_get_julian_from_sys();

    //convert from radec to az,el in libnova horizontal coordinates
    struct ln_equ_posn ra_dec;
    ra_dec.ra = ra_decimal;
    ra_dec.dec = dec_decimal;
    struct ln_hrz_posn az_el;
    ln_get_hrz_from_equ(&ra_dec, &observer, JD, &az_el);

    //libnova has az = 0 => south while hamlib is az = 0 => north, so need to turn around
    *az = az_el.az-180;
    *el = az_el.alt;
}

void hamlib_az_el_to_indi_ra_dec(struct ln_lnlat_posn observer, double az, double el, double *ra, double *dec)
{
    struct ln_hrz_posn az_el;
    az_el.az = az + 180;
    az_el.alt = el;

    double JD = ln_get_julian_from_sys();

    struct ln_equ_posn ra_dec;
    ln_get_equ_from_hrz(&az_el, &observer, JD, &ra_dec);

    *ra = ra_dec.ra*24.0/360.0;
    *dec = ra_dec.dec;
}
