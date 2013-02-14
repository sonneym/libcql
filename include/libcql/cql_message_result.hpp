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

#ifndef CQL_MESSAGE_RESULT_H_
#define CQL_MESSAGE_RESULT_H_

#include <vector>
#include "libcql/cql.hpp"

namespace cql {

    class cql_row_t;
    class cql_result_metadata_t;

    class cql_message_result_t
    {

    public:
        virtual cql_int_t
        result_type() const = 0;

        virtual cql::cql_opcode_enum
        opcode() const = 0;

        virtual cql_int_t
        size() const = 0;

        virtual std::string
        str() const = 0;

        virtual cql_int_t
        column_count() const = 0;

        virtual cql_int_t
        row_count() const = 0;

        virtual const std::vector<cql::cql_byte_t>&
        query_id() const = 0;

        virtual const cql_row_t&
        operator[](size_t n) const = 0;

        virtual const cql_result_metadata_t&
        metadata() const = 0;
    };

} // namespace cql

#endif // CQL_MESSAGE_RESULT_H_
