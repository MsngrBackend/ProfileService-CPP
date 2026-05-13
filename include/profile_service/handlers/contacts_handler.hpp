#ifndef MSNGR__PROFILE__HANDLERS__CONTACTS_HANDLER_HPP_
#define MSNGR__PROFILE__HANDLERS__CONTACTS_HANDLER_HPP_

#include "profile_service/handlers/ihandler.hpp"
#include "profile_service/repository/repository.hpp"
#include <memory>

namespace msngr::profile::handlers {

class ContactsHandler : public IHandler {
public:
  explicit ContactsHandler(std::shared_ptr<repository::ContactsRepository> contactsRepo);

  HttpResponse Handle(const HttpRequest & request, const RouteContext & context) override;

private:
  HttpResponse HandleGetContacts(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleAddContact(const HttpRequest & request, const RouteContext & context);
  HttpResponse HandleDeleteContact(const HttpRequest & request, const RouteContext & context);

private:
  std::shared_ptr<repository::ContactsRepository> m_contactsRepo;
};

} // namespace msngr::profile::handlers

#endif  // MSNGR__PROFILE__HANDLERS__CONTACTS_HANDLER_HPP_