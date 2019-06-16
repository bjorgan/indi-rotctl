#include "hamlib_connection.h"
#include <cstring>

HamlibRotator::HamlibRotator(INDI::DefaultDevice* device) : Connection::TCP(device)
{
}

bool HamlibRotator::Connect()
{
    const char *hostname = AddressT[0].text;
    const char *port     = AddressT[1].text;

    //create rotator
    rotator = rot_init(ROT_MODEL_NETROTCTL);

    //set hostname and port of rotator
    char host_port[FILPATHLEN];
    snprintf(host_port, FILPATHLEN, "%s:%s", hostname, port);
    rot_set_conf(rotator, rot_token_lookup(rotator, "rot_pathname"), host_port);

    //connect to rotator
    int retval = rot_open(rotator);
    return retval == RIG_OK;
}

bool HamlibRotator::Disconnect()
{
    int retval = rot_close(rotator);
    return retval == RIG_OK;
}

ROT* HamlibRotator::getRotator()
{
    return rotator;
}
