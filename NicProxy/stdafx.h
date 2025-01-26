#pragma once

#include <net/if.h>
#include <iostream>
#include <string>
#include <cstring>

#include <boost/json.hpp>
#include <cstdlib>
#include <thread>
#include <fstream>
#include <atomic>
#include <memory>
#include <list>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>

using SOCKET = std::shared_ptr<boost::asio::ip::tcp::socket>;
namespace po = boost::program_options;