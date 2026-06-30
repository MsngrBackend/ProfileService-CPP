#ifndef MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_
#define MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_

// This file is now just a convenience header that includes the new interface
#include "db_interface.hpp"
#include "postgres_connection.hpp"

// Keep the old class names for backward compatibility if needed
namespace msngr::profile::repository {

using IDatabaseConnection = IDatabaseConnection;
using PostgresSQLConnection = PostgresConnection;

} // namespace msngr::profile::repository

#endif // MSNGR__PROFILE__REPOSITORY__DB_CONNECTION_HPP_