TEMPLATES=cgi.cc impl.cc impl.h impl_docgi.cc
SHAREDIR=/usr/share/cerverpages

install: 
	if [ ! -d $(SHAREDIR) ];then mkdir $(SHAREDIR);fi
	install -m644 $(TEMPLATES) $(SHAREDIR)

uninstall:
	for f in $(TEMPLATES); do rm -vf $(SHAREDIR)/$$f;done;
