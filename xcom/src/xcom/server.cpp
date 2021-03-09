#include "xcom/server.hpp"
#include <iostream>

namespace xcom {
namespace server {

std::unique_ptr<item> item::create(settings_t settings)
{
    return std::make_unique<item_impl>(std::move(settings));
}

session::session(xcom::session::registrar* registrar, const settings_t& settings, socket&& socket):
    session::common(registrar, settings, std::move(socket)),
    settings_(settings)
{}

const settings_t& session::settings() const
{
    return settings_;
}

item_impl::item_impl(settings_t settings):
    settings_(std::move(settings)),
    io_context_(2),
    acceptor_(io_context_, endpoint(settings.address.index() == 0 ? tcp::v4() : tcp::v6(), settings.port))
{
    if (!settings_.session_handler)
    {
        throw std::runtime_error("session handler not callable");
    }

    if (!settings_.sending_handler)
    {
        throw std::runtime_error("sending handler not callable");
    }

    if (!settings_.receiving_handler)
    {
        throw std::runtime_error("receiving handler not callable");
    }
}

void item_impl::add(session::item* item)
{
    registrar_.add(item);
}

void item_impl::remove(session::item* item)
{
    registrar_.remove(item);
}

const settings_t& item_impl::settings() const
{
    return settings_;
}

void item_impl::accept()
{
    socket_.emplace(io_context_);
    acceptor_.async_accept(*socket_, [this](error_code error)
    {
        try
        {
            xcom::session::item_unique_ptr new_session;
            if (!error)
            {
                new_session.reset(new session(this, settings_, std::move(*socket_)));
            }

            error_t local_error = convert_error(error);
            settings_.session_handler(new_session.get(), local_error);
            if (!local_error)
            {
                registrar_.add(new_session.release());
            }
        }
        catch(...)
        {
            std::cout << "error accepting\n";
        }

        accept();
    });
}

void item_impl::run()
{
    accept();
    io_context_.run();
}

}}
