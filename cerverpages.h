#ifndef __CERVERPAGES_H
#define __CERVERPAGES_H

class CerverPages {
public:
	CerverPages () {
	};
	virtual ~CerverPages () {
	};
	virtual void init () {};
	virtual void docgi () {};
	virtual void shutdown () {};
};

#endif
