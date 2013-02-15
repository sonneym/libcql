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

#ifndef CQL_MESSAGE_SUPPORTED_IMPL_H_
#define CQL_MESSAGE_SUPPORTED_IMPL_H_

#include <sstream>

#include "libcql/cql.hpp"
#include "libcql/internal/cql_message.hpp"

#include <boost/algorithm/string/join.hpp>

namespace cql {

    class cql_message_supported_impl_t :
        public cql_message_t
    {

    public:

        cql_message_supported_impl_t();

        cql_message_supported_impl_t(size_t size);

        void
        compressions(const std::list<std::string>& c);

        const std::list<std::string>&
        compressions() const;

        void
        versions(const std::list<std::string>& v);

        const std::list<std::string>&
        version() const;

        cql::cql_opcode_enum
        opcode() const;

        cql_int_t
        size() const;

        std::string
        str() const;

        bool
        consume(cql::cql_error_t& err);

        bool
        prepare(cql::cql_error_t& err);

        boost::shared_ptr<std::vector<cql::cql_byte_t> >
        buffer();

    private:
        std::vector<cql::cql_byte_t> _buffer;
        std::list<std::string>       _versions;
        std::list<std::string>       _compressions;
    };

} // namespace cql

#endif // CQL_MESSAGE_SUPPORTED_IMPL_H_
