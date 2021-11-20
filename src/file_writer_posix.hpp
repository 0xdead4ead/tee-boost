#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class file_writer_posix
    : public boost::enable_shared_from_this<file_writer_posix>,
      private boost::noncopyable
{
public:
  typedef boost::shared_ptr<file_writer_posix> pointer;

  explicit file_writer_posix (boost::asio::io_context &io_context);
  ~file_writer_posix ();

  void open (const std::string &filepath, bool append);
  void post_write (boost::asio::const_buffer buffer);

private:
  void open_file_trunc (const std::string &filepath);
  void open_file_append (const std::string &filepath);
  void advance (boost::asio::const_buffer buffer);

  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  int file_descriptor_;
};