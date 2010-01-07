/**
 * POSIX lock wrapping class
 */

#ifndef _COND_LOCK_H_
#define _COND_LOCK_H_

#include <config.h>

#include <pthread.h>
#include "Lock.h"

class CondLock: public Lock {
	pthread_cond_t *condSend;
	pthread_cond_t *condRecv;
public:
	CondLock();
	~CondLock();

	void waitSend() { pthread_cond_wait(condSend, mutex); }
	void signalSend() { pthread_cond_broadcast(condSend); }
	void waitRecv() { pthread_cond_wait(condRecv, mutex); }
	void signalRecv() { pthread_cond_broadcast(condRecv); }

/*	pthread_cond_t *getCondSend();
	pthread_cond_t *getCondRecv();
	void setCondSend(pthread_cond_t *condSend, bool free);
	void setCondRecv(pthread_cond_t *condRecv, bool free);*/
};

inline CondLock::CondLock() {
	condSend = new pthread_cond_t;
	condRecv = new pthread_cond_t;
	pthread_cond_init(condSend, NULL);
	pthread_cond_init(condRecv, NULL);
}

inline CondLock::~CondLock() {
	pthread_cond_destroy(condSend);
	pthread_cond_destroy(condRecv);
	delete condSend;
	delete condRecv;
}

/*inline pthread_cond_t *CondLock::getCondSend() {
	return condSend;
}

inline pthread_cond_t *CondLock::getCondRecv() {
	return condRecv;
}

inline void CondLock::setCondSend(pthread_cond_t *condSend, bool free) {
	if (this->condSend != condSend) {
		if (free)
			delete this->condSend;
		this->condSend = condSend;
	}
}

inline void CondLock::setCondRecv(pthread_cond_t *condRecv, bool free) {
	if (this->condRecv != condRecv) {
		if (free)
			delete this->condRecv;
		this->condRecv = condRecv;
	}
}*/

#endif
