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

#ifndef CQL_MESSAGE_EXECUTE_H_
#define CQL_MESSAGE_EXECUTE_H_

#include <vector>
#include "libcql/cql.hpp"

namespace cql {
    class cql_message_execute_impl_t;

    class cql_message_execute_t
    {

    public:
        typedef  std::vector<cql::cql_byte_t>       param_t;
        typedef  boost::ptr_list<param_t>           params_container_t;
        typedef  param_t*                           value_type;
        typedef  param_t&                           reference;
        typedef  const param_t&                     const_reference;
        typedef  params_container_t::iterator       iterator;
        typedef  params_container_t::const_iterator const_iterator;
        typedef  params_container_t::size_type      size_type;

        virtual const std::vector<cql::cql_byte_t>&
        query_id() const = 0;

        virtual void
        query_id(const std::vector<cql::cql_byte_t>& id) = 0;

        virtual cql::cql_short_t
        consistency() const = 0;

        virtual void
        consistency(const cql::cql_short_t consistency) = 0;

        virtual void
        push_back(const param_t& val) = 0;

        virtual void
        push_back(const std::string& val) = 0;

        virtual void
        push_back(const cql::cql_short_t val) = 0;

        virtual void
        push_back(const cql_int_t val) = 0;

        virtual void
        push_back(const cql::cql_bigint_t val) = 0;

        virtual void
        push_back(const float val) = 0;

        virtual void
        push_back(const double val) = 0;

        virtual void
        push_back(const bool val) = 0;

        virtual void
        pop_back() = 0;

        virtual const_iterator
        begin() const = 0;

        virtual const_iterator
        end() const = 0;

        virtual cql::cql_opcode_enum
        opcode() const = 0;

        virtual cql_int_t
        size() const = 0;

        virtual std::string
        str() const = 0;

        virtual cql_message_execute_impl_t*
        impl();
    };

} // namespace cql

#endif // CQL_MESSAGE_EXECUTE_H_
