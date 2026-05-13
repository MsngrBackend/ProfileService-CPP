#include "contacts_repository.hpp"

namespace msngr::profile::repository {

ContactsRepositoryPostgres::ContactsRepositoryPostgres(std::shared_ptr<pqxx::connection> conn)
    : conn_(std::move(conn)) {}

std::vector<domain::Contact> ContactsRepositoryPostgres::List(const std::string& owner_id) {
    pqxx::work txn(*conn_);
    
    auto result = txn.exec_params(
        "SELECT owner_id, contact_id, alias, created_at FROM contacts "
        "WHERE owner_id = $1 ORDER BY created_at DESC",
        owner_id
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

void ContactsRepositoryPostgres::Add(const domain::Contact& contact) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "INSERT INTO contacts (owner_id, contact_id, alias) "
        "VALUES ($1, $2, $3) "
        "ON CONFLICT (owner_id, contact_id) DO UPDATE SET alias = $3",
        contact.OwnerID, contact.ContactID, contact.Alias
    );
    txn.commit();
}

void ContactsRepositoryPostgres::Remove(const std::string& owner_id, const std::string& contact_id) {
    pqxx::work txn(*conn_);
    
    txn.exec_params(
        "DELETE FROM contacts WHERE owner_id = $1 AND contact_id = $2",
        owner_id, contact_id
    );
    txn.commit();
}

} // namespace msngr::profile::repository