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

#include <iomanip>
#include <boost/lexical_cast.hpp>
#include "libcql/internal/cql_defines.hpp"
#include "libcql/internal/cql_util.hpp"

#include "libcql/internal/cql_header.hpp"

cql::cql_header_t::cql_header_t() :
    _version(0),
    _flags(0),
    _stream(0),
    _opcode(0),
    _length(0)
{}

cql::cql_header_t::cql_header_t(cql::cql_byte_t version,
                                          cql::cql_byte_t flags,
                                          cql::cql_stream_id_t stream,
                                          cql::cql_byte_t opcode,
                                          cql::cql_int_t length) :
    _version(version),
    _flags(flags),
    _stream(stream),
    _opcode(opcode),
    _length(length)
{}

std::string
cql::cql_header_t::str() const
{
    std::stringstream output;
    output << std::setfill('0');
    output << "{version: 0x" << std::setw(2) << cql::hex(_version);
    output << ", flags: 0x" << std::setw(2) << cql::hex(_flags);
    output << ", stream: 0x" << std::setw(2) << cql::hex(_stream);
    output << ", opcode: 0x" << std::setw(2) << cql::hex(_opcode);
    output << ", length: " << boost::lexical_cast<std::string>(_length) << "}";
    return output.str();
}

std::ostream&
cql::cql_header_t::write(std::ostream& output) const
{
    output.put(_version);
    output.put(_flags);
    output.put(_stream);
    output.put(_opcode);

    cql::cql_int_t l = htonl(_length);
    output.write(reinterpret_cast<char*>(&l), sizeof(l));
    return output;
}

std::istream&
cql::cql_header_t::read(std::istream& input)
{
    _version = input.get();
    _flags = input.get();
    _stream = input.get();
    _opcode = input.get();
    input.read(reinterpret_cast<char*>(&_length), sizeof(_length));
    _length = ntohl(_length);
    return input;
}

cql::cql_int_t
cql::cql_header_t::size() const
{
    return sizeof(_version) + sizeof(_flags) + sizeof(_stream) + sizeof(_opcode) + sizeof(_length);
}

cql::cql_byte_t
cql::cql_header_t::version() const
{
    return _version;
}

cql::cql_byte_t
cql::cql_header_t::flags() const
{
    return _flags;
}

cql::cql_stream_id_t
cql::cql_header_t::stream() const
{
    return _stream;
}

cql::cql_byte_t
cql::cql_header_t::opcode() const
{
    return _opcode;
}

cql::cql_int_t
cql::cql_header_t::length() const
{
    return _length;
}

void
cql::cql_header_t::version(cql::cql_byte_t v)
{
    _version = v;
}

void
cql::cql_header_t::flags(cql::cql_byte_t v)
{
    _flags = v;
}

void
cql::cql_header_t::stream(cql::cql_stream_id_t v)
{
    _stream = v;
}

void
cql::cql_header_t::opcode(cql::cql_byte_t v)
{
    _opcode = v;
}

void
cql::cql_header_t::length(cql::cql_int_t v)
{
    _length = v;
}
