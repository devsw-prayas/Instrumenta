#include "Logger.h"

namespace instrumenta {
	// BaseLogger Implementation
	BaseLogger BaseLogger::instance;

	BaseLogger::BaseLogger() : libraryName("instrumenta") {}

	BaseLogger& BaseLogger::getInstance() {
		return instance;
	}

	void BaseLogger::log(E_LogLevel level, const std::string& component, const std::string& subComponent,
		const std::string& message, const std::vector<std::any>& args, const std::string& tag) {
		std::lock_guard<std::mutex> lock(loggerMutex);

		const std::string& activeTag = tag.empty() ? "default" : tag;
		TagConfig& config = getOrCreateTagConfig(activeTag);

		if (!config.enabled || level < config.minLevel) return;

		std::string timestamp = getCurrentTimestamp();
		std::vector<std::string> formattedArgs = formatArgs(args);
		std::string tempTag = activeTag;
		LogEntry entry(timestamp, level, libraryName, component,
			subComponent, message, formattedArgs, true, activeTag);

		logHistory.addLog(entry);
		logCounts[tempTag][level]++;

		// Write to appropriate sinks based on output destinations
		if ((config.outputDestinations & E_LogOutput::CONSOLE) != E_LogOutput::NONE) {
			routers.find(CONSOLE_SINK_ROUTER_TAG)->second->route(entry, sinks);
		}

		if ((config.outputDestinations & E_LogOutput::FILE) != E_LogOutput::NONE) {
			routers.find(FILE_SINK_ROUTER_TAG)->second->route(entry, sinks);
		}

		for (auto& router : routers) {
			if (router.first == CONSOLE_SINK_ROUTER_TAG || router.first == FILE_SINK_ROUTER_TAG)
				continue;
			if (router.second->route(entry, sinks)) {
				break;
			}
		}

		// If the log level is ERROR_, throw an exception
		if (level == E_LogLevel::ERROR_) {
			throw LoggedRuntimeError(entry.toString(), logHistory);
		}
	}

	void BaseLogger::registerSink(const std::string& tag, std::unique_ptr<ILogSink> sink) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		(*sinks)[tag] = std::move(sink);
	}

	void BaseLogger::removeSink(const std::string& tag) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		sinks->erase(tag);
	}

	ILogSink* BaseLogger::getSink(const std::string& tag) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto it = sinks->find(tag);
		return (it != sinks->end()) ? it->second.get() : nullptr;
	}

	void BaseLogger::setTagConfig(const std::string& tag, const TagConfig& config) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		tagConfigs[tag] = config;
	}

	TagConfig BaseLogger::getTagConfig(const std::string& tag) const {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto it = tagConfigs.find(tag);
		return (it != tagConfigs.end()) ? it->second : TagConfig{};
	}

	void BaseLogger::setEnabled(const std::string& tag, bool enable) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		getOrCreateTagConfig(tag).enabled = enable;
	}

	bool BaseLogger::isEnabled(const std::string& tag) const {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto it = tagConfigs.find(tag);
		return (it != tagConfigs.end()) ? it->second.enabled : true;
	}

	void BaseLogger::setMinLevel(const std::string& tag, E_LogLevel level) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		getOrCreateTagConfig(tag).minLevel = level;
	}

	E_LogLevel BaseLogger::getMinLevel(const std::string& tag) const {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto it = tagConfigs.find(tag);
		return (it != tagConfigs.end()) ? it->second.minLevel : E_LogLevel::DEBUG_;
	}

	void BaseLogger::setOutputDestinations(const std::string& tag, E_LogOutput destinations) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		getOrCreateTagConfig(tag).outputDestinations = destinations;
	}

	int BaseLogger::getLogCount(const std::string& tag, E_LogLevel level) const {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto tagIt = logCounts.find(tag);
		if (tagIt != logCounts.end()) {
			auto levelIt = tagIt->second.find(level);
			if (levelIt != tagIt->second.end()) {
				return levelIt->second;
			}
		}
		return 0;
	}

	int BaseLogger::getTotalLogCount() const {
		std::lock_guard<std::mutex> lock(loggerMutex);
		int total = 0;
		for (const auto& tagPair : logCounts) {
			for (const auto& levelPair : tagPair.second) {
				total += levelPair.second;
			}
		}
		return total;
	}

	void BaseLogger::synchronousFlush(const std::string& tag) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		if (tag.empty()) {
			for (auto& sinkPair : sinks) {
				if (sinkPair.second) sinkPair.second->flush();
			}
		}
		else {
			auto it = sinks->find(tag);
			if (it != sinks->end() && it->second) {
				it->second->flush();
			}
		}
	}

	const LogHistory& BaseLogger::getHistory() const {
		return logHistory;
	}

	void BaseLogger::setLibraryName(const std::string& name) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		libraryName = name;
	}

	std::string BaseLogger::getCurrentTimestamp() {
		try {
			auto now = std::chrono::system_clock::now();
			auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
			return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
		}
		catch (const std::exception& e) {
			return "[ERROR_: Timestamp fail: " + std::string(e.what()) + "]";
		}
	}

	std::vector<std::string> BaseLogger::formatArgs(const std::vector<std::any>& args) {
		std::vector<std::string> formatted;
		formatted.reserve(args.size());

		for (const auto& arg : args) {
			try {
				if (arg.type() == typeid(int)) {
					formatted.push_back(std::to_string(std::any_cast<int>(arg)));
				}
				else if (arg.type() == typeid(double)) {
					formatted.push_back(std::to_string(std::any_cast<double>(arg)));
				}
				else if (arg.type() == typeid(float)) {
					formatted.push_back(std::to_string(std::any_cast<float>(arg)));
				}
				else if (arg.type() == typeid(std::string)) {
					formatted.push_back(std::any_cast<std::string>(arg));
				}
				else if (arg.type() == typeid(const char*)) {
					formatted.emplace_back(std::any_cast<const char*>(arg));
				}
				else if (arg.type() == typeid(bool)) {
					formatted.emplace_back(std::any_cast<bool>(arg) ? "true" : "false");
				}
				else {
					formatted.emplace_back("<unknown_type>");
				}
			}
			catch (const std::bad_any_cast&) {
				formatted.emplace_back("<cast_error>");
			}
		}
		return formatted;
	}

	TagConfig& BaseLogger::getOrCreateTagConfig(const std::string& tag) {
		auto it = tagConfigs.find(tag);
		if (it == tagConfigs.end()) {
			tagConfigs[tag] = TagConfig{};
			return tagConfigs[tag];
		}
		return it->second;
	}

	void BaseLogger::registerTag(std::string& tag, bool isEnabled, E_LogLevel minLevel, E_LogOutput outputDestinations) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		TagConfig config;
		config.enabled = isEnabled;
		config.minLevel = minLevel;
		config.outputDestinations = outputDestinations;
		tagConfigs[tag] = config;
	}

	void BaseLogger::clearRouters() {
		std::lock_guard<std::mutex> lock(loggerMutex);
		routers.clear();
	}

	ISinkRouter* BaseLogger::getRouter(const std::string& routerTag) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		auto it = routers.find(routerTag);
		return (it != routers.end()) ? it->second.get() : nullptr;
	}

	void BaseLogger::registerRouter(const std::string& routerTag, std::unique_ptr<ISinkRouter> router) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		routers[routerTag] = std::move(router);
	}

	void BaseLogger::removeRouter(const std::string& routerTag) {
		std::lock_guard<std::mutex> lock(loggerMutex);
		routers.erase(routerTag);
	}
}