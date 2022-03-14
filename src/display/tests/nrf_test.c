#include "rpi.h"
#include "nrf.h"
//#include "nrf-test.h"
#include "rf-support.h"

// server pins are:
// CE = 21
// CSN = 7
// SCK = 11
// MO = 10
// MI = 9

#define ADDR 0xd5d5d5
#define OTHER_ADDR 0xe5e5e5
#define CHANNEL (11+35)

#define SEND_DATA 0

#ifdef SEND_DATA
const unsigned addr = ADDR;
#else
const unsigned addr = OTHER_ADDR;
#endif

#ifdef SEND_DATA
const unsigned other_addr = OTHER_ADDR;
#else
const unsigned other_addr = ADDR;
#endif

#define RETRAN 2
#define RETRAN_DELAY 250

nrf_t* mk_config(unsigned nbytes) {
	//nrf_config_t c = nrf_conf_reliable_mk(RETRAN, RETRAN_DELAY);
	nrf_config_t c = nrf_conf_unreliable_mk();
	c = nrf_conf_spi(c, 1);
	c = nrf_conf_ce(c, 21);
	c = nrf_set_Mhz_delta(c, CHANNEL);

	return nrf_init_noacked(c, addr, nbytes);
}

static inline msg_send_t mk_test_msg(uint32_t i, unsigned nbytes) {
	char* test_msg = "0123456789abcdefghijklmnopqrstuv";

	msg_send_t m = msg_send_mk(nbytes);
	msg_put_n(&m, &test_msg, nbytes);
	return m;
}

void test_messages(nrf_t* nic) {
	const unsigned TIMEOUT = 1000000;

    unsigned i = 5;
    while (1) {
		unsigned nbytes = i % 32 + 1;

		// send data
		#if SEND_DATA

		nrf_send_msg_noack(nic, other_addr, mk_test_msg(i, nbytes));

		#else

		// receive data
		nrf_pipe_t* pipe = nrf_get_pipe(nic, 0);
		msg_recv_t m;
		if (nrf_get_msg_exact_timeout(pipe, &m, nbytes, TIMEOUT) == nbytes) {
			// convert data
			assert(msg_nbytes_get(&m) == nbytes);
			char x[32];
			msg_get_n(&x, &m, nbytes);
			x[nbytes] = 0;

			// print
			output("%s\n", x);
		}
		else {
			// failed
			output("bad\n");
		}

		#endif
	}
}

void notmain(void) {
	unsigned nbytes = 1;

	nrf_t* nic = mk_config(nbytes);

	test_messages(nic);
}
