#include "profile_service/handlers/contacts_handler.hpp"
#include "profile_service/domain/models.hpp"
#include <nlohmann/json.hpp>

namespace msngr::profile::handlers {

    using json = nlohmann::json;

    ContactsHandler::ContactsHandler(std::shared_ptr<repository::ContactsRepository> contactsRepo)
        : m_contactsRepo(std::move(contactsRepo))
    {
    }

    HttpResponse ContactsHandler::Handle(const HttpRequest& request, const RouteContext& context)
    {
        auto method = request.method();
        auto target = std::string(request.target());
        if (method == beast_http::verb::get && target == "/contacts")
            return HandleGetContacts(request, context);
        else if (method == beast_http::verb::post && target == "/contacts")
            return HandleAddContact(request, context);
        else if (method == beast_http::verb::delete_ && target.find("/contacts/") == 0)
            return HandleDeleteContact(request, context);
        return ErrorResponse(404, request.version(), "not_found");
    }

    HttpResponse ContactsHandler::HandleGetContacts(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto contacts = m_contactsRepo->List(context.UserID);
            json response = json::array();
            for (const auto & contact : contacts) {
                json item;
                item["owner_id"] = contact.OwnerID;
                item["contact_id"] = contact.ContactID;
                item["alias"] = contact.Alias.value_or("");
                item["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(contact.CreatedAt.time_since_epoch()).count();
                response.push_back(item);
            }
            return JsonResponse(200, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(404, request.version(), "contacts not found");
        }
    }

    HttpResponse ContactsHandler::HandleAddContact(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto json_body = json::parse(request.body());
            std::string contactID = json_body.value("contact_id", "");
            if (contactID.empty())
                return ErrorResponse(400, request.version(), "contact_id is required");
            domain::Contact contact;
            contact.OwnerID = context.UserID;
            contact.ContactID = contactID;
            if (json_body.contains("alias"))
                contact.Alias = json_body["alias"].get<std::string>();
            m_contactsRepo->Add(contact);
            json response;
            response["owner_id"] = contact.OwnerID;
            response["contact_id"] = contact.ContactID;
            response["alias"] = contact.Alias.value_or("");
            return JsonResponse(201, request.version(), response.dump());
        }
        catch (const std::exception& e) {
            return ErrorResponse(409, request.version(), "failed to add contact");
        }
    }

    HttpResponse ContactsHandler::HandleDeleteContact(const HttpRequest& request, const RouteContext& context)
    {
        try {
            auto it = context.PathParams.find("contact_id");
            if (it == context.PathParams.end())
                return ErrorResponse(400, request.version(), "contact_id is required");
            m_contactsRepo->Remove(context.UserID, it->second);
            return JsonResponse(204, request.version(), "");
        }
        catch (const std::exception& e) {
            return ErrorResponse(404, request.version(), "contact not found");
        }
    }

} // namespace msngr::profile::handlers