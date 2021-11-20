AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = tee-boost

tee_boost_SOURCES = main.cpp \
                    file_writer_posix.cpp

AM_CPPFLAGS = \
    -DBOOST_ASIO_DISABLE_VISIBILITY \
    -DBOOST_BIND_GLOBAL_PLACEHOLDERS \
    -DBOOST_THREAD_VERSION=4 \
    @BOOST_CPPFLAGS@ \
    -I$(srcdir)/..

LDADD = \
    @BOOST_PROGRAM_OPTIONS_LIB@ \
    @BOOST_ASIO_LIB@ \
    @BOOST_THREAD_LIB@

AM_LDFLAGS = @BOOST_LDFLAGS@