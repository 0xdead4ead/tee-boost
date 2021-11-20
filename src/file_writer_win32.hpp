#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

class file_writer_win32
    : public boost::enable_shared_from_this<file_writer_win32>,
      private boost::noncopyable
{
public:
  typedef boost::shared_ptr<file_writer_win32> pointer;

  explicit file_writer_win32 (boost::asio::io_context &io_context,
                              const std::string &filepath, bool append);

private:
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
};