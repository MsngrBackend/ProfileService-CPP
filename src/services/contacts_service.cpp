#include "profile_service/services/contacts_service.hpp"
#include "profile_service/domain/models.hpp"
#include "profile_service/utils/logger.hpp"

#include <nlohmann/json.hpp>
#include <chrono>

namespace msngr::profile::services {

using json = nlohmann::json;

ContactsService::ContactsService(std::shared_ptr<repository::ContactsRepository> contactsRepo)
    : m_contactsRepo(std::move(contactsRepo)) {}

HttpResponse ContactsService::Execute(const HttpRequest& request, const ServiceContext& context) {
    auto method = request.method();
    auto target = std::string(request.target());

    if (method == boost::beast::http::verb::get && target == "/contacts") {
        return HandleGetContacts(request, context);
    }
    if (method == boost::beast::http::verb::post && target == "/contacts") {
        return HandleAddContact(request, context);
    }
    if (method == boost::beast::http::verb::delete_ && target.find("/contacts/") == 0) {
        return HandleDeleteContact(request, context);
    }

    return handlers::ErrorResponse(404, request.version(), "not_found");
}

HttpResponse ContactsService::HandleGetContacts(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto contacts = m_contactsRepo->List(context.UserID);

        json response = json::array();
        for (const auto& contact : contacts) {
            json item;
            item["owner_id"] = contact.OwnerID;
            item["contact_id"] = contact.ContactID;
            item["alias"] = contact.Alias.value_or("");
            item["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                contact.CreatedAt.time_since_epoch()).count();
            response.push_back(item);
        }

        return handlers::JsonResponse(200, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "GetContacts failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(500, request.version(), "failed_to_get_contacts");
    }
}

HttpResponse ContactsService::HandleAddContact(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto jsonBody = json::parse(request.body());

        std::string contactID = jsonBody.value("contact_id", "");
        if (contactID.empty()) {
            return handlers::ErrorResponse(400, request.version(), "contact_id_required");
        }

        domain::Contact contact;
        contact.OwnerID = context.UserID;
        contact.ContactID = contactID;

        if (jsonBody.contains("alias")) {
            contact.Alias = jsonBody["alias"].get<std::string>();
        }

        m_contactsRepo->Add(contact);

        json response;
        response["owner_id"] = contact.OwnerID;
        response["contact_id"] = contact.ContactID;
        response["alias"] = contact.Alias.value_or("");

        return handlers::JsonResponse(201, request.version(), response.dump());

    } catch (const std::exception& e) {
        LOG(error) << "AddContact failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(409, request.version(), "failed_to_add_contact");
    }
}

HttpResponse ContactsService::HandleDeleteContact(const HttpRequest& request, const ServiceContext& context) {
    try {
        auto it = context.PathParams.find("contact_id");
        if (it == context.PathParams.end()) {
            return handlers::ErrorResponse(400, request.version(), "contact_id_required");
        }

        m_contactsRepo->Remove(context.UserID, it->second);
        return handlers::JsonResponse(204, request.version(), "");

    } catch (const std::exception& e) {
        LOG(error) << "DeleteContact failed for user " << context.UserID << ": " << e.what();
        return handlers::ErrorResponse(404, request.version(), "contact_not_found");
    }
}

} // namespace msngr::profile::services