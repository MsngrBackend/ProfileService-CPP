#include "profile_service/persistence/repositories/contacts_repository.hpp"
#include "profile_service/persistence/repositories/mappers.hpp"

namespace msngr::profile::repository {

ContactsRepository::ContactsRepository(std::shared_ptr<QueryExecutor> executor)
  : m_executor(std::move(executor)) {}

std::vector<domain::Contact> ContactsRepository::List(const std::string & ownerId)
{
  const std::string query =
    "SELECT owner_id, contact_id, alias, created_at FROM contacts "
    "WHERE owner_id = $1 ORDER BY created_at DESC";

  auto result = m_executor->ExecuteSelect(query, {ownerId});

  std::vector<domain::Contact> contacts;
  if (result && !result->Empty()) {
    for (size_t i = 0; i < result->Size(); ++i) {
      auto row = result->GetRow(i);
      contacts.push_back(ContactMapper::MapRow(row.get()));
    }
  }

  return contacts;
}

void ContactsRepository::Add(const domain::Contact & contact)
{
  const std::string query =
    "INSERT INTO contacts (owner_id, contact_id, alias) "
    "VALUES ($1, $2, $3) "
    "ON CONFLICT (owner_id, contact_id) DO UPDATE SET alias = $3";

  m_executor->ExecuteModify(query, {
    contact.OwnerID,
    contact.ContactID,
    contact.Alias.value_or("")
  });
}

void ContactsRepository::Remove(const std::string & ownerId, const std::string & contactId)
{
  const std::string query =
    "DELETE FROM contacts WHERE owner_id = $1 AND contact_id = $2";

  m_executor->ExecuteModify(query, {ownerId, contactId});
}

} // namespace msngr::profile::repository