#include <nudock/nudock.hpp>
#include <random>

void randomize_parameters(nlohmann::json& _request, std::normal_distribution<double>& dist, std::mt19937& gen)
{
    // Randomize osc_pars
    _request["osc_pars"]["Deltam2_32"] = 0.0025 + dist(gen) * 0.0001;
    _request["osc_pars"]["Deltam2_21"] = 0.000075 + dist(gen) * 0.00001;
    _request["osc_pars"]["Theta13"] = 0.15 + dist(gen) * 0.01;
    _request["osc_pars"]["Theta12"] = 0.55 + dist(gen) * 0.02;
    _request["osc_pars"]["Theta23"] = 0.5 + dist(gen) * 0.02;
    _request["osc_pars"]["DeltaCP"] = 0.0 + dist(gen) * 10.0;

    // Randomize sys_pars
    for (auto& [key, value] : _request["sys_pars"].items()) {
        _request["sys_pars"][key] = dist(gen);
    }
}

int main()
{
    // Random gaussian number generator to set the parameters
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution dist(0.0, 1.0);
    
    // Create a nudock instance, with debugging enabled and using unix domain sockets
    NuDock client(true, "", CommunicationType::UNIX_DOMAIN_SOCKET);
    client.start_client();

    // Prepare the set_parameters request json
    nlohmann::json set_pars_request;
    set_pars_request["osc_pars"]["Deltam2_32"] = 0.0025;
    set_pars_request["osc_pars"]["Deltam2_21"] = 0.000075;
    set_pars_request["osc_pars"]["Theta13"] = 0.15;
    set_pars_request["osc_pars"]["Theta12"] = 0.55;
    set_pars_request["osc_pars"]["Theta23"] = 0.5;
    set_pars_request["osc_pars"]["DeltaCP"] = 0.0;
    set_pars_request["sys_pars"]["sys1"] = 0.01;
    set_pars_request["sys_pars"]["sys2"] = 0.02;

    // Empty log_likelihood request json
    nlohmann::json logl_request = "";

    while (true) {
        // Randomize parameters for each iteration
        randomize_parameters(set_pars_request, dist, gen);

        // Send set_parameters request
        client.send_request("/set_parameters", set_pars_request);

        // Send log_likelihood request and print the result
        nlohmann::json logl_response = client.send_request("/log_likelihood", logl_request);
        double logl = logl_response["log_likelihood"];
        std::cout << "Log-likelihood: " << logl << std::endl;

        // Wait for a second before next iteration
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}