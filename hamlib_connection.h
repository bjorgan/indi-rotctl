#include "connectionplugins/connectiontcp.h"

#include "indilogger.h"
#include "indistandardproperty.h"

#include <hamlib/rotator.h>

class HamlibRotator : public Connection::TCP {
    /**
     * INDI::Telescope needs an active Connection instance - instead of
     * creating a dummy Connection, we wrap Hamlib inside.
     **/

    public:
        HamlibRotator(INDI::DefaultDevice *device);

        bool Connect();

        bool Disconnect();

        ROT *getRotator();
    private:
        ROT *rotator;
};
