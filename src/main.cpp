#include <boost/asio.hpp>
#include <boost/config.hpp>
#include <boost/predef.h>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <iostream>

#if defined(BOOST_OS_WINDOWS_AVAILABLE)
#  include "file_writer_win32.hpp"
static std::vector<file_writer_win32::pointer> file_writers;
#else
#  include "file_writer_posix.hpp"
static std::vector<file_writer_posix::pointer> file_writers;
#endif

static boost::asio::io_context io_context;
static boost::thread_group threads;

int
main (int argc, char *argv[])
{
  boost::program_options::options_description description ("All options");

  description.add_options () ("help,h", "Produce help message") (
      "append,a", "Appends the output to each file") (
      "file,f", boost::program_options::value<std::vector<std::string> > (),
      "A list of files, each of which receives the output");

  boost::program_options::positional_options_description
      positional_description;
  positional_description.add ("file", -1);

  boost::program_options::variables_map variable_map;

  boost::program_options::store (
      boost::program_options::command_line_parser (argc, argv)
          .options (description)
          .positional (positional_description)
          .run (),
      variable_map);

  boost::program_options::notify (variable_map);

  if (variable_map.count ("help"))
    {
      std::cout << description << std::endl;
      return EXIT_FAILURE;
    }

  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard (boost::asio::make_work_guard (io_context));

  const std::vector<std::string> *file_list = NULL;

  if (variable_map.count ("file"))
    {
      file_list = &variable_map["file"].as<std::vector<std::string> > ();

      std::size_t threads_count = boost::thread::hardware_concurrency ();
      if (file_list->size () < threads_count)
        {
          threads_count = file_list->size ();
        }

      threads.create_thread (
          boost::bind (&boost::asio::io_context::run, &io_context));

      for (std::vector<std::string>::size_type i = 0; i < file_list->size ();
           i++)
        {
          file_writers.push_back (
#if defined(BOOST_OS_WINDOWS_AVAILABLE)
              file_writer_win32::pointer (new file_writer_win32 (io_context))
#else
              file_writer_posix::pointer (new file_writer_posix (io_context))
#endif
          );

          file_writers.back ()->open ((*file_list)[i],
                                      variable_map.count ("append"));
        }
    }

  boost::asio::streambuf buffer;

  std::cin.rdbuf ()->pubsetbuf (0, 0);
  std::cout.rdbuf ()->pubsetbuf (0, 0);

  while (true)
    {
      if (std::cin.rdbuf ()->in_avail () > 0)
        {
          boost::asio::mutable_buffer b = buffer.prepare (
              static_cast<std::size_t> (std::cin.rdbuf ()->in_avail ()));

          buffer.commit (static_cast<std::size_t> (std::cin.readsome (
              reinterpret_cast<char *> (b.data ()), b.size ())));
        }
      else if (buffer.size () == 0)
        {
          if (!std::cin.eof ())
            {
              boost::asio::mutable_buffer b = buffer.prepare (1024);

              std::cin.read (reinterpret_cast<char *> (b.data ()), b.size ());

              if (std::cin.fail () && !std::cin.eof ())
                {
                  // To process the error!
                  break;
                }

              buffer.commit (static_cast<std::size_t> (std::cin.gcount ()));
            }
          else
            {
              break;
            }
        }

      boost::asio::const_buffer for_send = buffer.data ();

      if (file_list)
        {
          for (std::vector<std::string>::size_type i = 0;
               i < file_list->size (); i++)
            {
#if defined(BOOST_OS_WINDOWS_AVAILABLE)
// ...
#else
              file_writers[i]->post_write (for_send);
#endif
            }
        }

      std::cout.write (reinterpret_cast<const char *> (for_send.data ()),
                       for_send.size ());

      buffer.consume (buffer.size ());
    }

  work_guard.reset ();

  if (!file_writers.empty ())
    {
      threads.join_all ();
    }

  file_writers.clear ();

  return EXIT_SUCCESS;
}