#include "xcom/common.hpp"

namespace xcom
{
    namespace client
    {
        class session: public xcom::session::common
        {
        public:
            session(xcom::session::registrar* registrar, settings_t settings, io_context& context);

            const settings_t& settings() const override;

        protected:
            settings_t settings_;
        };

        using session_shared_ptr = std::shared_ptr<client::session>;


        class item_impl: public xcom::client::item
        {
        public:
            item_impl();

            void new_session(settings_t settings) override;

            void run() override;

        protected:
            void add(session::item* item) override;
            void remove(session::item* item) override;

            io_context io_context_;
            xcom::session::registrar_impl registrar_;
        };
    }
}
