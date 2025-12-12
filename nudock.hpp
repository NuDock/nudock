/**
 * @file nudock.hpp
 * 
 * @brief A small tool for server-client communication using JSON over HTTP.
 * 
 * @todo: Add debugging that prints out all the json messages into a file.
 * @todo: Sort out the debugging define, should be more descriptive.
 * @todo: Add schema validation layers on the client side too.
 * @todo: The validation should be a compile-time option? Or better, templated. E.g. NuDock<true> for validation, NuDock<false> for no validation.
 * @todo: All functions should have documentation. More comments.
 * @todo: Rethink abort(). 
 * @todo: Add versioning to the schemas, so that client and server can negotiate which version to use.
 * @todo: Add ability to start a container with the server inside (with explicit port number) for easier deployment.
 * 
 * @date 2025-07-01
 * @author Artur Sztuc (a.sztuc@ucl.ac.uk)
 */

#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json-schema.hpp>

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>

#include "nudock_config.hpp"

//using nlohmann::json;
using nlohmann::json_schema::json_validator;
using HandlerFunction = std::function<nlohmann::json(const nlohmann::json&)>;

// Debugging macro to print debug messages with function name and line number
#define DEBUG() (this->m_debug_prefix + "::" + __func__ + "::L" + std::to_string(__LINE__) + " ")

// Macro to set an error response and stop the server
#define ERROR_RESPONSE(res, message) \
  res.status = 400; \
  res.set_content(message, "text/plain"); \
  m_server->stop(); \
  return;

// SchemaValidator struct to hold request and response validators along with the
// full schema
struct SchemaValidator {
  std::shared_ptr<json_validator> request_validator;
  std::shared_ptr<json_validator> response_validator;
  nlohmann::json schema;
};

// Custom error handler that throws exceptions on validation errors
class custom_throwing_error_handler : public nlohmann::json_schema::error_handler
{
	void error(const nlohmann::json::json_pointer &ptr, const nlohmann::json &instance, const std::string &message) override
	{
    //nlohmann::json_schema::basic_error_handler::error(ptr, instance, message);
    throw std::invalid_argument(std::string("Pointer: \"") + ptr.parent_pointer().to_string() + "\" instance: \"" + instance.dump() + "\"error message: \"" + message + "\"");
	}
};

enum class CommunicationType {
  UNIX_DOMAIN_SOCKET,
  LOCALHOST,
  TCP,
};

class NuDock
{
  // Public member functions
  public:
    /**
     * @brief NuDock constructor
     * 
     * Constructor for NuDock api instance, which can be used as a server or a client.
     * 
     * @param _debug Whether to print extra debug messages & do extra validations (not implemented yet)
     * @param _default_schemas_location Default location of the json schemas. Using NuDock install folder if not specified.
     * @param _comm_type Communication type between server and client, default is localhost. Unix domain sockets are faster, but only work on the same machine. TCP not implemented.
     * @param _port Port number for communication, default is 1234. Not important if using unix domain socket.
     */
    NuDock(bool _debug=true, 
           const std::string& _default_schemas_location=NUDOCK_SCHEMAS_DIR,
           const CommunicationType& _comm_type=CommunicationType::LOCALHOST,
           const int& _port=1234);

    /** 
     * @brief Server: responds to requests from the client
     * 
     * Blocking function, meaning software execution stops here until the server is stopped.
     * It starts the server, waits for requests from the client and responds to them.
     */ 
    void start_server();

    /**
     * @brief Client: sends requests to the server and receives answers
     * 
     * It creates httplib::Client instance, and validates the server by sending
     * the internal software version as a string. Server must be started first.
     * 
     * @note MUST be run before sending any requests.
     * @todo Maybe add start_client() in the send_request() function if client not initialised?
     */
    void start_client();

    /**
     * @brief Register the server's response function for a specific request name.
     * 
     * The handler function must take a json object as input (the request) and
     * return a json object (the response). 
     * 
     * The request name must be unique, e.g. /set_parameters. 
     * 
     * The schema path is optional, if not provided it will use the default
     * schema location + request name + ".schema.json".
     * 
     * @param _request Request ID name, including the leading slash, e.g. "/set_parameters"
     * @param _handler_function Function to handle the request, takes json request and returns a json response
     * @param _schema_path Path of the schema file for the request and response validation.
     */
    void register_response(const std::string& _request_name, 
                           HandlerFunction _handler_function,
                           const std::string& _schema_path = "");

    /**
     * @brief Function for the client to send a request to the server.
     * 
     * @param _request Request ID name
     * @param _message json object with the request message
     * @return json object with the response from the server
     */
    nlohmann::json send_request(const std::string& _request_name,
                                const nlohmann::json& _message);

  // Private member functions
  private:
    /**
     * @brief Validates the server / client communication.
     * 
     * @param _message Message contains the version of the client, to be compared with the server's version.
     * @return true Communication successful
     * @return false Communication failed
     */
    bool validate_start(const nlohmann::json& _message);

    /**
     * @brief Loads json object from a given file path.
     * 
     * @param _path Path to the json file
     * @return json Json object loaded from the file
     */
    nlohmann::json load_json_file(const std::string& _path);

  // Private member data
  private:
    /// @brief version of the server/client.
    std::string m_version = NUDOCK_VERSION;

    /// @brief server object with external experiment
    std::unique_ptr<httplib::Server> m_server;

    /// @brief client requesting responses from external experiment
    std::unique_ptr<httplib::Client> m_client;

    /// @brief map of request names to their handler functions
    std::unordered_map<std::string, HandlerFunction> m_request_handlers;

    /// @brief map of request names to their schema validators
    std::unordered_map<std::string, SchemaValidator> m_schema_validator;

    nlohmann::json m_request;
    nlohmann::json m_response;

    /// @brief whether we want to print debug messages
    bool m_debug;

    /// @brief string prefix for debugging messages
    std::string m_debug_prefix;

    std::string m_default_schemas_location;

    /// @brief custom error handler for json validation
    custom_throwing_error_handler m_err;

    /// @brief Counter for the number of requests sent / processed
    uint64_t m_request_counter;

    CommunicationType m_comm_type;
    int m_port;
};