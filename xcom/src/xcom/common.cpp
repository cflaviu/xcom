#include "xcom/common.hpp"
#include <iostream>

namespace xcom::session {

registrar_impl::~registrar_impl() noexcept
{
    try
    {
        clear();
    }
    catch(...)
    {}
}

void registrar_impl::add(item* item)
{
    {
        lock_t lock(container_.mutex);
        container_.insert(item);
    }

    std::cout << "registrar - added " << std::hex << item << std::dec << '\n';
}

void registrar_impl::remove(item* item)
{
    {
        lock_t lock(container_.mutex);
        container_.erase(item);
    }

    std::cout << "registrar - removed " << std::hex << item << std::dec << '\n';
    item->~item();
}

void registrar_impl::clear()
{
    lock_t lock(container_.mutex);
    for(auto* item : container_)
    {
        item->~item();
    }

    container_.clear();
}

common::common(registrar* registrar, const xcom::settings_t& settings, io_context& context):
    socket_(context),
    receiving_buffer_(settings.receiving_buffer_size, {}),
    registrar_(registrar)
{}

common::common(registrar* registrar, const xcom::settings_t& settings, socket&& socket):
    socket_(std::move(socket)),
    receiving_buffer_(settings.receiving_buffer_size, {}),
    registrar_(registrar)
{
}

error_t common::sending_error() const noexcept
{
    return sending_error_;
}

const progress_info& common::sending_progress() const noexcept
{
    return sending_progress_;
}

error_t common::receiving_error() const noexcept
{
    return receiving_error_;
}

const progress_info& common::receiving_progress() const noexcept
{
    return receiving_progress_;
}

void common::async_cancel_sending()
{
    // not implemented
}

void common::async_cancel_receiving()
{
    // not implemented
}

void common::disconnect()
{
    socket_.shutdown(tcp::socket::shutdown_both);
    socket_.close();
}

void common::give_up()
{
    disconnect();
    registrar_->remove(this);
    std::cout << "given up\n";
}

void common::enable_keep_alive()
{
    assert(socket_.is_open());
    boost::system::error_code ec;
    socket_.set_option(boost::asio::socket_base::keep_alive(true), ec);
    assert(!ec);
}

void common::async_send(buffer_view buffer)
{
    std::cout << "sending...\n";
              //<< "socket-open=" << socket_.is_open() << '\n';
    sending_error_ = 0;
    sending_progress_.bytes_transferred = 0;
    sending_progress_.bytes_expected = buffer.size();
    *reinterpret_cast<std::uint32_t*>(buffer.data()) = static_cast<std::uint32_t>(buffer.size());
    try
    {
        //std::cout << "sending buffer-size=" << buffer.size() << '\n';
        boost::asio::async_write(socket_, boost::asio::const_buffer(buffer.data(), buffer.size()),
               [this](error_code error, std::size_t bytes_transferred)
        {
            sending_progress_.bytes_transferred += bytes_transferred;
            sending_error_ = convert_error(error);
            try
            {
                //std::cout << "bytes_transferred=" << bytes_transferred << '\n';
                settings().sending_handler(this, sending_error_, sending_progress_);
                if (sending_error_)
                {
                    give_up();
                }
            }
            catch(...)
            {
                give_up();
            }
        });
    }
    catch(...)
    {
        std::cout << "error async_send\n";
    }
}

void common::async_read() noexcept
{
    //std::cout << "async_read begin\n";
    std::size_t read_amount;
    auto buffer_size = receiving_buffer_.size();
    if (amount_to_receive_ <= buffer_size)
    {
        read_amount = amount_to_receive_;
        amount_to_receive_ = 0;
    }
    else
    {
        read_amount = buffer_size;
        amount_to_receive_ -= buffer_size;
    }

    try
    {
        boost::asio::async_read(socket_, boost::asio::buffer(receiving_buffer_, read_amount),
               [this](error_code error, std::size_t bytes_transferred)
        {
            //std::cout << "bytes_transferred=" << bytes_transferred << '\n';
            receiving_progress_.bytes_transferred += bytes_transferred;
            receiving_error_ = convert_error(error);
            try
            {
                settings().receiving_handler(this, receiving_error_, receiving_progress_,
                                             buffer_view(receiving_buffer_.data(), bytes_transferred));
            }
            catch(...)
            {}

            if (receiving_error_ == 0)
            {
                if (amount_to_receive_ != 0)
                {
                    async_read();
                }
            }
            else
            {
                give_up();
            }
        });
    }
    catch(...)
    {
        std::cout << "error async_read\n";
    }
}

void common::async_receive()
{
    try
    {
        std::cout << "receiving...\n";
                  //<< "socket-open=" << socket_.is_open() << '\n';
        boost::asio::async_read(socket_, boost::asio::buffer(reinterpret_cast<std::uint8_t*>(&amount_to_receive_), sizeof(amount_to_receive_)),
               [this](error_code error, std::size_t bytes_transferred)
        {
            receiving_progress_.bytes_transferred = bytes_transferred;
            receiving_progress_.bytes_expected = amount_to_receive_;

            receiving_error_ = convert_error(error);
            if (receiving_error_ == 0 && amount_to_receive_ <= settings().max_receiving_buffer_size)
            {
                //std::cout << "bytes_transferred=" << bytes_transferred << '\n';
                //std::cout << "receiving " << amount_to_receive_ << " bytes...\n";
                amount_to_receive_ -= bytes_transferred;
                async_read();
            }
            else
            {
                give_up();
            }
        });
    }
    catch(...)
    {
        std::cout << "error async_receive\n";
    }
}

}
