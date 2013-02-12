/*
  Copyright (c) 2012 Matthew Stump

  This file is part of libcql.

  libcql is free software; you can redistribute it and/or modify it under
  the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  libcql is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CQL_CLIENT_IMPL_H_
#define CQL_CLIENT_IMPL_H_

#include <iomanip>
#include <iostream>
#include <istream>
#include <ostream>
#include <stdint.h>
#include <string>

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "libcql/cql.hpp"
#include "libcql/cql_client.hpp"
#include "libcql/cql_event.hpp"
#include "libcql/cql_message.hpp"
#include "libcql/internal/cql_defines.hpp"
#include "libcql/internal/cql_error_impl.hpp"
#include "libcql/internal/cql_header_impl.hpp"
#include "libcql/internal/cql_message_execute_impl.hpp"
#include "libcql/internal/cql_message_prepare_impl.hpp"
#include "libcql/internal/cql_message_query_impl.hpp"
#include "libcql/internal/cql_message_result_impl.hpp"
#include "libcql/internal/cql_message_credentials_impl.hpp"
#include "libcql/internal/cql_message_error_impl.hpp"
#include "libcql/internal/cql_message_options_impl.hpp"
#include "libcql/internal/cql_message_ready_impl.hpp"
#include "libcql/internal/cql_message_register_impl.hpp"
#include "libcql/internal/cql_message_startup_impl.hpp"
#include "libcql/internal/cql_message_supported_impl.hpp"
#include "libcql/cql_serialization.hpp"

namespace cql {

    template <typename cql_transport_t>
    class cql_client_impl_t :
        boost::noncopyable,
        public cql::cql_client_t
    {

    public:

        cql_client_impl_t(boost::asio::io_service& io_service,
                          cql_transport_t* transport) :
            _port(0),
            _resolver(io_service),
            _transport(transport),
            _header_buffer(sizeof(cql::cql_header_impl_t)),
            _request_buffer(CQL_FRAME_MAX_SIZE),
            _connect_callback(0),
            _connect_errback(0),
            _log_callback(0),
            _events_registered(false),
            _event_callback(0),
            _defunct(false),
            _ready(false),
            _closing(false)
        {}

        cql_client_impl_t(boost::asio::io_service& io_service,
                          cql_transport_t* transport,
                          cql::cql_client_t::cql_log_callback_t log_callback) :
            _port(0),
            _resolver(io_service),
            _transport(transport),
            _header_buffer(sizeof(cql::cql_header_impl_t)),
            _request_buffer(CQL_FRAME_MAX_SIZE),
            _connect_callback(0),
            _connect_errback(0),
            _log_callback(log_callback),
            _events_registered(false),
            _event_callback(0),
            _defunct(false),
            _ready(false),
            _closing(false)
        {}

        void
        connect(const std::string& server,
                unsigned int port,
                cql::cql_client_t::cql_connection_callback_t callback,
                cql::cql_client_t::cql_connection_errback_t errback)
        {
            std::list<std::string> events;
            cql::cql_client_t::cql_credentials_t credentials;
            connect(server, port, callback, errback, NULL, events, credentials);
        }

        void
        connect(const std::string& server,
                unsigned int port,
                cql::cql_client_t::cql_connection_callback_t callback,
                cql::cql_client_t::cql_connection_errback_t errback,
                cql::cql_client_t::cql_event_callback_t event_callback,
                const std::list<std::string>& events)
        {
            cql::cql_client_t::cql_credentials_t credentials;
            connect(server, port, callback, errback, event_callback, events, credentials);
        }

        void
        connect(const std::string& server,
                unsigned int port,
                cql_connection_callback_t callback,
                cql_connection_errback_t errback,
                cql::cql_client_t::cql_event_callback_t event_callback,
                const std::list<std::string>& events,
                cql::cql_client_t::cql_credentials_t credentials)
        {
            _server = server;
            _port = port;
            _connect_callback = callback;
            _connect_errback = errback;
            _event_callback = event_callback;
            _events = events;
            _credentials = credentials;

            resolve();
        }

        cql::cql_stream_id_t
        query(const std::string& query,
              cql_int_t consistency,
              cql::cql_client_t::cql_message_callback_t callback,
              cql::cql_client_t::cql_message_errback_t errback)
        {
            cql::cql_message_query_t m(query, consistency);
            cql::cql_stream_id_t stream = create_request(m,
                                                         boost::bind(&cql_client_impl_t::write_handle,
                                                                     this,
                                                                     boost::asio::placeholders::error,
                                                                     boost::asio::placeholders::bytes_transferred));

            _callback_map.insert(callback_map_t::value_type(stream, callback_pair_t(callback, errback)));
            return stream;
        }

        cql::cql_stream_id_t
        prepare(const cql::cql_message_prepare_t& message,
                cql::cql_client_t::cql_message_callback_t callback,
                cql::cql_client_t::cql_message_errback_t errback)
        {
            cql::cql_stream_id_t stream = create_request(message,
                                                         boost::bind(&cql_client_impl_t::write_handle,
                                                                     this,
                                                                     boost::asio::placeholders::error,
                                                                     boost::asio::placeholders::bytes_transferred));

            _callback_map.insert(callback_map_t::value_type(stream, callback_pair_t(callback, errback)));
            return stream;
        }

        cql::cql_stream_id_t
        execute(const cql::cql_message_execute_t& message,
                cql::cql_client_t::cql_message_callback_t callback,
                cql::cql_client_t::cql_message_errback_t errback)
        {
            cql::cql_stream_id_t stream = create_request(message,
                                                         boost::bind(&cql_client_impl_t::write_handle,
                                                                     this,
                                                                     boost::asio::placeholders::error,
                                                                     boost::asio::placeholders::bytes_transferred));

            _callback_map.insert(callback_map_t::value_type(stream, callback_pair_t(callback, errback)));
            return stream;
        }

        bool
        defunct() const
        {
            return _defunct;
        }

        bool
        ready() const
        {
            return _ready;
        }

        void
        close()
        {
            _closing = true;
            log(CQL_LOG_INFO, "closing connection");
            boost::system::error_code ec;
            _transport->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _transport->lowest_layer().close();
        }

        void
        close(cql_error_t& err)
        {
            _closing = true;
            boost::system::error_code ec;
            _transport->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
            _transport->lowest_layer().close();
            err.application(false);
            err.application_error(0);
            err.transport_error(ec.value());
            err.message(ec.message());
        }

        const std::string&
        server() const
        {
            return _server;
        }

        unsigned int
        port() const
        {
            return _port;
        }

        const std::list<std::string>&
        events() const
        {
            return _events;
        }

        cql::cql_client_t::cql_event_callback_t
        event_callback() const
        {
            return _event_callback;
        }

        const cql_credentials_t&
        credentials() const
        {
            return _credentials;
        }

        void
        reconnect()
        {
            _closing = false;
            _events_registered = false;
            _ready = false;
            _defunct = false;
            resolve();
        }

    private:

        inline void
        log(cql::cql_short_t level,
            const std::string& message)
        {
            if (_log_callback) {
                _log_callback(level, message);
            }
        }

        cql::cql_stream_id_t
        get_new_stream()
        {
            if (_stream_counter < INT8_MAX) {
                return _stream_counter++;
            }
            else {
                _stream_counter = 0;
                return _stream_counter;
            }
        }

        void
        resolve()
        {
            log(CQL_LOG_DEBUG, "resolving remote host " + _server + ":" + boost::lexical_cast<std::string>(_port));
            boost::asio::ip::tcp::resolver::query query(_server, boost::lexical_cast<std::string>(_port));
            _resolver.async_resolve(query,
                                    boost::bind(&cql_client_impl_t::resolve_handle,
                                                this,
                                                boost::asio::placeholders::error,
                                                boost::asio::placeholders::iterator));
        }

        void
        resolve_handle(const boost::system::error_code& err,
                       boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
        {
            if (!err) {
                log(CQL_LOG_DEBUG, "resolved remote host, attempting to connect");
                boost::asio::async_connect(_transport->lowest_layer(),
                                           endpoint_iterator,
                                           boost::bind(&cql_client_impl_t::connect_handle,
                                                       this,
                                                       boost::asio::placeholders::error));
            }
            else {
                log(CQL_LOG_CRITICAL, "error resolving remote host " + err.message());
                check_transport_err(err);
            }
        }

        void
        connect_handle(const boost::system::error_code& err)
        {
            if (!err) {
                log(CQL_LOG_DEBUG, "connection successful to remote host");
                if (_transport->requires_handshake()) {
                    _transport->async_handshake(boost::bind(&cql_client_impl_t::handshake_handle,
                                                            this,
                                                            boost::asio::placeholders::error));
                }
                else {
                    options_write();
                }
            }
            else {
                log(CQL_LOG_CRITICAL, "error connecting to remote host " + err.message());
                check_transport_err(err);
            }
        }

        void
        handshake_handle(const boost::system::error_code& err)
        {
            if (!err) {
                log(CQL_LOG_DEBUG, "successful ssl handshake with remote host");
                options_write();
            }
            else {
                log(CQL_LOG_CRITICAL, "error performing ssl handshake " + err.message());
                check_transport_err(err);
            }
        }

        cql::cql_stream_id_t
        create_request(const cql::cql_message_t* message,
                       cql::cql_client_t::cql_message_callback_t callback,
                       cql::cql_client_t::cql_message_errback_t errback)
        {
            cql::cql_stream_id_t stream = create_request(message,
                                                         boost::bind(&cql_client_impl_t::write_handle,
                                                                     this,
                                                                     boost::asio::placeholders::error,
                                                                     boost::asio::placeholders::bytes_transferred));

            _callback_map.insert(callback_map_t::value_type(stream, callback_pair_t(callback, errback)));
            return stream;
        }

        cql::cql_stream_id_t
        create_request(cql::cql_message_t* message,
                       write_callback_t callback)
        {
            boost::shared_ptr<cql::cql_message_t> data(message);
            log(CQL_LOG_DEBUG, "sending message: " + data->str());

            cql::cql_stream_id_t id = get_new_stream();
            boost::shared_ptr<cql::cql_header_impl_t> header(
                new cql::cql_header_impl_t(CQL_VERSION_1_REQUEST,
                                           CQL_FLAG_NOFLAG,
                                           id,
                                           _request_message.opcode(),
                                           _request_message.size()));


            boost::asio::async_write(*_transport, header.get(), header->size());
            boost::asio::async_write(*_transport, data->buffer());

            // we have to keep the buffers around until the write is complete
            _request_queue.push_back(request_buffer_t::value_type(header, data));
            _check_request_queue();
            return id;
        }

        void
        write_handle(const boost::system::error_code& err,
                     std::size_t num_bytes)
        {
            if (!_request_queue.empty()) {
                // the write request is complete free the request buffers
                _request_queue.pop_front();
            }
            if (!err) {
                log(CQL_LOG_DEBUG, "wrote to socket " + boost::lexical_cast<std::string>(num_bytes) + " bytes");
            }
            else {
                log(CQL_LOG_ERROR, "error writing to socket " + err.message());
                check_transport_err(err);
            }
        }

        void
        header_read()
        {
            _response_header_buffer.consume(_response_header_buffer.size());
            boost::asio::async_read(*_transport,
                                    _response_header_buffer,
                                    boost::asio::transfer_exactly(sizeof(cql::cql_header_impl_t)),
                                    boost::bind(&cql_client_impl_t<cql_transport_t>::header_read_handle, this, boost::asio::placeholders::error));
        }

        void
        header_read_handle(const boost::system::error_code& err)
        {
            if (!err) {
                cql::cql_header_impl_t header;
                std::istream response_stream(&_response_header_buffer);
                header.read(response_stream);
                log(CQL_LOG_DEBUG, "received header for message " + header.str());
                body_read(header);
            }
            else {
                log(CQL_LOG_ERROR, "error reading header " + err.message());
                check_transport_err(err);
            }
        }

        void
        body_read(const cql::cql_header_impl_t& header)
        {

            switch (header.opcode()) {

            case CQL_OPCODE_ERROR:
                _response_message.reset(new cql::cql_message_error_t(header.length()));
                break;

            case CQL_OPCODE_RESULT:
                _response_message.reset(new cql::cql_message_result_t(header.length()));
                break;

            case CQL_OPCODE_SUPPORTED:
                _response_message.reset(new cql::cql_message_supported_t(header.length()));
                break;

            case CQL_OPCODE_READY:
                _response_message.reset(new cql::cql_message_ready_t(header.length()));
                break;

            case CQL_OPCODE_EVENT:
                _response_message.reset(new cql::cql_message_event_t(header.length()));
                break;

            default:
                // need some bucket to put the data so we can get past the unkown
                // body in the stream it will be discarded by the body_read_handle
                _response_message = cql::cql_message_result_t(header.length());
                break;
            }

            boost::asio::async_read(*_transport,
                                    _response_message.buffer(),
                                    boost::asio::transfer_exactly(header.length()),
                                    boost::bind(&cql_client_impl_t<cql_transport_t>::body_read_handle, this, header, boost::asio::placeholders::error));
        }


        void
        body_read_handle(const cql::cql_header_impl_t& header,
                         const boost::system::error_code& err)
        {
            log(CQL_LOG_DEBUG, "received body for message " + header.str());

            if (!err) {
                switch (header.opcode()) {

                case CQL_OPCODE_RESULT:
                    _response_message.consume();
                    log(CQL_LOG_DEBUG, "received result message " + header.stream());

                    callback_map_t::iterator it = _callback_map.find(header.stream());
                    if (it != _callback_map.end()) {
                        (*it).second.first(*this, header.stream(), _response_message);
                        _callback_map.erase(it);
                    }
                    else {
                        log(CQL_LOG_ERROR, "no callback found for message " + header.str());
                    }
                    break;

                case CQL_OPCODE_EVENT:
                    log(CQL_LOG_DEBUG, "received event message");

                    // _response_message.consume();
                    // std::auto_ptr<cql::cql_message_event_t> m(cql::read_cql_event(response_stream));

                    // if (_event_callback) {
                    //     _event_callback(*this, *m);
                    // }
                    break;

                case CQL_OPCODE_ERROR:
                    _response_message.consume();
                    callback_map_t::iterator it = _callback_map.find(header.stream());
                    if (it != _callback_map.end()) {
                        cql_error_t err(true, m.code(), 0, m.message());
                        (*it).second.second(*this, header.stream(), err);
                        _callback_map.erase(it);
                    }
                    else {
                        log(CQL_LOG_INFO, "no callback found for message " + header.str());
                    }
                    break;

                case CQL_OPCODE_READY:
                    log(CQL_LOG_DEBUG, "received ready message");
                    if (!_events_registered) {
                        events_register();
                    }
                    else  {
                        _ready = true;
                        if (_connect_callback) {
                            // let the caller know that the connection is ready
                            _connect_callback(*this);
                        }
                    }
                    break;

                case CQL_OPCODE_SUPPORTED:
                    log(CQL_LOG_DEBUG, "received supported message " + m.str());
                    startup_write();
                    break;

                case CQL_OPCODE_AUTHENTICATE:
                    credentials_write();
                    break;

                default:
                    log(CQL_LOG_ERROR, "unhandled opcode " + header.str());
                }
            }
            else {
                log(CQL_LOG_ERROR, "error reading body " + err.message());
                check_transport_err(err);
            }
            header_read(); // loop
        }

        void
        events_register()
        {
            std::auto_ptr<cql::cql_message_t> m(new cql::cql_message_register_t());
            m->events(_events);
            create_request(m.release(),
                           boost::bind(&cql_client_impl_t::write_handle,
                                       this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
            _events_registered = true;
        }

        void
        options_write()
        {
            create_request(new cql::cql_message_options_t(),
                           (boost::function<void (const boost::system::error_code &, std::size_t)>)boost::bind(&cql_client_impl_t::write_handle,
                                                                                                               this,
                                                                                                               boost::asio::placeholders::error,
                                                                                                               boost::asio::placeholders::bytes_transferred));

            // start listening
            header_read();
        }

        void
        startup_write()
        {
            std::auto_ptr<cql::cql_message_t> m(new cql::cql_message_startup_t());
            m->version(CQL_VERSION_IMPL);
            create_request(m.release(),
                           boost::bind(&cql_client_impl_t::write_handle,
                                       this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
        }

        void
        credentials_write()
        {
            std::auto_ptr<cql::cql_message_t> m(new cql::cql_message_credentials_t());
            m->credentials(_credentials);
            create_request(m,
                           boost::bind(&cql_client_impl_t::write_handle,
                                       this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
        }

        inline void
        check_transport_err(const boost::system::error_code& err)
        {
            if (!_transport->lowest_layer().is_open()) {
                _ready = false;
                _defunct = true;
            }

            if (_connect_errback && !_closing) {
                cql_error_t e(false, 0, err.value(), err.message());
                _connect_errback(*this, e);
            }
        }

        typedef boost::tuple<boost::shared_ptr<cql::cql_header_impl_t>, boost::shared_ptr<cql::cql_message_t> > request_t;
        typedef std::list<request_t> request_buffer_t;
        typedef std::pair<cql_message_callback_t, cql_message_errback_t> callback_pair_t;
        typedef boost::unordered_map<cql::cql_stream_id_t, callback_pair_t> callback_map_t;
        typedef boost::function<void(const boost::system::error_code&, std::size_t)> write_callback_t;

        std::string                           _server;
        unsigned int                          _port;
        boost::asio::ip::tcp::resolver        _resolver;
        std::auto_ptr<cql_transport_t>        _transport;

        cql::cql_stream_id_t                  _stream_counter;
        request_buffer_t                      _request_buffer;
        cql::cql_header_impl_t                _request_header;
        boost::shared_ptr<cql::cql_message_t> _request_message;
        boost::asio::streambuf                _response_header_buffer;
        std::auto_ptr<cql::cql_message_t>     _response_message;
        callback_map_t                        _callback_map;

        cql_connection_callback_t             _connect_callback;
        cql_connection_errback_t              _connect_errback;
        cql_log_callback_t                    _log_callback;

        bool                                  _events_registered;
        std::list<std::string>                _events;
        cql_event_callback_t                  _event_callback;

        cql::cql_client_t::cql_credentials_t  _credentials;

        bool                                  _defunct;
        bool                                  _ready;
        bool                                  _closing;
    };

} // namespace cql

#endif // CQL_CLIENT_IMPL_H_
