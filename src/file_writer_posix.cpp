#include "file_writer_posix.hpp"
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/bind.hpp>
#include <fcntl.h>

file_writer_posix::file_writer_posix (boost::asio::io_context &io_context)
    : strand_ (boost::asio::make_strand (io_context)), file_descriptor_ (-1)
{
}

file_writer_posix::~file_writer_posix () { ::close (file_descriptor_); }

void
file_writer_posix::open (const std::string &filepath, bool append)
{
  void (file_writer_posix::*open_func) (const std::string &)
      = append ? &file_writer_posix::open_file_append
               : &file_writer_posix::open_file_trunc;

  boost::asio::post (boost::asio::bind_executor (
      strand_,
      boost::bind (open_func, shared_from_this (), boost::ref (filepath))));
}

void
file_writer_posix::post_write (boost::asio::const_buffer buffer)
{
  boost::asio::post (boost::asio::bind_executor (
      strand_,
      boost::bind (&file_writer_posix::advance, shared_from_this (), buffer)));
}

void
file_writer_posix::open_file_trunc (const std::string &filepath)
{
  file_descriptor_ = ::open (filepath.c_str (), O_WRONLY | O_CREAT | O_TRUNC,
                             S_IRUSR | S_IWUSR);
  if (file_descriptor_ == -1)
    {
      ::fprintf (stderr, ::strerror (errno));
    }
}

void
file_writer_posix::open_file_append (const std::string &filepath)
{
  file_descriptor_ = ::open (filepath.c_str (), O_WRONLY | O_CREAT | O_APPEND,
                             S_IRUSR | S_IWUSR);
  if (file_descriptor_ == -1)
    {
      ::fprintf (stderr, ::strerror (errno));
    }
}

void
file_writer_posix::advance (boost::asio::const_buffer buffer)
{
  if (file_descriptor_ != -1)
    {
      if (::write (file_descriptor_, buffer.data (), buffer.size ())
          != buffer.size ())
        {
          //::fprintf (stderr, ::strerror (errno));
        }
    }
}