#include "profile_service/repository/contacts_repository.hpp"

namespace msngr::profile::repository {

ContactsRepositoryPostgres::ContactsRepositoryPostgres(std::shared_ptr<IDatabaseConnection> conn)
  : m_connection(std::move(conn)) {}

std::vector<domain::Contact> ContactsRepositoryPostgres::List(const std::string & ownerId) {
  pqxx::work txn(m_connection->Connection());

  auto result = txn.exec_params(
    "SELECT owner_id, contact_id, alias, created_at FROM contacts "
    "WHERE owner_id = $1 ORDER BY created_at DESC",
    ownerId
  );

  std::vector<domain::Contact> contacts;
  for (const auto& row : result) {
    domain::Contact contact;
    contact.OwnerID = row["owner_id"].as<std::string>();
    contact.ContactID = row["contact_id"].as<std::string>();
    if (!row["alias"].is_null()) {
      contact.Alias = row["alias"].as<std::string>();
    }
    contacts.push_back(contact);
  }

  return contacts;
}

void ContactsRepositoryPostgres::Add(const domain::Contact & contact) {
  pqxx::work txn(m_connection->Connection());

  txn.exec_params(
    "INSERT INTO contacts (owner_id, contact_id, alias) "
    "VALUES ($1, $2, $3) "
    "ON CONFLICT (owner_id, contact_id) DO UPDATE SET alias = $3",
    contact.OwnerID, contact.ContactID, contact.Alias
  );
  txn.commit();
}

void ContactsRepositoryPostgres::Remove(const std::string & ownerId, const std::string & contactId) {
  pqxx::work txn(m_connection->Connection());

  txn.exec_params(
    "DELETE FROM contacts WHERE owner_id = $1 AND contact_id = $2",
    ownerId, contactId
  );
  txn.commit();
}

} // namespace msngr::profile::repository