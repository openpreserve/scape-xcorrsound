#include <tpie/tpie.h>
#include <tpie/file_stream.h>
#include <tpie/progress_indicator_arrow.h>
#include <tpie/fractional_progress.h>
#include <tpie/tpie_log.h>
#include <iostream>

int main() {
    tpie::tpie_init();

    {
	size_t n=1024*1024*128;
	
	tpie::progress_indicator_arrow pi("", n);
	
	tpie::get_log().add_target(
	    new tpie::stderr_log_target(tpie::LOG_INFORMATIONAL) );

	tpie::fractional_progress fp(&pi);
	tpie::fractional_subindicator write_p(fp, "write", TPIE_FSI, n, "writing");
	tpie::fractional_subindicator read_p(fp, "read", TPIE_FSI, n, "reading");
	fp.init();
	
	tpie::log_info() << "Writing" << std::endl;
	tpie::log_debug() << "Some bytes "  << n << std::endl;

	write_p.init(n);
	{
	    tpie::file_stream<size_t> kat;
	    kat.open("kat.tpie", tpie::access_write);
	    for (size_t i=0; i < n; ++i) {
		kat.write(i);
		write_p.step();
	    }
	}
	write_p.done();

	tpie::log_info() << "Reading" << std::endl;
	
	read_p.init(n);
	{
	    tpie::file_stream<size_t> kat;
	    kat.open("kat.tpie", tpie::access_read);
	    size_t a=0;
	    for (size_t i=0; i < n; ++i) {
		a += kat.read();
		read_p.step();
	    }
	    std::cout << a << std::endl;
	}
	read_p.done();
	
	fp.done();
	
    }

    tpie::tpie_finish();
}
