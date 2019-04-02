#ifndef COORDINATE_CONVERSION_H_DEFINED
#define COORDINATE_CONVERSION_H_DEFINED

/**
 * Convert ra, dec as obtained from the INDI framework to
 * az, el as expected in rotctld.
 *
 * \param observer Coordinates of observer
 * \param ra RA of object
 * \param dec DEC of object
 * \param az Returned azimuth
 * \param el Returned elevation
 */
void indi_ra_dec_to_hamlib_az_el(struct ln_lnlat_posn observer, double ra, double dec, double *az, double *el);

/**
 * Convert az, el as obtained from rotctld to ra, dec as expected
 * to be sent back to the INDI framework.
 *
 * \param observer Coordinates of observer
 * \param az Azimuth
 * \param el Elevation
 * \param ra Returned RA
 * \param dec Returned DEC
 */
void hamlib_az_el_to_indi_ra_dec(struct ln_lnlat_posn observer, double az, double el, double *ra, double *dec);

#endif
