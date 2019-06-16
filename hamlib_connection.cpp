#include "hamlib_connection.h"


HamlibRotator::HamlibRotator(INDI::DefaultDevice* device) : Connection::TCP(device)
{
}

bool HamlibRotator::Connect()
{
    rotator = rot_init(ROT_MODEL_NETROTCTL);
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
