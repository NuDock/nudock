#include <nudock/nudock.hpp>

nlohmann::json pong(const nlohmann::json& _request) 
{
  std::cout << "Received request from client: " << _request.dump() << std::endl;
  nlohmann::json response;
  response = "pong";
  return response;
};

class Experiment
{
public:
  // Set the osc and sys parameters from the request json
  // This will be used for the fake logl calulation. Will also print the set
  // parameters to stdout
  nlohmann::json set_parameters(const nlohmann::json& _request)
  {
    for (auto& [key, value] : _request["osc_pars"].items()) {
      if (!value.is_number()) {
        std::cerr << "Invalid osc_param value for key: " << key << std::endl;
        throw std::invalid_argument("Invalid osc_param value for key: " + key);
      }
      osc_pars_[key] = value.get<double>();
    }

    for (auto& [key, value] : _request["sys_pars"].items()) {
      if (!value.is_number()) {
        std::cerr << "Invalid sys_param value for key: " << key << std::endl;
        throw std::invalid_argument("Invalid sys_param value for key: " + key);
      }
      sys_pars_[key] = value.get<double>();
    }

    nlohmann::json response;
    response["status"] = "parameters set";
    std::cout << "Set osc_pars: ";
    for (const auto& [key, value] : osc_pars_) {
      std::cout << key << "=" << value << " ";
    }
    std::cout << std::endl;
    std::cout << "Set sys_pars: ";
    for (const auto& [key, value] : sys_pars_) {
      std::cout << key << "=" << value << " ";
    }
    std::cout << std::endl;
    return response;
  }

  // Simple fake log-likelihood calculation
  // It will compute a fake log-likelihood based on the internally held parameters
  nlohmann::json log_likelihood(const nlohmann::json& _request)
  {
    // Implementation of log-likelihood calculation using osc_pars_ and sys_pars_
    nlohmann::json response;
    response["log_likelihood"] = 0.0; // Placeholder

    // Compute fake log-likelihood based on current parameters
    double logl = 0.0;

    for (const auto& [key, central_value] : osc_par_central_) {
      double current_value = osc_pars_.count(key) ? osc_pars_[key] : central_value;
      logl += std::pow(current_value - central_value, 2);
    }

    // And systeatics, centered at 0 with sigma 1
    for (const auto& [key, current_value] : sys_pars_) {
      logl += std::pow(current_value - 0.0, 2);
    }


    response["log_likelihood"] = logl;

    return response;
  }

  private:
    // Hold the osc parameter name -- value pairs
    std::map<std::string, double> osc_pars_;

    // Hold the systematic name -- value pairs
    std::map<std::string, double> sys_pars_;

    // Central values for osc parameters, used for fake logl calculation
    std::map<std::string, double> osc_par_central_ = {
      {"Deltam2_32", 0.0025},
      {"Deltam2_21", 0.000075},
      {"Theta12", 0.55},
      {"Theta13", 0.15},
      {"Theta23", 0.5},
      {"DeltaCP", 0.0}
    };
};

int main()
{
  // Example fake experiment class (e.g. SingleSample/Multi Experiment from NOvA, or some SamplePDFSKBase (or class that contains multiple SamplePDFBase etc) from T2K
  Experiment experiment;

  // Create a NuDock instance with debugging enabled and using unix domain sockets.
  // Empty schema location means it will use the default installed location.
  NuDock dock(true, "", CommunicationType::UNIX_DOMAIN_SOCKET);

  // You can bind to a function that's not a member of any class
  dock.register_response("/ping", std::bind(pong, std::placeholders::_1));

  // Or bind to member functions of a class instance
  dock.register_response("/set_parameters", std::bind(&Experiment::set_parameters, &experiment,  std::placeholders::_1));
  dock.register_response("/log_likelihood", std::bind(&Experiment::log_likelihood, &experiment, std::placeholders::_1));
  dock.start_server();
  return 0;
}
