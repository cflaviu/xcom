#include "xcom/common.hpp"

namespace xcom
{
    namespace server
    {
        class settings;

        class session: public xcom::session::common
        {
        public:
            session(xcom::session::registrar* registrar, const settings_t& settings, socket&& socket);

            const settings_t& settings() const override;

        protected:
            const settings_t& settings_;
        };

        using session_shared_ptr = std::shared_ptr<server::session>;


        class item_impl: public xcom::server::item
        {
        public:
            item_impl(settings_t settings);

            const settings_t& settings() const override;

            void run() override;

        protected:
            void accept();
            void add(session::item* item) override;
            void remove(session::item* item) override;

            settings_t settings_;
            io_context io_context_;
            std::optional<socket> socket_;
            acceptor acceptor_;
            xcom::session::registrar_impl registrar_;
        };
    }
}
