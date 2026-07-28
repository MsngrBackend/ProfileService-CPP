#ifndef MSNGR__PROFILE__UTILS__LOGGER_HPP_
#define MSNGR__PROFILE__UTILS__LOGGER_HPP_

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_logger.hpp>

namespace msngr::utils {

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;

using logger_t = src::severity_logger_mt<logging::trivial::severity_level>;

BOOST_LOG_INLINE_GLOBAL_LOGGER_INIT(my_logger, logger_t) {
  logger_t lg;

  logging::core::get()->set_filter(
    logging::trivial::severity >= logging::trivial::info
  );

  logging::add_console_log(std::cout,
    keywords::format = "[%TimeStamp%] %Severity%: %Message%"
  );

  logging::add_common_attributes();
  return lg;
}

#define LOG(sev) BOOST_LOG_SEV(msngr::utils::my_logger::get(), boost::log::trivial::sev)

} // namespace msngr::utils

#endif // MSNGR__PROFILE__UTILS__LOGGER_HPP_