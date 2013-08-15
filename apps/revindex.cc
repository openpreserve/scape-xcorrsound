// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2012, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#include <iostream>
#include <unistd.h>
#include <tpie/pipelining.h>
#include <tpie/file_stream.h>
#include <tpie/tempname.h>

namespace TP = tpie;
namespace P = tpie::pipelining;

template <typename dest_t>
class stdin_reader_t : public P::node {
    dest_t dest;
    const static size_t bufSize = 1 << 20;

public:
    stdin_reader_t(const dest_t & dest)
        : dest(dest)
    {
        add_push_destination(dest);
        set_minimum_memory(bufSize);
    }

    virtual void go() {
        TP::array<uint32_t> buf(bufSize);
        while (true) {
            int nRead = read(0, &buf[0], bufSize*sizeof(uint32_t));
            if (nRead == 0) break;
            for (size_t i = 0; i < nRead/sizeof(uint32_t); ++i) dest.push(buf[i]);
        }
    }
};

P::pipe_begin<P::factory_0<stdin_reader_t> >
stdin_reader() {
    return P::factory_0<stdin_reader_t>();
}

template <typename dest_t>
class annotator_t : public P::node {
    dest_t dest;
    size_t idx;

public:
    annotator_t(const dest_t & dest)
        : dest(dest)
        , idx(0)
    {
        add_push_destination(dest);
    }

    void push(uint32_t c) override {
        dest.push(std::make_pair(c, idx++));
    }
};

P::pipe_middle<P::factory_0<annotator_t> >
annotator() {
    return P::factory_0<annotator_t>();
}

class indexer_t : public P::node {
    uint32_t c;
    TP::file_stream<size_t> * positions;
    TP::file_stream<size_t> * lists;

public:
    typedef std::pair<uint32_t, size_t> item_type;

    indexer_t()
        : c(0)
    {
        set_minimum_memory(2 * TP::file_stream<size_t>::memory_usage());
    }

    virtual void begin() override {
        positions = TP::tpie_new<TP::file_stream<size_t> >();
        positions->open("/media/jsn/Big_disk/positions");
        positions->truncate(0);
        lists = TP::tpie_new<TP::file_stream<size_t> >();
        lists->open("/media/jsn/Big_disk/lists");
        lists->truncate(0);
        lists->write(0);
    }

    void push(const std::pair<uint32_t, size_t> & el) {
        while (el.first > c) {
            ++c;
            lists->write(positions->offset());
        }
        positions->write(el.second);
    }

    virtual void end() override {
        do {
            lists->write(positions->offset());
            ++c;
        } while (c != 0);
        TP::tpie_delete(lists);
        TP::tpie_delete(positions);
    }
};

P::pipe_end<P::termfactory_0<indexer_t> >
indexer() {
    return P::termfactory_0<indexer_t>();
}

int main() {
    size_t mem_limit = 1024*1024; mem_limit *= 1024*2;
    TP::tpie_init();
    TP::get_memory_manager().set_limit(mem_limit);
    {
        P::pipeline p
            = stdin_reader()
            | annotator()
            | P::pipesort()
            | indexer();
        p.plot(std::clog);
        p();
    }
    TP::tpie_finish();
    return 0;
}
