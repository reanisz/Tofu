#pragma once

#include <cassert>
#include <array>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <stdexcept>
#include <chrono>
#include <thread>
#include <condition_variable>
#include <functional>
#include <optional>
#include <cmath>

#include "utils/observer_ptr.h"
#include "utils/strong_numeric.h"
#include "utils/tvec2.h"
#include "utils/service_locator.h"
#include "utils/scheduled_update_thread.h"
#include "utils/job.h"
