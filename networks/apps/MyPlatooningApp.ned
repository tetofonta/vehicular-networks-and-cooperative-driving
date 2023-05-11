package org.car2x.plexe.vncd.apps;

import org.car2x.plexe.apps.BaseApp;

simple MyPlatooningApp like BaseApp
{
    parameters:
        int headerLength @unit("bit") = default(0 bit);
        @display("i=block/app2");
        @class(plexe::vncd::MyPlatooningApp);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}