#include "xcom/client.hpp"
#include <memory>
#include <iostream>

namespace xcom {
namespace client {

std::unique_ptr<item> item::create()
{
    return std::make_unique<item_impl>();
}

session::session(xcom::session::registrar* registrar, settings_t settings, io_context& context):
    session::common(registrar, settings, context),
    settings_(std::move(settings))
{
    auto address = (settings.address.index() == 0) ?
        address_t(address_v4_t(std::get<ipv4_address>(settings.address))) :
        address_t(address_v6_t(std::get<ipv6_address>(settings.address)));
    socket_.async_connect(endpoint(address, settings_.port), [this](error_code error_code)
    {
        error_t error = convert_error(error_code);
        bool no_error = error == 0;
        try
        {
            settings_.session_handler(no_error ? this : nullptr, error);
            if (no_error)
            {
                registrar_->add(this);
            }
            else
            {
                std::cout << "error connecting a\nsession closed";
                delete this;
            }
        }
        catch(...)
        {
            std::cout << "error connecting b\nsession closed";
            delete this;
        }
    });
}

const settings_t& session::settings() const
{
    return settings_;
}

item_impl::item_impl():
    io_context_(2)
{}

void item_impl::add(session::item* item)
{
    registrar_.add(item);
}

void item_impl::remove(session::item* item)
{
    registrar_.remove(item);
}

void item_impl::new_session(settings_t settings)
{
    new session(this, std::move(settings), io_context_);
}

void item_impl::run()
{
    io_context_.run();
}

}}
