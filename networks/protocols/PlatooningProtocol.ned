package org.car2x.plexe.vncd.protocols;

import org.car2x.plexe.protocols.BBaseProtocol;

simple PlatooningProtocol extends BBaseProtocol
{
    parameters:
        @display("i=block/network2");
        @class(plexe::vncd::PlatooningProtocol);

        double platoonAdvertisementInterval @unit("s");
        double platooningFormationSpeedRange;
}