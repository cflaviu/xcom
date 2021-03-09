#pragma once
#ifndef PCH
    #include <atomic>
    #include <cstdint>
    #include <functional>
    #include <memory>
    #include <span>
    #include <variant>
#endif

namespace xcom
{
    using buffer_view = std::span<std::uint8_t>;
    using error_t = std::int32_t;
    using aerror_t = std::atomic<error_t>;
    using ipv4_address = std::array<std::uint8_t, 4>;
    using ipv6_address = std::array<std::uint8_t, 16>;
    using ip_address = std::variant<ipv4_address, ipv6_address>;
    using port_no = std::uint16_t;

    /// @brief Info about communication progress
    struct progress_info
    {
        std::atomic<std::uint32_t> bytes_transferred;
        std::atomic<std::uint32_t> bytes_expected;

        progress_info(std::uint32_t bytes_expected = 0):
            bytes_transferred(0),
            bytes_expected(bytes_expected)
        {}

        bool completed() const noexcept
        {
            return bytes_transferred == bytes_expected;
        }
    };

    class settings_t;

    namespace session
    {
        class item;
        using item_unique_ptr = std::unique_ptr<item>;
        using item_ptr = item*;

        using handler = std::function<void(item_ptr, error_t&)>;
        using sending_handler_t = std::function<void(item_ptr, aerror_t&, const progress_info&)>;
        using receiving_handler_t = std::function<void(item_ptr, aerror_t&, const progress_info&, buffer_view)>;

        /// @brief Session registrar interface
        class registrar
        {
        public:
            virtual ~registrar() noexcept = default;

            virtual void add(item* item) = 0;
            virtual void remove(item* item) = 0;
        };

        /// @brief Session interface
        class item
        {
        public:
            virtual ~item() noexcept = default;

            virtual void async_send(buffer_view) = 0;
            virtual void async_cancel_sending() = 0;
            virtual error_t sending_error() const noexcept = 0;
            virtual const progress_info& sending_progress() const noexcept = 0;

            virtual void async_receive() = 0;
            virtual void async_cancel_receiving() = 0;
            virtual error_t receiving_error() const noexcept = 0;
            virtual const progress_info& receiving_progress() const noexcept = 0;

            virtual const settings_t& settings() const = 0;
        };
    }

    /// @brief Servc
    class settings_t
    {
    public:
        session::handler session_handler;
        session::sending_handler_t sending_handler;
        session::receiving_handler_t receiving_handler;
        std::size_t receiving_buffer_size = 2 * 1024 * 1024;
        std::size_t max_receiving_buffer_size = ~0;
        ip_address address;
        port_no port;
        std::uint8_t thread_pool_size = 0;
    };

    namespace client
    {
        using settings_t = xcom::settings_t;

        /// @brief Client interface
        class item: public session::registrar
        {
        public:
            static std::unique_ptr<item> create();

            virtual void new_session(settings_t settings) = 0;

            virtual void run() = 0;
        };
    }

    namespace server
    {
        using settings_t = xcom::settings_t;

        /// @brief Server interface
        class item: public session::registrar
        {
        public:
            static std::unique_ptr<item> create(settings_t settings);

            virtual const settings_t& settings() const = 0;

            virtual void run() = 0;
        };
    }
}
