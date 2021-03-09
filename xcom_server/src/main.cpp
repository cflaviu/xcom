#include <xcom.hpp>
#include <iostream>

using buffer = std::vector<std::uint8_t>;

buffer my_buffer;

constexpr std::size_t receiving_buffer_size = 8 * 1024;

void test()
{
    using namespace xcom;
    server::settings_t settings;
    settings.receiving_buffer_size = receiving_buffer_size;
    settings.address = ipv4_address{0,0,0,0};
    settings.port = 2021;
    settings.session_handler = [](session::item_ptr session, error_t& error)
    {
        if (!error)
        {
            std::cout << "new session created " << std::hex << session << std::dec << '\n';
            session->async_receive();
        }
        else
        {
            std::cout << "error creating new session: " << error << '\n';
        }
    };

    settings.sending_handler = [](session::item_ptr session, aerror_t& error, const progress_info& progress)
    {
        std::cout << "session " << std::hex << session << std::dec << " - sending ";
        if (!error)
        {
            std::cout << "progress: " << progress.bytes_transferred << '/' << progress.bytes_expected << '\n';
            if (progress.completed())
            {
                std::cout << "fully sent!\n";
                my_buffer.clear();
            }
        }
        else
        {
            std::cout << "error: " << error << '\n';
        }
    };

    settings.receiving_handler = [](session::item_ptr session, aerror_t& error, const progress_info& progress, buffer_view buffer)
    {
        std::cout << "session " << std::hex << session << std::dec << " - receiving ";
        if (!error)
        {
            std::cout << "progress: " << progress.bytes_transferred << '/' << progress.bytes_expected << '\n';
            my_buffer.insert(my_buffer.end(), buffer.begin(), buffer.end());
            if (progress.completed())
            {
                std::cout << "fully received!\n";
                my_buffer.clear();
                session->async_receive();
            }
        }
        else
        {
            std::cout << "error: " << error << '\n';
            my_buffer.clear();
        }
    };

    auto server = server::item::create(settings);
    server->run();
}

int main()
{
    std::cout << "starting server...\n";
    try
    {
        test();
    }
    catch(const std::exception& ex)
    {
        std::cout << ex.what() << '\n';
    }
    catch(...)
    {
        std::cout << "error\n";
    }

    std::cout << "server ends\n";
    return 0;
}
