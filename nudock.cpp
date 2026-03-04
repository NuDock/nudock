#include "nudock.hpp"

NuDock::NuDock(bool _debug, 
               const std::string &_default_schemas_location,
               const CommunicationType& _comm_type,
               const int& _port,
               const VerbosityLevel& _verbosity)
    : m_server(nullptr),
      m_client(nullptr),
      m_debug(_debug), m_debug_prefix("Undefined"),
      m_default_schemas_location(_default_schemas_location),
      m_request_counter(0),
      m_comm_type(_comm_type), 
      m_port(_port),
      m_verbosity(_verbosity)
{
  if (m_default_schemas_location.empty()) {
    m_default_schemas_location = NUDOCK_SCHEMAS_DIR;
  }
  LOG_INFO("Created Nudock instance!");
  LOG_INFO("debug  : " << m_debug);
  LOG_INFO("schemas: " << m_default_schemas_location);
}

nlohmann::json NuDock::load_json_file(const std::string& _path)
{
  std::ifstream file(_path.c_str());
  if (!file.is_open()) {
    throw std::runtime_error("Could not open file: " + std::string(_path));
  }

  nlohmann::json j;
  try {
    j = nlohmann::json::parse(file);
  } catch (const nlohmann::json::parse_error& e) {
    throw std::runtime_error("JSON parsing error: " + std::string(e.what()));
  }

  return j;
}

void NuDock::register_response(const std::string& _request,
                               HandlerFunction _handler_function,
                               const std::string& _schema_path)
{
  std::string schema_path = _schema_path.empty() ? m_default_schemas_location + _request + ".schema.json" : std::string(_schema_path);

  // Check if the request name is valid
  if (_request.empty()) {
    LOG_ERROR("Request name is empty!");
    return;
  }
  if (m_request_handlers.count(_request)) {
    LOG_ERROR("Request handler for \"" << _request << "\" already exists!");
    return;
  }

  // Create the schema validator for a specific request
  nlohmann::json schema = load_json_file(schema_path);
  SchemaValidator validator;
  validator.schema = schema["properties"];

  // Request validator
  validator.request_validator = std::make_shared<json_validator>();
  validator.request_validator->set_root_schema(schema["properties"]["request"]);

  // Response validator
  validator.response_validator = std::make_shared<json_validator>();
  validator.response_validator->set_root_schema(schema["properties"]["response"]);

  // Move the validator to the internally held map
  m_schema_validator[_request] = std::move(validator);

  // Add the request handler function
  m_request_handlers[_request] = std::move(_handler_function);
  LOG_INFO("Registered request handler for \"" << _request << "\" with schema at: " << schema_path);
}

bool NuDock::validate_start(const nlohmann::json& _message)
{
  if (!_message.contains("version")) {
    LOG_ERROR("Received /validate_start request without provided \"version\" entry! We will crash. Full request received:");
    LOG_ERROR(_message.dump());
    return false;
  }

  if (_message["version"] != m_version) {
    LOG_ERROR("Received request from client with version: " << _message["version"] << ", this server's version is " << m_version);
    return false;
  }
  else {
    LOG_INFO("Internal version: " << m_version << " external version: " << _message["version"]);
  }

  return true;
}

