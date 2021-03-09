#pragma  once
#include "xcom.hpp"
#include <boost/asio.hpp>
#include <mutex>
#include <type_traits>
#include <unordered_set>
#include <stdext/secured.hpp>

namespace xcom
{
    using lock_t = std::lock_guard<std::mutex>;
    using io_context = boost::asio::io_context;
    using endpoint = boost::asio::ip::tcp::endpoint;
    using tcp = boost::asio::ip::tcp;
    using socket = boost::asio::ip::tcp::socket;
    using acceptor = boost::asio::ip::tcp::acceptor;
    using error_code = boost::system::error_code;
    using address_t = boost::asio::ip::address;
    using address_v4_t = boost::asio::ip::address_v4;
    using address_v6_t = boost::asio::ip::address_v6;
    using buffer_t = std::vector<std::uint8_t>;

    inline error_t convert_error(const error_code& code)
    {
        return code.value();
    }

    namespace session
    {
        class registrar_impl
        {
        public:
            using item_container = stdext::secured<std::unordered_set<item*>>;

            ~registrar_impl() noexcept;
            const item_container& container() const { return container_; }

            void add(item* item);
            void remove(item* item);
            void clear();

        protected:
            item_container container_;
        };

        class common: public item
        {
        public:
            void async_send(buffer_view buffer) override;
            void async_cancel_sending() override;
            virtual error_t sending_error() const noexcept override;
            virtual const progress_info& sending_progress() const noexcept override;

            void async_receive() override;
            void async_cancel_receiving() override;
            virtual error_t receiving_error() const noexcept override;
            virtual const progress_info& receiving_progress() const noexcept override;

        protected:
            common(registrar* registrar, const settings_t& settings, io_context& context);
            common(registrar* registrar, const settings_t& settings, socket&& socket);

            void async_read() noexcept;
            void disconnect();
            void give_up();
            void enable_keep_alive();

            socket socket_;
            buffer_t receiving_buffer_;
            registrar* registrar_;
            progress_info sending_progress_ {};
            progress_info receiving_progress_ {};
            std::atomic<error_t> sending_error_ = 0;
            std::atomic<error_t> receiving_error_ = 0;
            std::uint32_t amount_to_receive_ = 0;
        };
    }
}
