// Starts the BVHView main loop

#include "app/application.hpp"
#include "core/profile.hpp"

#include <memory>

int main(int argc, char** argv)
{
    PROFILE_INIT();
    PROFILE_TICKERS_INIT();

    auto app = std::make_unique<bvhview::ApplicationState>();
    bvhview::ApplicationInit(app.get(), argc, argv);
    while (!WindowShouldClose())
    {
        bvhview::ApplicationUpdate(app.get());
    }
    bvhview::ApplicationShutdown(app.get());

    return 0;
}
