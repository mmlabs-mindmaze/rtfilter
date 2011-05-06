ACLOCAL_AMFLAGS=-I m4
SUBDIRS = src doc tests

doc_DATA = AUTHORS NEWS README COPYING

EXTRA_DIST = debian/changelog debian/compat debian/control\
		debian/copyright debian/docs		\
		debian/postinst.ex debian/postrm.ex	\
		debian/preinst.ex debian/prerm.ex	\
		debian/rules				\
		debian/librtfilter1.install		\
		debian/librtfilter1-dev.install		

REMOVE_DISTDIR = \
  { test ! -d "$(distdir)" \
    || { find "$(distdir)" -type d ! -perm -200 -exec chmod u+w {} ';' \
         && rm -fr "$(distdir)"; }; }

dist-deb: distdir
	am__cwd=`pwd` \
	&& cd $(distdir) \
	&& CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" FFLAGS="$(FFLAGS)" CPPFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS)" dpkg-buildpackage \
	&& cd "$$am__cwd" 
	$(REMOVE_DISTDIR)
