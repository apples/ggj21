#include "scene_mainmenu.hpp"

#include "ember/engine.hpp"
#include "ember/config.hpp"

#include <asio.hpp>
#include <trellis/trellis.hpp>

#include <iostream>
#include <stdexcept>
#include <cstddef>
#include <functional>

void print_exception(const std::exception& e, int level = 0) {
    std::cerr << std::string(level, ' ') << "exception: " << e.what() << '\n';
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception &e) {
        print_exception(e, level + 1);
    } catch (...) {
    }
}

int main(int argc, char* argv[]) try {
    SDL_SetMainReady();

    std::cout << "Init..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error(SDL_GetError());
    }

    std::cout << "Loading config..." << std::endl;

    auto config = ember::config::config{};
    config.display.width = 1920;
    config.display.height = 1080;

    std::cout << "Instantiating engine..." << std::endl;

    auto io = asio::io_context();

    auto io_work = asio::make_work_guard(io);

    auto io_thread = std::thread([&]{
        io.run();
    });

    auto engine = std::make_unique<ember::engine>(io, config);

    engine->queue_transition<scene_mainmenu>(false);

    std::cout << "Success." << std::endl;
    
    std::cout << "Running main loop..." << std::endl;

    try {
        while (!engine->get_should_quit()) {
            engine->tick();
        }
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception:" << std::endl;
        print_exception(e, 1);
        std::terminate();
    }

    io.stop();
    io_thread.join();

    SDL_Quit();

    return EXIT_SUCCESS;
} catch (const std::exception& e) {
    std::cerr << "Fatal exception: " << e.what() << std::endl;
    return EXIT_FAILURE;
}
