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

#ifndef CQL_MESSAGE_PREPARE_H_
#define CQL_MESSAGE_PREPARE_H_

#include "libcql/cql.hpp"
#include "libcql/cql_message.hpp"

namespace cql {

    class cql_message_prepare_t :
        public cql_message_t
    {

    public:

        cql_message_prepare_t();

        cql_message_prepare_t(const std::string& query);

        const std::string&
        prepare() const;

        const std::string&
        query() const;

        void
        query(const std::string& q);

        cql::cql_byte_t
        opcode() const;

        cql_int_t
        size() const;

        std::string
        str() const;

        std::istream&
        read(std::istream& input);

        std::ostream&
        write(std::ostream& output) const;

    private:
        std::string _query;
    };

} // namespace cql

#endif // CQL_MESSAGE_PREPARE_H_
