//
// Created by user on 5/11/23.
//

#ifndef VNCD_PROJECT_MYPLATOONINGAPP_H
#define VNCD_PROJECT_MYPLATOONINGAPP_H

#include <plexe/apps/BaseApp.h>

namespace plexe::vncd {
    class MyPlatooningApp : public BaseApp {
    public:
        MyPlatooningApp() = default;
    };

    Define_Module(MyPlatooningApp)
}


#endif //VNCD_PROJECT_MYPLATOONINGAPP_H
