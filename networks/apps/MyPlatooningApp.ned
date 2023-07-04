package org.car2x.plexe.vncd.apps;

import org.car2x.plexe.apps.BaseApp;

simple MyPlatooningApp like BaseApp
{
    parameters:
        int headerLength @unit("bit") = default(0 bit);
        @display("i=block/app2");
        @class(plexe::vncd::MyPlatooningApp);

        @signal[maneuverSpeed](type=double);
        @statistic[maneuverSpeed](title="maneuverSpeed"; source="maneuverSpeed"; record=vector,stats; interpolationmode=none);
        @signal[platoonSize](type=int);
        @statistic[platoonSize](title="platoonSize"; source="platoonSize"; record=vector,stats; interpolationmode=none);
    gates:
        input lowerLayerIn;
        output lowerLayerOut;
        input lowerControlIn;
        output lowerControlOut;
}