void NuDock::start_server()
{
  if (m_client || m_server) {
    LOG_ERROR("Client or server already started");
    return;
  }

  m_debug_prefix = "Server";
  // Create the server instance
  m_server = std::make_unique<httplib::Server>();

  if (!m_server->is_valid()){
    LOG_ERROR("Server is not valid");
    return;
  }

  // Checks the served does upon receiving "validate_start" message: checks
  // clients version against its own, crashes if needed, but not before
  // sending an appropriate response.
  m_server->Post("/validate_start", [&](const httplib::Request& req, httplib::Response& res) {
    try {
      LOG_INFO("Server received request for /validate_start");

      // deserialize
      nlohmann::json req_json = nlohmann::json::parse(req.body);
      bool validated = validate_start(req_json);
      LOG_INFO("Server validated, sending validation response to the client to validate it");

      m_response.clear();
      m_response["version"] = m_version;

      res.set_content(m_response.dump(), "application/json");

      if (!validated) {
        m_server->stop();
      }
    }
    catch (const std::exception& e) {
      LOG_ERROR("Exception caught: \"" << e.what() << "\" Setting response to 400");
      ERROR_RESPONSE(res, e.what());
    }
  });

  // Iterate over and listen to the registered requests
  for (const auto& [request_name, handler]: m_request_handlers) {
    m_server->Post(request_name.c_str(), [request_name, handler, this](const httplib::Request& req, httplib::Response& res) {
      try {
        m_request_counter++;
        // Validating the request
        m_request.clear();
        m_request = nlohmann::json::parse(req.body);

        if (m_debug) {
          try {
            m_schema_validator[request_name].request_validator->validate(m_request, m_err);
          }
          catch (const std::exception& e) {
            LOG_ERROR("Validating the request with name \"" << request_name << "\" failed! Here is why: " << e.what());
            LOG_ERROR(" -- Expected format : " << m_schema_validator[request_name].schema["request"].dump());
            LOG_ERROR(" -- Request received: " << m_request.dump());
            LOG_ERROR(" -- Aborting");
            ERROR_RESPONSE(res, "Server request validation failed: " + std::string(e.what()));
          }
        }

        // Getting the response
        //m_response.clear();
        m_response = handler(m_request);

        // Validating the response
        if (m_debug) {
          try {
            m_schema_validator[request_name].response_validator->validate(m_response, m_err);
          }
          catch (const std::exception& e) {
            LOG_ERROR("Validating the response failed! Here is why: " << e.what());
            LOG_ERROR("Expected format: " << m_schema_validator[request_name].schema["response"].dump());
            LOG_ERROR("Response given : " << m_response.dump());
            LOG_ERROR("Aborting");
            ERROR_RESPONSE(res, "Server response validation failed: " + std::string(e.what()));
          }
        }

        // Sending the response back to the client
        res.set_content(m_response.dump(), "application/json");
        LOG_DEBUG("Request counter: " << m_request_counter);
      } 
      catch (const std::exception& e) {
        LOG_ERROR("Exception caught for request \"" << request_name << "\" : \"" << e.what() << "\" Setting response to 400");
        ERROR_RESPONSE(res, e.what());
      }
    });
  }

  m_server->Post(R"(/.*)", [&](const httplib::Request& req, httplib::Response& res) {
    const std::string& path = req.path;
    if (m_request_handlers.count(path)) {
        // Let the registered handler deal with it.
        return;
    }

    nlohmann::json err = {
        {"error", "Unknown request title: " + path}
    };
    res.status = 404;
    res.set_content(err.dump(2), "application/json");
  });

  LOG_INFO("Registered requests handlers: ");
  for (const auto& request_name: m_request_handlers) {
    LOG_INFO(request_name.first);
  }

  LOG_INFO("VERSION: " << m_version << " started");

  switch (m_comm_type) {
    case CommunicationType::UNIX_DOMAIN_SOCKET:
      LOG_INFO("Using UNIX domain socket for communication");
      // Clean up the old socket file, if any
      unlink(("/tmp/nudock_" +  std::to_string(m_port) + ".sock").c_str());
      m_server->set_address_family(AF_UNIX).listen(("/tmp/nudock_" +  std::to_string(m_port) + ".sock").c_str(), m_port);
      break;
    case CommunicationType::LOCALHOST:
      LOG_INFO("Using localhost for communication");
      m_server->listen("localhost", m_port);
      break;
    case CommunicationType::TCP:
      LOG_INFO("TCP for communication not supported!");
    default:
      LOG_ERROR("Unsupported communication type!");
      return;
  }
}

void NuDock::start_client()
{
  if (m_client || m_server) {
    LOG_ERROR("Client or server already started");
    return;
  }

  m_debug_prefix = "Client";
  LOG_INFO("Starting the client");

  switch (m_comm_type) {
    case CommunicationType::UNIX_DOMAIN_SOCKET:
      LOG_INFO("Using UNIX domain socket for communication");
      m_client = std::make_unique<httplib::Client>("/tmp/nudock_" +  std::to_string(m_port) + ".sock");
      m_client->set_address_family(AF_UNIX);
      break;
    case CommunicationType::LOCALHOST:
      LOG_INFO("Using localhost for communication");
      m_client = std::make_unique<httplib::Client>("localhost", m_port);
      break;
    case CommunicationType::TCP:
      LOG_INFO("TCP for communication not supported!");
    default:
      LOG_ERROR("Unsupported communication type!");
      return;
  }

  LOG_INFO("Client started! Waiting for the server...");
  // Since we just started the client, we will validate it against the server
  // straight away by sending a request to the server with the version of the
  // client.
  /// @todo: Change this to use the send_request function
  nlohmann::json req_json_validate;
  req_json_validate["version"] = m_version;

  auto res = m_client->Post("/validate_start", req_json_validate.dump(), "application/json");
  if (res && res->status == 200) {
    auto res_json = nlohmann::json::parse(res->body);
    validate_start(res_json);
    LOG_INFO("Client validated!");
  }
  else {
    LOG_ERROR("Client failed to validate!");
    LOG_ERROR(" -- The message was: " << req_json_validate.dump());
    LOG_ERROR("Request failed with status: " << (res ? res->status : 0) << " and error: " << res.error());
    throw res.error();
  }

  LOG_INFO("VERSION: " << m_version << " started");
}

nlohmann::json NuDock::send_request(const std::string& _request, const nlohmann::json& _message)
{
  m_request_counter++;
  if (!m_client) {
    LOG_ERROR("Client needs to be started first!");
    std::abort();
  }

  if (_request.empty()) {
    LOG_ERROR("Request name is empty!");
    std::abort();
  }

  try{
    httplib::Result res = m_client->Post(_request, _message.dump(), "application/json");

    if (res && res->status == 200) {
      m_response.clear();
      m_response = nlohmann::json::parse(res->body);
      LOG_DEBUG("Received response: " << m_response << " from Server");
      LOG_DEBUG("Request counter: " << m_request_counter);
      return m_response;
    } else {
      LOG_ERROR("Request failed with status: " << (res ? res->status : 0) 
                << ", error: \"" << (res ? res->body : "") 
                << "\", message: " << _message.dump());
      LOG_DEBUG("Request counter: " << m_request_counter);
      std::abort();
    }
  } catch (const std::exception& e) {
    LOG_ERROR("Exception caught while sending request: " << e.what());
    std::abort();
  }
}