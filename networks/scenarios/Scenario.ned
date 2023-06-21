package org.car2x.plexe.vncd.scenarios;

import org.car2x.plexe.scenarios.ManeuverScenario;

simple Scenario extends ManeuverScenario
{
    parameters:
        @display("i=block/app2");
        @class(plexe::vncd::Scenario);

}
