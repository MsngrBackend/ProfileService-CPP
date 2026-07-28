#ifndef MSNGR__PROFILE__SERVICES__CONTACTS_SERVICE_HPP_
#define MSNGR__PROFILE__SERVICES__CONTACTS_SERVICE_HPP_

#include "profile_service/services/iservice.hpp"
#include "profile_service/persistence/repositories/contacts_repository.hpp"

#include <memory>

namespace msngr::profile::services {

class ContactsService : public IService {
public:
  explicit ContactsService(std::shared_ptr<repository::ContactsRepository> contactsRepo);

  HttpResponse Execute(const HttpRequest & request, const ServiceContext & context) override;

private:
  HttpResponse HandleGetContacts(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleAddContact(const HttpRequest & request, const ServiceContext & context);
  HttpResponse HandleDeleteContact(const HttpRequest & request, const ServiceContext & context);

  std::shared_ptr<repository::ContactsRepository> m_contactsRepo;
};

} // namespace msngr::profile::services

#endif // MSNGR__PROFILE__SERVICES__CONTACTS_SERVICE_HPP_