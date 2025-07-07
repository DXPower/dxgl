#include <catch2/catch_session.hpp>

#include <common/Logging.hpp>

int main( int argc, char* argv[] ) {
    using namespace services;
    logging::SetCommonSink(logging::CreateConsoleSink());

    int result = Catch::Session().run( argc, argv );
    return result;
}