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

#ifndef CQL_MESSAGE_RESULT_IMPL_H_
#define CQL_MESSAGE_RESULT_IMPL_H_

#include <boost/ptr_container/ptr_vector.hpp>

#include "libcql/cql.hpp"
#include "libcql/cql_message_result.hpp"
#include "libcql/internal/cql_message.hpp"
#include "libcql/internal/cql_result_metadata.hpp"
#include "libcql/internal/cql_row_impl.hpp"

namespace cql {

    class cql_message_result_impl_t :
        boost::noncopyable,
        public cql_message_t, cql_message_result_t
    {

    public:
        typedef  boost::ptr_vector<cql_row_t>    row_container_t;
        typedef  cql_row_t*                      value_type;
        typedef  cql_row_t&                      reference;
        typedef  const cql_row_t&                const_reference;
        typedef  row_container_t::iterator       iterator;
        typedef  row_container_t::const_iterator const_iterator;
        typedef  row_container_t::size_type      size_type;

        cql_message_result_impl_t(size_t size);

        cql_message_result_impl_t();

        cql_int_t
        result_type() const;

        cql::cql_opcode_enum
        opcode() const;

        cql_int_t
        size() const;

        std::string
        str() const;

        cql_int_t
        column_count() const;

        cql_int_t
        row_count() const;

        const std::vector<cql::cql_byte_t>&
        query_id() const;

        const cql_row_t&
        operator[](size_type n) const;

        const cql_row_t&
        at(size_type n) const;

        const_iterator
        begin() const;

        const_iterator
        end() const;

        const cql_result_metadata_t&
        metadata() const;

        bool
        consume(cql::cql_error_t& err);

        bool
        prepare(cql::cql_error_t& err);

        boost::shared_ptr<std::vector<cql::cql_byte_t> >
        buffer();

    private:
        std::vector<cql::cql_byte_t>    _buffer;
        std::vector<cql::cql_byte_t>    _query_id;
        cql_int_t                       _result_type;
        cql_int_t                       _row_count;
        std::string                     _keyspace_name;
        std::string                     _table_name;
        cql::cql_result_metadata_t      _metadata;
        row_container_t                 _rows;
    };

} // namespace cql

#endif // CQL_MESSAGE_RESULT_IMPL_H_
