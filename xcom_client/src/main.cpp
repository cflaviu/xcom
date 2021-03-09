#include <xcom.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using buffer = std::vector<std::uint8_t>;

constexpr std::size_t buffer_size = 32 * 1024;

buffer my_buffer(buffer_size, 0);

void test()
{
    using namespace xcom;
    client::settings_t settings;
    settings.address = ipv4_address{0,0,0,0};
    settings.port = 2021;
    settings.session_handler = [](session::item_ptr session, error_t& error)
    {
        if (!error)
        {
            std::cout << "new session created " << std::hex << session << std::dec << '\n';
            session->async_send(my_buffer);
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
            if (progress.bytes_transferred == progress.bytes_expected)
            {
                std::cout << "fully sent!\n";
                //my_buffer.clear();
                using namespace std::chrono_literals;
                std::this_thread::sleep_for(1000ms);
                std::cout << "send again\n";
                session->async_send(my_buffer);
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
            if (progress.bytes_transferred == progress.bytes_expected)
            {
                std::cout << "filly received!\n";
                my_buffer.clear();
            }
        }
        else
        {
            std::cout << "error: " << error << '\n';
            my_buffer.clear();
        }
    };

    auto client = client::item::create();
    client->new_session(settings);
    client->run();
}

int main()
{
    std::cout << "starting client...\n";
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

    std::cout << "client stops\n";
    return 0;
}
