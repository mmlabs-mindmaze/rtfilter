ACLOCAL_AMFLAGS=-I m4
SUBDIRS = src doc tests

doc_DATA = AUTHORS NEWS README COPYING

EXTRA_DIST = debian/changelog debian/compat debian/control\
		debian/copyright debian/symbols		\
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
	&& dpkg-buildpackage $(DPKG_BUILDPACKAGE_FLAGS) \
	&& cd "$$am__cwd" 
	$(REMOVE_DISTDIR)
