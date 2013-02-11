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

#ifndef CQL_MESSAGE_EVENT_TOPOLOGY_IMPL_H_
#define CQL_MESSAGE_EVENT_TOPOLOGY_IMPL_H_

#include <istream>
#include <ostream>
#include <string>
#include <boost/noncopyable.hpp>

#include "libcql/cql_message_event_impl.hpp"

namespace cql {

    class cql_message_event_topology_impl_t :
        boost::noncopyable,
        public cql_message_event_impl_t
    {

    public:

        cql_message_event_topology_impl_t();

        cql_message_event_topology_impl_t(cql_event_topology_enum direction,
                                          const std::string& ip,
                                          const cql_int_t port);

        cql::cql_opcode_enum
        opcode() const;

        cql_int_t
        size() const;

        std::istream&
        read(std::istream&);

        std::ostream&
        write(std::ostream&) const;

        std::string
        str() const;

        cql_event_enum
        event_type() const;

        cql_event_topology_enum
        direction() const;

        const std::string&
        ip() const;

        cql_int_t
        port() const;

    private:
        cql_event_topology_enum  _direction;
        std::string              _ip;
        cql_int_t                _port;
    };

} // namespace cql

#endif // CQL_MESSAGE_EVENT_TOPOLOGY_IMPL_H_
